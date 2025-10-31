#pragma once

#include <http/request.hpp>
#include <http/response.hpp>
#include <util/connection.hpp>
#include <util/threadpool.hpp>

// simple unix/posix! :DDD
#include <arpa/inet.h>
#include <errno.h>
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
#define RECEIVE_BUFFER_SIZE 1024
#define INVALID_SOCKET -1

typedef int socket_t;
typedef std::function<HttpResponse(const HttpRequest&)> server_callback_t;
// TODO: make this accept any request, not just HTTP

inline bool isValidSocket(socket_t sock) {
    return sock >= 0;
}

inline bool isError(int result) {
    return result < 0;
}

class Server {
    public:
        socket_t hostSocket;
        std::unordered_map<connectionId_t, socket_t> connections;
        std::string ipAddress;
        ThreadPool* threadPool;
        struct epoll_event* epollEvents;
        short port;
        bool isBound = false;
        bool isListening = false;
        bool isAccepting = false;

        Server(const std::string &ipAddress, const short &port);
        ~Server();
        void listen(const size_t &numThreads = DEFAULT_NUM_THREADS,
                    const size_t &tasksQueueSize = DEFAULT_TASKS_QUEUE_SIZE,
                    const size_t &listeningQueueSize = DEFAULT_LISTENING_QUEUE_SIZE);
        void accept(const connectionId_t &connectionId);
        void close(const connectionId_t &connectionId);
        void closeSelf();
        std::string receiveSync(const connectionId_t &connectionId);
        void receive(const connectionId_t &connectionId, const server_callback_t &callback);
        void send(const connectionId_t &connectionId, const std::string &data);
        void run();
};