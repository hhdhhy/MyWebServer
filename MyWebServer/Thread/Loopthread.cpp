#include<Loopthread.h>

Loopthread::Loopthread()
:thread_([this](){run_loop();}),loop_(nullptr)
{

}

Loopthread::~Loopthread()
{
    if(loop_)
    {
        loop_->stop();
        thread_.join();
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
    Loop loop;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_=&loop;
        cond_.notify_all();
    }
    loop_->run();
    std::unique_lock<std::mutex> lock(mutex_);

    loop_=nullptr;
}
