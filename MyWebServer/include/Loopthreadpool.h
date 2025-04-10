#pragma once
#include "Loopthread.h"

class Loopthreadpool
{

public:

    Loopthreadpool(int num);
    ~Loopthreadpool();
    Loop* get_next_loop();
    void start();
private:
    
    std::vector<Loopthread*> loopthreads_;
    std::vector<Loop*> loops_;
    int next_;
    int num_;
};

