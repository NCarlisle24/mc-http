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
    Server* server = new Server("0.0.0.0", 3000);
    server->listen();
    server->accept(0);
    std::string response = server->receive(0);
    server->send(0, "Hi there!");

    std::cout << response;

    delete server;

    // cleanup
    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;
}