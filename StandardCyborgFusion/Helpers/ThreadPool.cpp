//
//  ThreadPool.cpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/28/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#include "ThreadPool.hpp"
#include <pthread/qos.h>

ThreadPool::ThreadPool(int threadCount, qos_class_t qosClass)
{
    _qosClass = qosClass;
    
    for (int i = 0; i < threadCount; ++i)
    {
        _threads.emplace_back(std::bind(&ThreadPool::_threadMain, this, i));
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(_lock);
        _shutdown = true;
        _semaphore.notify_all();
    }
    
    // Wait for all threads to stop
    for (auto& thread : _threads)
    {
        thread.join();
    }
}

void ThreadPool::addJob(std::function<void (void)> function)
{
    std::unique_lock<std::mutex> lock(_lock);
    
    _jobs.emplace(std::move(function));
    
    _semaphore.notify_one();
}

void ThreadPool::_threadMain(int i)
{
    pthread_set_qos_class_self_np(_qosClass, 0);
    
    std::function<void (void)> job;
    
    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(_lock);
            
            while (!_shutdown && _jobs.empty())
            {
                _semaphore.wait(lock);
            }
            
            if (_jobs.empty()) { return; }
            
            job = std::move(_jobs.front());
            _jobs.pop();
        }
        
        job();
    }
}
