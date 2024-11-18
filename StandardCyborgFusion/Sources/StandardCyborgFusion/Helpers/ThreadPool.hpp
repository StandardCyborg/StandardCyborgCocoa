//
//  ThreadPool.hpp
//  StandardCyborgFusion
//
//  Created by Aaron Thompson on 8/28/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

#pragma once

#import <functional>
#import <iostream>
#import <mutex>
#import <queue>
#import <thread>

class ThreadPool
{
public:
    ThreadPool(int threadCount, qos_class_t qosClass);
    ~ThreadPool();
    
    void addJob(std::function<void (void)> function);
    
private:
    void _threadMain(int i);
    
    std::vector<std::thread> _threads;
    qos_class_t _qosClass;
    std::mutex _lock;
    std::condition_variable _semaphore;
    std::queue<std::function <void (void)>> _jobs;
    bool _shutdown = false;
};
