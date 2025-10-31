#pragma once

#include <util/connection.hpp>
#include <http/request.hpp>
#include <http/response.hpp>

#include <algorithm>
#include <functional>
#include <pthread.h>
#include <list>
#include <vector>

typedef std::function<void()> task_function_t;

typedef struct {
    connectionId_t connectionId;
    task_function_t function;
} Task;

class ThreadPool {
    public:
        pthread_t* threads;
        // TODO: remove tasks when a connection goes away
        std::list<Task> tasks;
        size_t maxTasks;
        pthread_mutex_t tasksMutex;
        pthread_cond_t tasksCondition;

        ThreadPool(const size_t &numThreads = 10, const size_t &queueSize = 10);
        ~ThreadPool();
        bool enqueueTask(const connectionId_t &connectionId, const task_function_t &taskFunction);
        void removeConnectionTasks(const connectionId_t &connectionId);
};