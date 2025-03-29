#include "Tcpconnection.h"
#include <unistd.h>


Tcpconnection::Tcpconnection(int connect_id, Loop* loop, int socket_fd)
:connect_id_(connect_id),
loop_(loop),
channel_(loop,socket_fd)
{
    channel_.set_callback_close([this](){this->handle_close();});
    channel_.set_callback_error([this](){this->handle_error();});
    channel_.set_callback_read([this](){this->handle_read();});
    channel_.set_callback_write([this](){this->handle_write();});
}

Tcpconnection::~Tcpconnection()
{
    
}

void Tcpconnection::set_callback_connect(const callback_connect &callback)
{
    callback_connect_ = callback;
}

void Tcpconnection::set_callback_message(const callback_message &callback)
{
    callback_message_ = callback;
}

void Tcpconnection::handle_read()
{
    if(!loop_->is_loop_in_thread())
    {
        throw  std::runtime_error("Tcpconnection::handle_read() must be called in loop thread");
    }

    int err=0;
    int len=in_buffer_->fd_read(channel_.get_fd(),err);
    if(len > 0)
    {
        if(callback_message_)
        {
            callback_message_(shared_from_this(),in_buffer_,len);
        }
    }
    else if(len == 0)
    {
        handle_close();
    }
    else
    {
        handle_error();
    }
}

void Tcpconnection::handle_write()
{

}

void Tcpconnection::handle_close()
{
    channel_.disable_all();
    callback_close_(shared_from_this());

}

void Tcpconnection::handle_error()
{
    throw  std::runtime_error("Tcpconnection::handle_error() must be called in loop thread");
}

void Tcpconnection::connect_close()
{
    if(!loop_->is_loop_in_thread())
    {
        throw  std::runtime_error("Tcpconnection::handle_read() must be called in loop thread");
    }
    channel_.disable_all();

}
