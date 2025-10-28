#include <server/server.hpp>

Server::Server(const char* const &ipAddress, const short &port) {
    this->ipAddress = ipAddress;
    this->port = port;

    // create the socket
    hostSocket = ::socket(AF_INET, SOCK_STREAM, 0); // IPv4, stream socket, TCP
    if (!isValidSocket(hostSocket)) {
        std::cerr << "[Server::Server] Error: Failed to create socket. Error code " << errno
                    << std::endl;
        return;
    }

    // bind the socket
    struct sockaddr_in serverInfo;
    serverInfo.sin_family = AF_INET; // IPv4
    serverInfo.sin_port = htons(port); // IPv4
    int result = inet_pton(AF_INET, ipAddress, &(serverInfo.sin_addr.s_addr));
    if (result < 0) {
        std::cerr << "[Server::Server] Error: Failed to convert IP address'" << ipAddress
                  << "' to network short. Error code " << errno << "." << std::endl;
        
        return;
    }

    result = bind(hostSocket, (struct sockaddr*)&serverInfo, sizeof(serverInfo));
    if (isError(result)) {
        std::cerr << "[Server::Server] Error: Failed to bind socket on address'" << ipAddress
                    << ":" << port << "'. Error code " << errno << "." << std::endl;
        return;
    }
}

void Server::listen(const unsigned int &maxConnections) {
    int result = ::listen(hostSocket, maxConnections);
    if (isError(result)) {
        std::cerr << "[Server::listen] Error: Failed to listen to socket on address'" << ipAddress
                    << ":" << port << "'. Error code " << errno << "." << std::endl;
        return;
    }
}

void Server::accept(const connectionId_t &clientId) {
    // check if connection exists
    if (connections.contains(clientId)) {
        std::cerr << "[Server::accept] Warning: Client with ID '" << clientId
                  << "' already exists. Closing old connection." << std::endl;
        
        close(clientId);
    }

    // accept the connection
    struct sockaddr_in clientInfo;
    socklen_t clientInfoSize;

    socket_t clientSocket = ::accept(hostSocket, (struct sockaddr*)&clientInfo, &clientInfoSize);
    if (!isValidSocket(clientSocket)) {
        std::cerr << "[Server::accept] Error: Failed to accept connection with identifier '" << clientId
                  << "'. Error code " << errno << "." << std::endl;

        return;
    }

    connections.insert({clientId, clientSocket});
}

void Server::close(const connectionId_t &clientId) {
    int result = ::CLOSESOCKET(connections[clientId]);
    if (isError(result)) {
        std::cerr << "[Server::close] Error: Failed to close connection with identifier '" << clientId
                  << "'. Error code " << errno << "." << std::endl;
        
        return;
    }
}