#ifndef WORKERTHREADPOOL_H
#define WORKERTHREADPOOL_H

#include <Poco/ThreadPool.h>


using Poco::ThreadPool;

class WorkerThreadPool {
private:

    WorkerThreadPool() {
        tp = new ThreadPool();
    }

    ~WorkerThreadPool() {
        tp->joinAll();
        delete tp;
    }
public:
    ThreadPool * tp;

    static WorkerThreadPool& getInstance() {
        static WorkerThreadPool instance;
        return instance;
    }
};

#endif /* WORKERTHREADPOOL_H */

