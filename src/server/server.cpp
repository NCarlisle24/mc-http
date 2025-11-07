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
    int result = ::shutdown(this->hostSocket, SHUT_RDWR);
    if (isError(result)) {
        std::cerr << "[Server::~Server] Error: Failed to shutdown server socket. Error code " << errno << "." << std::endl;
    }

    for (const auto& connection : this->connections) {
        HttpResponse closeResponse;
        closeResponse.setResponseHeader("Connection", "close");
        this->send(connection.first, closeResponse.serialize());

        result = ::close(connection.second);
        if (isError(result)) {
            std::cerr << "[Server::~Server] Error: Failed to close connection with identifier '" << connection.first
                    << "'. Error code " << errno << "." << std::endl;
            return;
        }
    }

    this->closeHost();

    this->isBound = false;

    free(this->epollEvents);
}

void Server::listen(const size_t &numThreads, const size_t &tasksQueueSize, const size_t &listeningQueueSize,
                    const size_t &maxConnections) {
    int result = ::listen(this->hostSocket, listeningQueueSize);
    if (isError(result)) {
        std::cerr << "[Server::listen] Error: Failed to listen to socket on address'" << this->ipAddress
                    << ":" << this->port << "'. Error code " << errno << "." << std::endl;
        
        return;
    }

    this->threadPool = new ThreadPool(numThreads, tasksQueueSize);

    // setup epoll
    this->epollManager = epoll_create1(0); // no flags
    if (isError(this->epollManager)) {
        std::cerr << "[Server::listen] Error: Failed to create epoll instance. Error code " << errno << std::endl;
        return;
    }

    this->maxConnections = maxConnections;
    this->epollEvents = (struct epoll_event*)malloc(maxConnections * sizeof(struct epoll_event));

    // add connection port to epoll
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLERR; // know when the socket is readable
    event.data.fd = this->hostSocket;

    result = epoll_ctl(this->epollManager, EPOLL_CTL_ADD, this->hostSocket, &event);
    if (isError(result)) {
        std::cerr << "[Server::listen] Error: Failed to add host socket to epoll manager. Error code " << errno << std::endl;
        return;
    }

    this->isListening = true;
}

void Server::accept(const connection_id_t &connectionId) {
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

    makeSocketNonBlocking(clientSocket);

    // add socket to epoll events
    struct epoll_event newEvent;
    newEvent.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP | EPOLLERR;
    newEvent.data.fd = clientSocket;

    epoll_argument_t* eventData = (epoll_argument_t*)malloc(sizeof(epoll_argument_t));
    eventData->connectionId = connectionId;

    newEvent.data.ptr = eventData;

    int result = epoll_ctl(this->epollManager, EPOLL_CTL_ADD, clientSocket, &newEvent);
    if (isError(result)) {
        std::cerr << "[Server::accept] Error: Failed to add connection socket with ID '"
                  << connectionId << "' to epoll manager. Error code " << errno << std::endl;
        return;
    }

    this->connections.insert({connectionId, clientSocket});
}

void Server::close(const connection_id_t &connectionId) {
    if (!this->connections.contains(connectionId)) {
        std::cerr << "[Server::close] Error: Connection with ID '" << connectionId
                  << "' doesn't exist." << std::endl;
        return;
    }

    // wait for the tasks to end, then send an HTTP close response
    this->threadPool->awaitConnectionTasks(connectionId);

    HttpResponse closeResponse;
    closeResponse.setResponseHeader("Connection", "close");
    this->send(connectionId, closeResponse.serialize());

    int result = ::close(this->connections[connectionId]);
    if (isError(result)) {
        std::cerr << "[Server::close] Error: Failed to close connection with identifier '" << connectionId
                  << "'. Error code " << errno << "." << std::endl;
        return;
    }

    // TODO: clean up epoll stuff

    this->connections.erase(connectionId);
}

void Server::closeHost() {
    int result = ::close(this->hostSocket);
    if (isError(result)) {
        std::cerr << "[Server::~Server] Error: Failed to close server socket. Error code "
                  << errno << "." << std::endl;
        
        return;
    }

    this->isBound = false;
}

void Server::receive(const connection_id_t &connectionId) {
    if (!this->connections.contains(connectionId)) {
        std::cerr << "[Server::receive] Error: Connection with ID '" << connectionId
                  << "' doesn't exist." << std::endl;
        return;
    }

    this->threadPool->enqueueTask(connectionId, [this, &connectionId]() {
        std::string buffer(RECEIVE_BUFFER_SIZE, '\0');
        int result = recv(this->connections[connectionId], (char*)buffer.c_str(), RECEIVE_BUFFER_SIZE, 0);
        if (isError(result)) {
            std::cerr << "[Server::receive] Error: Failed to receive data over connection with ID '" << connectionId
                      << "'. Error code " << errno << std::endl;
           return;
        }

        std::cerr << "Received the following request:" << std::endl << buffer << "----------------------------" << std::endl;

        HttpRequest request(buffer);

        if (request.isCloseRequest || result == 0) {
            this->close(connectionId);
        } else {
            HttpResponse response = this->callback(request);
            this->send(connectionId, response.serialize());
        }
    });
}

std::string Server::receiveSync(const connection_id_t &connectionId) {
    if (!this->connections.contains(connectionId)) {
        std::cerr << "[Server::receiveSync] Error: Connection with ID '" << connectionId
                  << "' doesn't exist." << std::endl;
        return "";
    }

    std::string buffer(RECEIVE_BUFFER_SIZE, '\0');

    int result = recv(this->connections[connectionId], (char*)buffer.c_str(), RECEIVE_BUFFER_SIZE, 0);
    if (isError(result)) {
        std::cerr << "[Server::receiveSync] Error: Failed to receive data over connection with ID '" << connectionId
                    << "'. Error code " << errno << std::endl;
        return buffer;
    }

    return buffer;
}

void Server::send(const connection_id_t &connectionId, const std::string &data) {
    if (!this->connections.contains(connectionId)) {
        std::cerr << "[Server::sendText] Error: Connection with ID '" << connectionId
                  << "' doesn't exist." << std::endl;
        return;
    }

    int result = ::send(this->connections[connectionId], data.c_str(), data.length(), 0);
    if (isError(result)) {
        std::cerr << "[Server::sendText] Error: Failed to send data across connection with ID '"
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

    while (true) {
        int numEvents = epoll_wait(this->epollManager, this->epollEvents, this->maxConnections, -1); // wait indefinitely for an event
        if (isError(numEvents)) {
            std::cerr << "[Server::run] Error: Failed to wait for epoll events. Error code " << errno << std::endl;
            return;
        }

        for (int i = 0; i < numEvents; i++) {
            const auto event = this->epollEvents[i];
            const auto eventData = static_cast<epoll_argument_t*>(event.data.ptr);
            const auto eventSocket = event.data.fd;

            if (eventSocket == this->hostSocket) {
                // handle host socket
                this->accept(this->numConnections);
                this->numConnections++;
            } else {
                // handle other sockets
                // if the socket is readable, read the HTTP request and call the middleware.
                if (event.events & (EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                    this->receive(eventData->connectionId);
                }
            }
        }
    }
}