#pragma once

#include <server/server.hpp>
#include <http/request.hpp>

#include <functional>
#include <csignal>

typedef std::function<HttpResponse(HttpRequest)> ws_callback_t;

class WebServer : public Server {
    public:
        std::unordered_map<std::string, ws_callback_t> routes;
        
        WebServer(const std::string &ipAddress, const short &port) : Server(ipAddress, port) {};
        void addRoute(const std::string &path, const ws_callback_t &middleware);
};