#include <server/webServer.hpp>

void WebServer::addRoute(const std::string &path, const ws_callback_t &middleware) {
    this->routes.insert({path, middleware});
}