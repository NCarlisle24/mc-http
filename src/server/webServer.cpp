#include <server/webServer.hpp>

void WebServer::addRoute(const std::string &path, const ws_callback_t &middleware) {
    this->routes.insert({path, middleware});
}

void WebServer::run() {
    this->listen();
    while (true) {
        // use epoll to monitor both the original server socket and all other connections
    }
}