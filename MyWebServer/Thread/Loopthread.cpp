#include<Loopthread.h>

Loopthread::Loopthread()
:thread_(std::make_shared<std::thread>([this](){run_loop();}))
{

}

Loopthread::~Loopthread()
{
    if(loop_)
    {
        loop_->stop();
        thread_->join();
    }
}

std::shared_ptr<Loop> Loopthread::get_loop()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this](){return loop_.get() != nullptr;});
    }
    return loop_;
}

void Loopthread::run_loop()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_=std::make_shared<Loop>();
        cond_.notify_all();
    }
    loop_->run();
    std::unique_lock<std::mutex> lock(mutex_);
    loop_.reset();
}
