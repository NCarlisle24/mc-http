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
    Server* server = new Server("127.0.0.1", 3000);
    server->listen();
    server->accept(0);

    // cleanup
    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;
}