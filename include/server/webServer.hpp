#include <server/server.hpp>

#include <functional>

class WebServer : public Server {
    public:
        void addPath(const std::string &path, std::function middleware);
        void run(const short &port);
}