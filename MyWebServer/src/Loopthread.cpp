#include<Loopthread.h>
#include"Loger.h"
Loopthread::Loopthread()
:thread_([this](){run_loop();}),loop_(nullptr)
{

}

Loopthread::~Loopthread()
{
    if(loop_)
    {
        LOG_INFO << " Stopping event loop...";
        loop_->stop();
        thread_.join();
        LOG_INFO << " Thread joined successfully";
    }
}

Loop* Loopthread::get_loop()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this](){return loop_ != nullptr;});
    }
    
    return loop_;
}

void Loopthread::run_loop()
{
    LOG_INFO << "geting loop in thread";
    Loop loop;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_=&loop;
        cond_.notify_all();
    }
    LOG_INFO << "run_loop";
    loop_->run();
    std::unique_lock<std::mutex> lock(mutex_);

    loop_=nullptr;
}
