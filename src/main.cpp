#include <server/server.hpp>

int main() {
    // if on windows, initialize winsock
    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            perror("[main] Failed to initialize Winsock.\n");
            return 1;
        }
    #endif

    // do stuff
    socket_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        perror("[main] Failed to create socket.\n");
        return 1;
    }


    // cleanup
    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;
}