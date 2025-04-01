#include"Channel.h"
#include<unistd.h>
#include"Loger.h"
Channel::Channel(Loop* loop, int fd, int event=NONE_EVENT)
: loop_(loop),fd_(fd),events_(event),is_in_epoll_(false)
{
}

Channel::~Channel()
{
    if(is_in_epoll_)
    disable_all();
    if(is_calling_)
    {
        LOG_FATAL<<"Channel is calling";
    }
    if(fd_>=0)
    {
        int ret=close(fd_);
        if(ret<0)
        {
            LOG_ERROR<<" close fd error";
        }
    }
    
}

void Channel::set_callback_read(const callback_function &callback)
{
    callback_read_ = callback;
}

void Channel::set_callback_write(const callback_function &callback)
{
    callback_write_ = callback;
}

void Channel::set_callback_close(const callback_function &callback)
{
    callback_close_ = callback;
}

void Channel::set_callback_error(const callback_function &callback)
{
    callback_error_ = callback;
}

void Channel::set_events(const int &event)
{
    events_ = event;
    update_epoll();
}

void Channel::set_revents(const int &revent)
{
    revents_ = revent;
}

int Channel::get_events()
{
    return events_;
}

int Channel::get_revents()
{
    return revents_;
}

int Channel::get_fd()
{
    return fd_;
}
void Channel::handle_do_all()
{
    if(tie_)
    {
        std::shared_ptr<void> guard = tie_;
        if(guard)
        {
            handle_all();
        }
    }
    else
    {
        handle_all();
    }
    
}
void Channel::handle_all()
{
    is_calling_ = true;

    if((revents_ & EPOLLHUP)&&!(revents_ & EPOLLIN))
    {
        if(callback_close_)
        {
            callback_close_();
        }
    }
    if(revents_ & EPOLLERR)   
    {
        LOG_ERROR<<"EPOLLERR on:"<<fd_;
        if(callback_error_)
        {
            callback_error_();
        }
    }
    if(revents_ & (EPOLLIN|EPOLLPRI))
    {
        if(callback_read_)
        {
            callback_read_();
        }
    }
    if(revents_ & EPOLLOUT)
    {
        if(callback_write_)
        {
            callback_write_();
        }
    }

    is_calling_ = false;
}

bool Channel::is_in_epoll()
{
    return is_in_epoll_;
}

void Channel::tie(const std::shared_ptr<void>& ti)
{
    tie_=ti;
}

Loop* Channel::get_loop()
{
    return loop_;
}

bool Channel::is_enable_read()
{
    return events_&EPOLLIN;
}

bool Channel::is_enable_write()
{
    return events_&EPOLLOUT;
}

void Channel::enable_read()
{
    events_ |= EPOLLIN;
    update_epoll();
}

void Channel::enable_write()
{
    events_ |= EPOLLOUT;
    update_epoll();
}

void Channel::disable_read()
{
    events_ &= ~EPOLLIN;
    update_epoll();
}

void Channel::disable_write()
{
    events_ &= ~EPOLLOUT;
    update_epoll();
}

void Channel::disable_all()
{
    events_ = 0;
    update_epoll();
}

void Channel::update_epoll()
{
    loop_->update_channel(this);
}
