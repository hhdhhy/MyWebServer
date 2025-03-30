#include "Tcpconnection.h"
#include <unistd.h>


Tcpconnection::Tcpconnection(int connect_id, Loop* loop, int socket_fd)
:connect_id_(connect_id),
loop_(loop),
channel_(loop,socket_fd)
{
    channel_.set_callback_close([this](){handle_close();});
    channel_.set_callback_error([this](){handle_error();});
    channel_.set_callback_read([this](){handle_read();});
    channel_.set_callback_write([this](){handle_write();});

    
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
    int len=in_buffer_.fd_read(channel_.get_fd(),err);
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

void Tcpconnection::send(std::string str)
{
    if(loop_->is_loop_in_thread())
    {
        handle_send(std::move(str));
    }
    else
    {
        loop_->add_run_callback([this,str=std::move(str)]() mutable {handle_send(std::move(str));});
    }
}

void Tcpconnection::handle_send(std::string str)//TL下可以少发一次减少延迟
{
    if(!loop_->is_loop_in_thread())
    {
        throw  std::runtime_error("Tcpconnection::handle_read() must be called in loop thread");
    }

    int len=0;
    if(!channel_.is_enable_write()||out_buffer_.read_capacity()==0)//缓存为空，直接发送
    len=write(channel_.get_fd(),str.data(),str.size());
    if(len < 0)
    {
        throw  std::runtime_error("handle_send() write error");
    }
    else if(len < str.size())
    {
        out_buffer_.cstr_read(str.data()+len,str.size()-len);
        if(!channel_.is_enable_write())
        channel_.enable_write();
    }
    else
    {
        if(callback_complete_write_)
        loop_->add_run_callback([this](){callback_complete_write_(shared_from_this());});
    }
    
}

void Tcpconnection::handle_write()
{
    if(!loop_->is_loop_in_thread())
    {
        throw  std::runtime_error("Tcpconnection::handle_read() must be called in loop thread");
    }
    int err=0;
    int len=out_buffer_.fd_read(channel_.get_fd(),err);
    if(len<0)
    {
        throw  std::runtime_error("handle_write() write error");
    }
    else if(out_buffer_.read_capacity()==0)
    {
        channel_.disable_write();
        if(callback_complete_write_)
        loop_->add_run_callback([this](){callback_complete_write_(shared_from_this());});
    }
}

void Tcpconnection::handle_close()
{
    channel_.disable_all();
    callback_close_(shared_from_this());//实际调用handle_destroy（保证自己在主线程中被删除（不跨线程））

}

void Tcpconnection::handle_error()
{
    throw  std::runtime_error("Tcpconnection::handle_error() must be called in loop thread");
}

void Tcpconnection::handle_destroy()
{
    if(!loop_->is_loop_in_thread())
    {
        throw  std::runtime_error("Tcpconnection::handle_read() must be called in loop thread");
    }
    channel_.disable_all();
    callback_connect_(shared_from_this());
    loop_->remove_channel(&channel_);

}

void Tcpconnection::handle_close_write()
{
    shutdown(channel_.get_fd(),SHUT_WR);
}

void Tcpconnection::write_close()
{
    loop_->add_run_callback([this](){handle_close_write();});
}

int Tcpconnection::get_connect_id()
{
    return connect_id_;
}

Loop *Tcpconnection::get_loop()
{
    return loop_;
}

