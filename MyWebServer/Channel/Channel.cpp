#include"Channel.h"
#include<unistd.h>
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
        throw std::runtime_error("Channel::handle_all() is calling");
    }
    if(fd_>=0)
    close(fd_);
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

void Channel::handle_all()
{
    is_calling_ = true;
    if(revents_ & EPOLLERR)   
    {
        if(callback_error_)
        {
            callback_error_();
        }
    }
    if(revents_ & (EPOLLIN||EPOLLHUP||EPOLLPRI))
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
    if(revents_ & EPOLLOUT)
    {
        if(callback_close_)
        {
            callback_close_();
        }
    }
    is_calling_ = false;
}

bool Channel::is_in_epoll()
{
    return is_in_epoll_;
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
