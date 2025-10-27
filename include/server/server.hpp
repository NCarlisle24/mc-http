#pragma once

#include <http/request.hpp>
#include <http/response.hpp>

#ifdef _WIN32
    // nasty windows stuff
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>

    typedef SOCKET socket_t;
    #define CLOSESOCKET closesocket
#else
    // simple unix/posix! :DDDD
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>

    typedef int socket_t;
    #define CLOSESOCKET close
    #define INVALID_SOCKET -1
#endif

#include <stdio.h>

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