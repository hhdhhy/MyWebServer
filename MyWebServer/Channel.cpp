#include"Channel.h"

Channel::Channel(int fd,int event)
: m_fd(fd),m_event(event)
{
    
}

Channel::~Channel()
{
}

void Channel::set_callback_read(callback_function callback)
{
}

void Channel::set_callback_write(callback_function callback)
{
}

void Channel::set_callback_close(callback_function callback)
{
}

void Channel::set_callback_error(callback_function callback)
{
}
