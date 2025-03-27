#include "Loop.h"
#include <thread>

thread_local Loop* is_oneloop_onethread = nullptr;
Loop::Loop()
:epoll_(new Epoll),running_(false),thread_id_(std::this_thread::get_id())
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
    if(!is_in_thread())
    {
        throw std::runtime_error("loop is not in thread");
    }

    running_ = true;
    while (!stop_)
    {
        active_channels.clear();
        epoll_->poll(MAX_EPOLL_TIME,active_channels);
        for (auto channel : active_channels)
        {
            channel->callback_all();
        }
    }
    running_ = false;
}

void Loop::add_channel(Channel *channel)
{
    if(!is_in_thread())
    {
        throw std::runtime_error("loop is not in thread");
    }
    if(!is_channel_in_loop(channel))
    {
        throw std::runtime_error("channel is not in loop");
    }
    epoll_->add_channel(channel);
}

void Loop::remove_channel(Channel *channel)
{
    if(!is_in_thread())
    {
        throw std::runtime_error("loop is not in thread");
    }
    if(!is_channel_in_loop(channel))
    {
        throw std::runtime_error("channel is not in loop");
    }
    epoll_->delete_channel(channel);
}

void Loop::update_channel(Channel *channel)
{
    if(!is_in_thread())
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

bool Loop::is_channel_in_loop(Channel *channel)
{
    return channel->get_loop() == this;
}
