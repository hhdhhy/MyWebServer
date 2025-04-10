#include "Loopthreadpool.h"
Loopthreadpool::Loopthreadpool(int num)
:num_(num),
loopthreads_(num),
loops_(num)
{
}

Loopthreadpool::~Loopthreadpool()
{
    for(int i=0;i<num_;i++)
    {
        delete loopthreads_[i];
    }
}

void Loopthreadpool::start()
{
    for(int i=0;i<num_;i++)
    {
        loopthreads_[i] = new Loopthread();
        loops_[i] = loopthreads_[i]->get_loop();
    }
}
Loop* Loopthreadpool::get_next_loop()
{
    ++next_;
    return loops_[(next_%=num_)];
}