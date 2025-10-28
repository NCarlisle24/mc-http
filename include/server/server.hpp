#pragma once

#include <http/request.hpp>
#include <http/response.hpp>

// OS-specific includes
#ifdef _WIN32
    // nasty windows stuff
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>

    typedef SOCKET socket_t;
    #define CLOSESOCKET closesocket
#else
    // simple unix/posix! :DDD
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/ip.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>

    typedef int socket_t;
    #define CLOSESOCKET close
    #define INVALID_SOCKET -1
#endif

// Actual stuff
#include <iostream>
#include <unordered_map>
#include <stdbool.h>

#define DEFAULT_MAX_CONNECTIONS 20

typedef unsigned int connectionId_t;

inline bool isValidSocket(socket_t sock) {
    #ifdef _WIN32
        return sock != INVALID_SOCKET;
    #else
        return sock >= 0;
    #endif
}

inline bool isError(int result) {
    #ifdef _WIN32
        return result == SOCKET_ERROR;
    #else
        return result < 0;
    #endif
}

class Server {
    public:
        socket_t hostSocket;
        std::unordered_map<connectionId_t, socket_t> connections;
        std::string ipAddress;
        short port;

        Server(const char* const &ipAddress, const short &port);
        void listen(const unsigned int &maxConnections = DEFAULT_MAX_CONNECTIONS);
        void accept(const connectionId_t &clientId);
        void close(const connectionId_t &clientId);
};