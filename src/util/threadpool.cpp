#include <util/threadpool.hpp>

static void* workerThread(void* argument) {
    while (true) {
        ThreadPool* threadPool = static_cast<ThreadPool*>(argument);

        pthread_mutex_lock(&(threadPool->tasksMutex));

        while (threadPool->tasks.size() == 0) { // while loop to handle spurious wake ups
            pthread_cond_wait(&(threadPool->tasksCondition), (&threadPool->tasksMutex));
        }

        // check for shut down
        if (threadPool->isShuttingDown && threadPool->tasks.size() == 0) {
            pthread_mutex_unlock((&threadPool->tasksMutex));
            pthread_exit(NULL);
        }

        // get the task and run it
        auto task = threadPool->nextTask;
        task->isRunning = true;
        (threadPool->nextTask)++;

        pthread_mutex_unlock(&(threadPool->tasksMutex));

        task->function();

        // remove the completed task from the list
        pthread_mutex_lock(&(threadPool->tasksMutex));
        task->isRunning = false;
        task->isComplete = true;
        threadPool->tasks.erase(task);
        pthread_mutex_unlock(&(threadPool->tasksMutex));
    }

    return NULL;
}

ThreadPool::ThreadPool(const size_t &inputNumThreads, const size_t &queueSize) {
    this->threads = (pthread_t*)malloc(inputNumThreads * sizeof(pthread_t));
    this->numThreads = inputNumThreads;
    this->maxTasks = queueSize;

    for (size_t i = 0; i < numThreads; i++) {
        pthread_create(this->threads + i, NULL, workerThread, this);
    }
}

ThreadPool::~ThreadPool() {
    this->shutdown();
}

bool ThreadPool::enqueueTask(const connectionId_t &connectionId, const task_function_t &taskFunction) {
    if (this->tasks.size() >= this->maxTasks) {
        std::cerr << "[ThreadPool::enqueueTask] Warning: Could not enqueue task for connection ID '"
            << connectionId << "' because queue is full." << std::endl;
        return false;
    }

    pthread_mutex_lock(&(this->tasksMutex));

    this->tasks.push_back((Task){connectionId, taskFunction, false, false});
    if (this->nextTask == tasks.end()) {
        this->nextTask = tasks.begin();
    }
    
    pthread_cond_signal(&(this->tasksCondition));
    pthread_mutex_unlock(&(this->tasksMutex));

    return true;
}

void ThreadPool::awaitConnectionTasks(const connectionId_t &connectionId) {
    pthread_mutex_lock(&(this->tasksMutex));

    std::vector<std::list<Task>::iterator> remainingTasks;

    std::list<Task>::iterator it = this->tasks.begin();
    while (it != this->tasks.end()) {
        if (it->connectionId == connectionId) remainingTasks.push_back(it);
        it++;
    }

    pthread_mutex_unlock(&(this->tasksMutex));

    if (remainingTasks.size() > 0)  {
        // TODO: optimize this
        // while there exists a task that isn't complete, wait
        while (std::find_if(remainingTasks.begin(), remainingTasks.end(),
            [](auto task){ return !(task->isComplete); }) != remainingTasks.end()) {};
    }
}

void ThreadPool::shutdown() {
    pthread_mutex_lock(&(this->tasksMutex));
    this->isShuttingDown = true;

    pthread_cond_broadcast(&(this->tasksCondition));
    pthread_mutex_unlock(&(this->tasksMutex));

    for (size_t i = 0; i < this->numThreads; i++) {
        pthread_join(this->threads[i], NULL);
    }

    free(this->threads);
    pthread_mutex_destroy(&(this->tasksMutex));
    pthread_cond_destroy(&(this->tasksCondition));
}