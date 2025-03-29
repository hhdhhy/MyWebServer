#include "Acceptor.h"
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>

Acceptor::Acceptor(Loop* loop, sockaddr_in addr)
: loop_(loop),
channel_(std::make_shared<Channel>(loop,get_socket_fd()))
{

    // SO_REUSEADDR 允许一个套接字强制绑定到一个已被其他套接字使用的端口。
    // SO_REUSEPORT 允许同一主机上的多个套接字绑定到相同的端口号。
    // SO_KEEPALIVE 启用在已连接的套接字上定期传输消息,如果另一端没有响应，则认为连接已断开并关闭
    int opt=1;
    setsockopt(channel_->get_fd(),SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    setsockopt(channel_->get_fd(),SOL_SOCKET,SO_REUSEPORT,&opt,sizeof(opt));

    if(0>::bind(channel_->get_fd(),(sockaddr*)&addr,sizeof(addr)))
    {
        throw std::runtime_error("bind error");
    }
    channel_->set_callback_read([this](){handle_accept();});
}

Acceptor::~Acceptor()
{
}

void Acceptor::handle_accept()
{

}

int Acceptor::get_socket_fd()
{
    int fd=socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC,IPPROTO_TCP);

    if(fd<0)
    {
        throw std::runtime_error("socket error");
    }
    return fd;
}

void Acceptor::listen()
{
    if(0>::listen(channel_->get_fd(),SOMAXCONN))
    {
        throw std::runtime_error("listen error");
    }
    channel_->enable_read();//注册到epoll上
}

void Acceptor::set_connect_callback(std::function<void(int,sockaddr_in)> callback)
{
    connect_callback_ = callback;
}