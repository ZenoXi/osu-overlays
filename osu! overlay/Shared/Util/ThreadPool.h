#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>
#include <memory>

class ThreadPool
{
public:
    struct ThreadData
    {
        std::thread handle;
        std::function<void(std::atomic<bool>* stopFlag)> task;
        std::atomic<bool> taskRunning = false;
        std::atomic<bool> stopFlag = false;

        void DoWork(std::function<void(std::atomic<bool>* stopFlag)> runnable)
        {
            if (taskRunning.load())
                return;

            task = std::move(runnable);
            taskRunning.store(true);
        }
    };

    ThreadPool() {}
    ~ThreadPool()
    {
        for (auto& thread : _threads)
            thread->stopFlag.store(true);
        for (auto& thread : _threads)
            if (thread->handle.joinable())
                thread->handle.join();
    }

    void AddThread()
    {
        auto thread = std::make_unique<ThreadData>();
        thread->handle = std::thread(&ThreadPool::_WorkerThread, this, thread.get());
        _threads.push_back(std::move(thread));
    }

    ThreadData* GetThread(int index)
    {
        return _threads[index].get();
    }

    int ThreadCount()
    {
        return _threads.size();
    }

private:
    std::vector<std::unique_ptr<ThreadData>> _threads;

    void _WorkerThread(ThreadData* data)
    {
        while (!data->stopFlag.load())
        {
            if (data->taskRunning.load())
            {
                data->task(&data->stopFlag);
                data->taskRunning.store(false);
            }
        }
    }
};