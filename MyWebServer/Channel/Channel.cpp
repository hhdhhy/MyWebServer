#include"Channel.h"

Channel::Channel(std::shared_ptr<Loop> loop, int fd, int event=NONE_EVENT)
: loop_(loop),fd_(fd),events_(event),is_in_epoll_(false)
{

}

Channel::~Channel()
{

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

void Channel::callback_all()
{
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
}

std::shared_ptr<Loop> Channel::get_loop()
{
    return loop_;
}
