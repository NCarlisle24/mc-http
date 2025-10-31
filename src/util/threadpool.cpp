#include <util/threadpool.hpp>

static void* workerThread(void* argument) {
    while (true) {
        ThreadPool* threadPool = static_cast<ThreadPool*>(argument);

        pthread_mutex_lock(&(threadPool->tasksMutex));

        while (threadPool->tasks.size() == 0) { // while loop to handle spurious wake ups
            pthread_cond_wait(&(threadPool->tasksCondition), (&threadPool->tasksMutex));
        }

        Task task = threadPool->tasks.front();
        threadPool->tasks.pop_front();

        pthread_mutex_unlock(&(threadPool->tasksMutex));

        task.function();
    }

    return NULL;
}

ThreadPool::ThreadPool(const size_t &numThreads, const size_t &queueSize) {
    this->threads = (pthread_t*)malloc(numThreads * sizeof(pthread_t));
    this->maxTasks = queueSize;

    for (size_t i = 0; i < numThreads; i++) {
        pthread_create(this->threads + i, NULL, workerThread, this);
    }
}

ThreadPool::~ThreadPool() {
    // TODO: free the threads

    free(this->threads);
}

bool ThreadPool::enqueueTask(const connectionId_t &connectionId, const task_function_t &taskFunction) {
    if (tasks.size() >= this->maxTasks) {
        std::cerr << "[ThreadPool::enqueueTask] Warning: Could not enqueue task for connection ID '"
            << connectionId << "' because queue is full." << std::endl;
        return false;
    }

    pthread_mutex_lock(&tasksMutex);
    tasks.push_back((Task){connectionId, taskFunction});
    pthread_cond_signal(&tasksCondition);
    pthread_mutex_unlock(&tasksMutex);

    return true;
}

void ThreadPool::removeConnectionTasks(const connectionId_t &connectionId) {
    // TODO: make this thread run at max priority
    pthread_mutex_lock(&tasksMutex);

    tasks.erase(std::remove_if(tasks.begin(), tasks.end(),
        [&connectionId](Task task) { return task.connectionId == connectionId; }),
        tasks.end());

    pthread_mutex_unlock(&tasksMutex);
}