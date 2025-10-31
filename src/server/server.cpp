#include <server/server.hpp>

Server::Server(const std::string &ipAddress, const short &port) {
    this->ipAddress = ipAddress;
    this->port = port;

    // create the socket
    this->hostSocket = ::socket(AF_INET, SOCK_STREAM, 0); // IPv4, stream socket, TCP
    if (!isValidSocket(this->hostSocket)) {
        std::cerr << "[Server::Server] Error: Failed to create socket. Error code " << errno
                    << std::endl;
        return;
    }

    int opt = 1;
    int result = setsockopt(this->hostSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (isError(result)) {
        std::cerr << "[Server::Server] Error: Failed to set SO_REUSEADDR on host socket. Error code "
                  << errno << std::endl;
        return;
    }

    // bind the socket
    struct sockaddr_in serverInfo;
    serverInfo.sin_family = AF_INET; // IPv4
    serverInfo.sin_port = htons(port); // IPv4
    result = inet_pton(AF_INET, ipAddress.c_str(), &(serverInfo.sin_addr.s_addr));
    if (result < 0) {
        std::cerr << "[Server::Server] Error: Failed to convert IP address '" << ipAddress
                  << "' to network short. Error code " << errno << "." << std::endl;
        
        return;
    }

    result = bind(hostSocket, (struct sockaddr*)&serverInfo, sizeof(serverInfo));
    if (isError(result)) {
        std::cerr << "[Server::Server] Error: Failed to bind socket on address '" << ipAddress
                    << ":" << port << "'. Error code " << errno << "." << std::endl;
        return;
    }

    this->isBound = true;
}

Server::~Server() {
    int result = shutdown(this->hostSocket, SHUT_RDWR);
    if (isError(result)) {
        std::cerr << "[Server::~Server] Error: Failed to shutdown server socket. Error code " << errno << "." << std::endl;
    }

    for (const auto& connection : this->connections) {
        result = ::close(connection.second);
        if (isError(result)) {
            std::cerr << "[Server::~Server] Error: Failed to close connection with identifier '" << connection.first
                    << "'. Error code " << errno << "." << std::endl;
            return;
        }
    }

    this->closeSelf();

    this->isBound = false;

    free(this->epollEvents);
}

void Server::listen(const size_t &numThreads, const size_t &tasksQueueSize, const size_t &listeningQueueSize) {
    int result = ::listen(this->hostSocket, listeningQueueSize);
    if (isError(result)) {
        std::cerr << "[Server::listen] Error: Failed to listen to socket on address'" << this->ipAddress
                    << ":" << this->port << "'. Error code " << errno << "." << std::endl;
        
        return;
    }

    this->threadPool = new ThreadPool(numThreads, tasksQueueSize);

    // TODO: setup epoll
    this->epollEvents = (struct epoll_event*)malloc(listeningQueueSize * sizeof(struct epoll_event));

    this->isListening = true;
}

void Server::accept(const connectionId_t &connectionId) {
    // check if connection exists
    if (connections.contains(connectionId)) {
        std::cerr << "[Server::accept] Warning: Connection with ID '" << connectionId
                  << "' already exists. Closing old connection." << std::endl;
        
        close(connectionId); // THIS IS CORRECT DO NOT MODIFY
    }

    // accept the connection
    struct sockaddr_in clientInfo;
    socklen_t clientInfoSize = sizeof(struct sockaddr_in);

    this->isAccepting = true;

    socket_t clientSocket = ::accept(this->hostSocket, (struct sockaddr*)&clientInfo, &clientInfoSize);

    this->isAccepting = false;

    if (!isValidSocket(clientSocket)) {
        std::cerr << "[Server::accept] Error: Failed to accept connection with ID '" << connectionId
                  << "'. Error code " << errno << "." << std::endl;
        return;
    }

    this->connections.insert({connectionId, clientSocket});
}

void Server::close(const connectionId_t &connectionId) {
    if (!this->connections.contains(connectionId)) {
        std::cerr << "[Server::close] Error: Connection with ID '" << connectionId
                  << "' doesn't exist." << std::endl;
        return;
    }

    this->threadPool->awaitConnectionTasks(connectionId);

    int result = ::close(this->connections[connectionId]);
    if (isError(result)) {
        std::cerr << "[Server::close] Error: Failed to close connection with identifier '" << connectionId
                  << "'. Error code " << errno << "." << std::endl;
        return;
    }

    this->connections.erase(connectionId);
}

void Server::closeSelf() {
    int result = ::close(this->hostSocket);
    if (isError(result)) {
        std::cerr << "[Server::~Server] Error: Failed to close server socket. Error code "
                  << errno << "." << std::endl;
        
        return;
    }

    this->isBound = false;
}

void Server::receive(const connectionId_t &connectionId, const server_callback_t &callback) {
    if (!this->connections.contains(connectionId)) {
        std::cerr << "[Server::receive] Error: Connection with ID '" << connectionId
                  << "' doesn't exist." << std::endl;
        return;
    }

    this->threadPool->enqueueTask(connectionId, [this, &connectionId, &callback]() {
        std::string buffer(RECEIVE_BUFFER_SIZE, '\0');
        int result = recv(this->connections[connectionId], (char*)buffer.c_str(), RECEIVE_BUFFER_SIZE, 0);
        HttpRequest request(buffer);
        HttpResponse response = callback(request);
        // send it here

        // testing
        this->send(0, "This should be working");
    });

    this->close(0);
}

std::string Server::receiveSync(const connectionId_t &connectionId) {
    if (!this->connections.contains(connectionId)) {
        std::cerr << "[Server::receiveSync] Error: Connection with ID '" << connectionId
                  << "' doesn't exist." << std::endl;
        return "";
    }

    std::string buffer(RECEIVE_BUFFER_SIZE, '\0');

    recv(this->connections[connectionId], (char*)buffer.c_str(), RECEIVE_BUFFER_SIZE, 0);

    return buffer;
}

void Server::send(const connectionId_t &connectionId, const std::string &data) {
    if (!this->connections.contains(connectionId)) {
        std::cerr << "[Server::send] Error: Connection with ID '" << connectionId
                  << "' doesn't exist." << std::endl;
        return;
    }

    int result = ::send(this->connections[connectionId], data.c_str(), data.length(), 0);
    if (isError(result)) {
        std::cerr << "[Server::send] Error: Failed to send data across connection with ID '"
                  << connectionId << "'.  Error code " << errno << "." << std::endl;
        
        return;
    }
}

void Server::setCallback(const server_callback_t &inputCallback) {
    this->callback = inputCallback;
}

void Server::run() {
    this->listen();
    if (!this->isListening) {
        return;
    }

    // testing
    this->accept(0);
    this->receive(0, this->callback);

    // use epoll to monitor both the original server socket and all other connections
    while (true) {}
}