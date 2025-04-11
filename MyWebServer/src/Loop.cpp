#include "Loop.h"
#include <thread>
#include <unistd.h>
#include <sys/eventfd.h>
#include "Loger.h"
#include "Channel.h"
#include "Epoll.h"
#include <cstring>
thread_local Loop* is_oneloop_onethread = nullptr;

Loop::Loop()
: epoll_(new Epoll),
running_(false),
thread_id_(std::this_thread::get_id()),
wakeup_channel_(this, get_wakeup_fd()) 
{
    LOG_INFO << "Initializing Loop object...";
    if (is_oneloop_onethread == nullptr)
    {
        is_oneloop_onethread = this;
    } 
    else 
    {
        LOG_FATAL << "Only one loop allowed per thread. Exiting...";
    }
    timequeue_.reset(new Timequeue(this));
    wakeup_channel_.enable_read();
    wakeup_channel_.set_callback_read(std::bind(&Loop::handle_getup, this));
    // epoll_->add_channel(&wakeup_channel_);
}

Loop::~Loop() 
{
    LOG_INFO << "Destroying Loop object...";
    is_oneloop_onethread = nullptr;
}

void Loop::run() 
{
    LOG_INFO << "Starting event loop...";
    if (!is_loop_in_thread()) 
    {
        LOG_ERROR << "Event loop is not running in the correct thread. Exiting...";
        std::exit(-1);
    }

    running_ = true;
    while (!stop_) 
    {
        active_channels_.clear();
        epoll_->poll(-1, active_channels_);
        for (auto channel : active_channels_) 
        {
            channel->handle_all();
        }
        run_wait_callbacks();
    }
    running_ = false;
    LOG_INFO << "Event loop stopped.";
}

void Loop::stop() 
{
    LOG_INFO << "Stopping event loop...";
    stop_ = true;
    if (!is_loop_in_thread()) 
    {
        wakeup();
    }
}

void Loop::add_channel(Channel* channel) 
{
    if (!is_loop_in_thread()) 
    {
        LOG_ERROR << "Attempted to add channel from a different thread. Exiting...";
        std::exit(-1);
    }
    if (!is_channel_in_loop(channel)) 
    {
        LOG_ERROR << "Channel does not belong to this loop. Exiting...";
        std::exit(-1);
    }
    epoll_->add_channel(channel);
}

void Loop::remove_channel(Channel* channel)
 {
    if (!is_loop_in_thread()) 
    {
        LOG_ERROR << "Attempted to remove channel from a different thread. Exiting...";
        std::exit(-1);
    }
    if (!is_channel_in_loop(channel)) 
    {
        LOG_ERROR << "Channel does not belong to this loop. Exiting...";
        std::exit(-1);
    }
    epoll_->delete_channel(channel);
}

void Loop::update_channel(Channel* channel) 
{
    if (!is_loop_in_thread())
    {
        LOG_ERROR << "Attempted to update channel from a different thread. Exiting...";
        std::exit(-1);
    }
    if (!is_channel_in_loop(channel)) 
    {
        LOG_ERROR << "Channel does not belong to this loop. Exiting...";
        std::exit(-1);
    }
    epoll_->update_channel(channel);
}

bool Loop::is_loop_in_thread() 
{
    auto pid=std::this_thread::get_id();
    return thread_id_ == pid;
}

bool Loop::is_channel_in_loop(Channel* channel) 
{
    return channel->get_loop() == this;
}

void Loop::add_timer(Timer::Timeus timeout, Timer::callback_function callback, Timer::Timeus interval) 
{
    LOG_INFO << "Adding timer with timeout: " << timeout << ", interval: " << interval;
    timequeue_->add_timer(timeout, callback, interval);
}

void Loop::add_run_callback(const Timer::callback_function& cbf) 
{
    if (is_loop_in_thread()) 
    {
        cbf();
    } 
    else 
    {
        {
            std::lock_guard<std::mutex> lock(wait_callbacks_mutex_);
            LOG_DEBUG << "Adding run callback...";
            wait_callbacks_.push_back(cbf);
            LOG_DEBUG << "wait_callbacks_.size():"<<wait_callbacks_.size();

        }
        if (!is_loop_in_thread() || has_wait_callbacks_) 
        {
            LOG_DEBUG << "wait_callbacks_.size():"<<wait_callbacks_.size();
            handle_wakeup() ;
        }
    }
}

void Loop::run_wait_callbacks() 
{
    if (!is_loop_in_thread())
    {
        LOG_ERROR << "Attempted to update channel from a different thread. Exiting...";
        std::exit(-1);
    }
    LOG_INFO << "Running pending callbacks...";
    has_wait_callbacks_ = true;
    LOG_DEBUG<<"loop:"<<wakeup_channel_.get_fd();
    LOG_DEBUG<< "Callbacks size:"<<wait_callbacks_.size();
    
    std::vector<callback_function> callbacks;
    {
        std::lock_guard<std::mutex> lock(wait_callbacks_mutex_);
        swap(callbacks, wait_callbacks_);
    }
    LOG_DEBUG<< "Callbacks size:"<<callbacks.size();
    for (auto& cbf : callbacks) 
    {
        LOG_DEBUG << "Executing callback..."<<cbf.target_type().hash_code();
        cbf();
    }
    has_wait_callbacks_ = false;
    LOG_INFO << "All pending callbacks executed.";
}

int Loop::get_wakeup_fd() {
    
    int fd = eventfd(0, EFD_CLOEXEC);
    if (fd < 0) 
    {
        LOG_ERROR << "Failed to create eventfd. Exiting...";
        std::exit(-1);
    }
    LOG_INFO << "Creating wakeup eventfd..fd:"<<fd;

    return fd;
}
void Loop::handle_getup() 
{
    uint64_t p = 1;
    int len = read(wakeup_channel_.get_fd(), &p, sizeof(p));
    if (len != sizeof(p))
    {
        LOG_ERROR<<"EventLoop::handleRead() reads"<<len<<"bytes instead of 8";
    }
}

void Loop::handle_wakeup() 
{
    if (wakeup() <= 0) 
    {
        LOG_ERROR << "Failed to handle wakeup. Exiting...";
        std::exit(-1);
    }
}

int Loop::wakeup() 
{
    uint64_t p = 1;
    int len;
    LOG_DEBUG<< "Waking up event loop... fd:"<<wakeup_channel_.get_fd();
    if ((len = write(wakeup_channel_.get_fd(), &p, sizeof(p))) <0) 
    {
      
        LOG_ERROR << "Write error occurred during wakeup."<<strerror(errno) ;
    }
    return len;
}