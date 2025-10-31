#include <server/webServer.hpp>
#include <http/request.hpp>

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 3000

Server* server;

void INThandler(int sig) {
    signal(sig, SIG_IGN); // ignore the signal

    std::cerr << "\nShutting down..." << std::endl;

    delete server;

    exit(0);
}

int main() {
    signal(SIGINT, INThandler);

    // do stuff
    server = new Server(SERVER_IP_ADDRESS, SERVER_PORT);
    if (!server->isBound) return 1;
    
    server->setCallback([](const HttpRequest &request) {
        std::cout << "Connected." << std::endl;
        request.print();

        HttpResponse placeholder;
        return placeholder;
    });
    
    server->run();

    delete server;
    return 0;
}