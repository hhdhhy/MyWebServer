#pragma once 
#include "Loop.h"
#include <condition_variable>

class Loopthread
{

public:
    Loopthread();
    ~Loopthread();

    void run_loop();// 线程函数启动loop
    
    Loop* get_loop();
private:
    

    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    Loop* loop_;//在临界区中得加锁
};


