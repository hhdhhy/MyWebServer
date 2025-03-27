#pragma once 
#include "Loop.h"
#include <condition_variable>

class Loopthread
{

public:
    Loopthread();
    ~Loopthread();

    void run_loop();// 线程函数启动loop
    
    std::shared_ptr<Loop> get_loop();
private:
    

    std::shared_ptr<std::thread> thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::shared_ptr<Loop> loop_;//在临界区中得加锁
};


