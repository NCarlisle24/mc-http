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
    connection_id_t connectionId;
    task_function_t function;
    bool isRunning = false;
    bool isComplete = false;
} Task;

class ThreadPool {
    public:
        pthread_t* threads;
        size_t numThreads;
        std::list<Task> tasks;
        std::list<Task>::iterator nextTask = tasks.begin();
        size_t maxTasks;
        pthread_mutex_t tasksMutex;
        pthread_cond_t tasksCondition;
        bool isShuttingDown = false;

        ThreadPool(const size_t &numThreads = 10, const size_t &queueSize = 10);
        ~ThreadPool();
        bool enqueueTask(const connection_id_t &connectionId, const task_function_t &taskFunction);
        void awaitConnectionTasks(const connection_id_t &connectionId);
        void shutdown();
};