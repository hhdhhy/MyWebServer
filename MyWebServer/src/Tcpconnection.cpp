#include "Tcpconnection.h"
#include <unistd.h>
#include "Loger.h"

Tcpconnection::Tcpconnection(int connect_id, Loop* loop, int socket_fd,std::string addr)
:connect_id_(connect_id),
loop_(loop),
channel_(loop,socket_fd),
state_(CONNECTING),
addr_(addr)
{
    channel_.set_callback_close([this](){handle_close();});
    channel_.set_callback_error([this](){handle_error();});
    channel_.set_callback_read([this](){handle_read();});
    channel_.set_callback_write([this](){handle_write();});

    // SO_KEEPALIVE 启用在已连接的套接字上定期传输消息。
    // 如果另一端没有响应，则认为连接已断开并关闭,这对于检测网络中失效的对等方非常有用。
    int opt=1;
    LOG_INFO<<"create tcp connection fd:"<<channel_.get_fd();
    ::setsockopt(channel_.get_fd(), SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
}

Tcpconnection::~Tcpconnection()
{
    LOG_INFO<<"delete tcp connection fd:"<<channel_.get_fd();
}

void Tcpconnection::set_callback_connect(const callback_connect &callback)
{
    callback_connect_ = callback;
}

void Tcpconnection::set_callback_message(const callback_message &callback)
{
    callback_message_ = callback;
}

void Tcpconnection::set_callback_close(const callback_connect &callback)
{
    callback_close_ = callback;
}

void Tcpconnection::set_callback_highwater_write(const callback_highwater_write &callback)
{
    callback_highwater_write_ = callback;
}

void Tcpconnection::set_callback_complete_write(const callback_complete_write &callback)
{
    callback_complete_write_ = callback;
}

void Tcpconnection::handle_read()
{
    if(!loop_->is_loop_in_thread())
    {
        LOG_FATAL<<"Tcpconnection::handle_read() must be called in loop thread";
    }

    int err=0;
    int len=in_buffer_.fd_read(channel_.get_fd(),err);

    LOG_INFO<<"read from fd: "<<channel_.get_fd()<<" len: "<<len<<" err: "<<err;
    if(len > 0)
    {
        if(callback_message_)
        {
            callback_message_(shared_from_this(),in_buffer_);
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
    if(state_ == State::CONNECTED)
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
}
    

void Tcpconnection::handle_connect()
{
    LOG_DEBUG<<"connect fd:"<<channel_.get_fd();
    set_state(State::CONNECTED);
    channel_.tie(shared_from_this());
    channel_.enable_read();
    callback_connect_(shared_from_this());
}

void Tcpconnection::handle_send(std::string str)//TL下可以少发一次减少延迟
{
    if(!loop_->is_loop_in_thread())
    {
        LOG_FATAL<<"must call in loop thread";
    }
    if(state_==State::CLOSED)
    {
        LOG_ERROR<<"connection is closed";
    }
    int len=0;
    if(!channel_.is_enable_write()||out_buffer_.read_capacity()==0)//缓存为空，直接发送
    len=write(channel_.get_fd(),str.data(),str.size());
    if(len < 0)
    {
        LOG_ERROR<<"write error";
    }
    else if(len < str.size())
    {
        LOG_INFO<<"write partial";
        out_buffer_.cstr_read(str.data()+len,str.size()-len);
        if(!channel_.is_enable_write())
        channel_.enable_write();
    }
    else
    {
        LOG_INFO<<"write complete";
        if(callback_complete_write_)
        loop_->add_run_callback([this](){callback_complete_write_(shared_from_this());});
    }
    
}

void Tcpconnection::handle_write()
{
    if(!loop_->is_loop_in_thread())
    {
        LOG_FATAL<<"must call in loop thread";
    }
    int err=0;
    int len=out_buffer_.fd_read(channel_.get_fd(),err);
    if(len<0)
    {
        LOG_ERROR<<"read buffer error";
    }
    else if(out_buffer_.read_capacity()==0)
    {
        LOG_INFO<<"write complete";
        channel_.disable_write();
        if(callback_complete_write_)
        loop_->add_run_callback([this](){callback_complete_write_(shared_from_this());});

        if(state_==State::CLOSING)
        {
            handle_close_write();
        }
    }
}

void Tcpconnection::handle_close()
{
    LOG_DEBUG<<"close fd:"<<channel_.get_fd();
    set_state(State::CLOSED);
    channel_.disable_all();
    callback_connect_(shared_from_this());
    callback_close_(shared_from_this());//实际调用handle_destroy（保证自己在主线程中被删除（不跨线程））

}

void Tcpconnection::handle_error()
{
    LOG_DEBUG<<"error fd:"<<channel_.get_fd();

    int opt;
    socklen_t optlen = sizeof(opt);
    if(getsockopt(channel_.get_fd(),SOL_SOCKET,SO_ERROR,&opt,&optlen)<0)
    {
        LOG_ERROR<<"getsockopt error :"<<errno;
    }
    else
    {
        LOG_ERROR<<"socket error :"<<opt;
    }
}

void Tcpconnection::handle_destroy()
{
    if(!loop_->is_loop_in_thread())
    {
        LOG_FATAL<<"must be called in loop thread";
    }
    if(state_.load()==State::CONNECTED)
    {
        set_state(State::CLOSING);
        channel_.disable_all();
        callback_connect_(shared_from_this());
    }
    
    loop_->remove_channel(&channel_);

}

void Tcpconnection::handle_close_write()
{
    LOG_INFO<<"write close fd:"<<channel_.get_fd();
    shutdown(channel_.get_fd(),SHUT_WR);
}

void Tcpconnection::write_close()
{
    if(state_.load()==State::CONNECTED)
    {
        set_state(State::CLOSING);
        loop_->add_run_callback([this](){handle_close_write();});

    }
}

void Tcpconnection::set_state(State state)
{
    state_.store(state);
}

int Tcpconnection::get_connect_id()
{
    return connect_id_;
}

Loop *Tcpconnection::get_loop()
{
    return loop_;
}

