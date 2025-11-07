#pragma once

#include <http/request.hpp>
#include <http/response.hpp>
#include <util/connection.hpp>
#include <util/threadpool.hpp>

// simple unix/posix! :DDD
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>

#define DEFAULT_LISTENING_QUEUE_SIZE 20
#define DEFAULT_TASKS_QUEUE_SIZE 20
#define DEFAULT_NUM_THREADS 20
#define DEFAULT_MAX_CONNECTIONS 30
#define RECEIVE_BUFFER_SIZE 1024
#define INVALID_SOCKET -1

typedef int socket_t;
// TODO: make this accept any request, not just HTTP
typedef std::function<HttpResponse(const HttpRequest&)> server_callback_t;
typedef struct {
    connection_id_t connectionId;
} epoll_argument_t;

static inline bool isValidSocket(socket_t sock) {
    return sock >= 0;
}

static inline bool isError(int result) {
    return result < 0;
}

static inline void makeSocketNonBlocking(socket_t socket) {
    int flags = fcntl(socket, F_GETFL, 0);
    if (isError(flags)) {
        std::cerr << "[makeSocketNonBlocking] Error: Failed to get socket flags via fcntl. Error code " << errno << std::endl;
        return;
    }

    flags |= O_NONBLOCK;
    int result = fcntl(socket, F_SETFL, flags);
    if (isError(result)) {
        std::cerr << "[makeSocketNonBlocking] Error: Failed to set socket flags via fcntl. Error code " << errno << std::endl;
    }

    return;
}

// TODO: separate public and private stuff
class Server {
    public:
        socket_t hostSocket;
        // TODO: change how connections are stored so that old IDs can be used
        std::unordered_map<connection_id_t, socket_t> connections;
        std::string ipAddress;
        ThreadPool* threadPool;
        int epollManager;
        struct epoll_event* epollEvents;
        server_callback_t callback;
        short port;
        size_t maxConnections;
        size_t numConnections = 0;
        bool isBound = false;
        bool isListening = false;
        bool isAccepting = false;

        Server(const std::string &ipAddress, const short &port);
        ~Server();
        void listen(const size_t &numThreads = DEFAULT_NUM_THREADS,
                    const size_t &tasksQueueSize = DEFAULT_TASKS_QUEUE_SIZE,
                    const size_t &listeningQueueSize = DEFAULT_LISTENING_QUEUE_SIZE,
                    const size_t &maxConnections = DEFAULT_MAX_CONNECTIONS);
        void accept(const connection_id_t &connectionId);
        void close(const connection_id_t &connectionId);
        void closeHost();
        std::string receiveSync(const connection_id_t &connectionId);
        void receive(const connection_id_t &connectionId);
        void send(const connection_id_t &connectionId, const std::string &data);
        void setCallback(const server_callback_t &inputCallback);
        void run();
    
    private:

};