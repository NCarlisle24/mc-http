#include <server/webServer.hpp>
#include <http/request.hpp>

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 3000

WebServer* server;

void INThandler(int sig) {
    signal(sig, SIG_IGN); // ignore the signal

    std::cerr << "Shutting down server..." << std::endl;

    shutdown(server->hostSocket, SHUT_RDWR);
    delete server;

    exit(0);
}

int main() {
    // do stuff
    server = new WebServer(SERVER_IP_ADDRESS, SERVER_PORT);

    server->addRoute("/", [](const HttpRequest &request) {
        std::cout << "Connected." << std::endl;
        request.print();

        HttpResponse placeholder;
        return placeholder;
    });
    server->run();

    delete server;
    return 0;
}