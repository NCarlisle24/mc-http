#include <server/server.hpp>
#include <http/request.hpp>

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
    if (!server->isBound) {
        return 1;
    }

    server->listen();
    if (!server->isListening) {
        return 1;
    }

    server->accept(0);

    std::string requestString = server->receive(0);
    server->send(0, "Hi there!");

    HttpRequest request(requestString);
    std::cerr << requestString << "\n--------------" << std::endl;
    request.print();
    std::cerr << "-----------" << std::endl;
    request.printQueryParameters();

    server->close(0);
    delete server;

    // cleanup
    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;
}