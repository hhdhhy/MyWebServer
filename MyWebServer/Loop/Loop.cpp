#include "Loop.h"
#include <thread>
#include <unistd.h>
#include <sys/eventfd.h>

thread_local Loop* is_oneloop_onethread = nullptr;
Loop::Loop()
:epoll_(new Epoll),
running_(false),
thread_id_(std::this_thread::get_id()),
timequeue_(new Timequeue(this)),
wakeup_channel_(this,get_wakeup_fd())
{
    if(is_oneloop_onethread == nullptr)
    {
        is_oneloop_onethread = this;
    }
    else
    {
        throw std::runtime_error("only one loop on one thread");
    }
}

Loop::~Loop()
{
    is_oneloop_onethread = nullptr;
}

void Loop::run()
{
    if(!is_loop_in_thread())
    {
        throw std::runtime_error("loop is not in thread");
    }

    running_ = true;
    while (!stop_)
    {
        active_channels_.clear();
        epoll_->poll(MAX_EPOLL_TIME,active_channels_);
        for (auto channel : active_channels_)
        {
            channel->handle_all();
        }
        run_wait_callbacks();
    }
    running_ = false;
}

void Loop::stop()
{
    stop_ = true;
    if(!is_loop_in_thread())
    {
        wakeup();
    }
}

void Loop::add_channel(Channel*channel)
{
    if(!is_loop_in_thread())
    {
        throw std::runtime_error("loop is not in thread");
    }
    if(!is_channel_in_loop(channel))
    {
        throw std::runtime_error("channel is not in loop");
    }
    epoll_->add_channel(channel);
}

void Loop::remove_channel(Channel*channel)
{
    if(!is_loop_in_thread())
    {
        throw std::runtime_error("loop is not in thread");
    }
    if(!is_channel_in_loop(channel))
    {
        throw std::runtime_error("channel is not in loop");
    }
    epoll_->delete_channel(channel);
}

void Loop::update_channel(Channel*channel)
{
    if(!is_loop_in_thread())
    {
        throw std::runtime_error("loop is not in thread");
    }
    if(!is_channel_in_loop(channel))
    {
        throw std::runtime_error("channel is not in loop");
    }
    
    epoll_->update_channel(channel);
}

bool Loop::is_loop_in_thread()
{
    return thread_id_==std::this_thread::get_id();
}

bool Loop::is_channel_in_loop(Channel*channel)
{
    return channel->get_loop() == this;
}

void Loop::add_timer(Timer::Timeus timeout,Timer::callback_function callback,Timer::Timeus interval)
{
   timequeue_->add_timer(timeout,callback,interval);
}

void Loop::add_run_callback(const Timer::callback_function &cbf)
{
    if(is_loop_in_thread())
    {
        cbf();
    }
    else
    {
        {
            std::lock_guard<std::mutex> lock(wait_callbacks_mutex_);
            wait_callbacks_.push_back(cbf);
        }
        if(!is_loop_in_thread()||has_wait_callbacks_)
        {
            wakeup();
        }
    }
}

void Loop::run_wait_callbacks()
{
    has_wait_callbacks_ = true;
    std::vector<callback_function> callbacks;
    {
        std::lock_guard<std::mutex> lock(wait_callbacks_mutex_);
        swap(callbacks,wait_callbacks_);
    }

    for(auto &cbf:callbacks)
    {
        cbf();
    }
    has_wait_callbacks_ = false;
}

int Loop::get_wakeup_fd()
{
    int fd= eventfd(0,EFD_NONBLOCK|EFD_CLOEXEC);
    if(fd<0)
    {
        throw std::runtime_error("eventfd error");
    }
    return fd;
}

void Loop::handle_wakeup()
{
    if(wakeup()<=0)
    {
        throw std::runtime_error("wakeup error");
    }
}


int Loop::wakeup()
{
    char p=1;
    int len;
    if((len=write(wakeup_channel_.get_fd(),&p,sizeof(p)))<=0)
    {
        throw std::runtime_error("wakeup error");
    }
    return len;
}
