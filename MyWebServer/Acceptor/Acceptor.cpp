#include "Acceptor.h"
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include "Loger.h"
Acceptor::Acceptor(Loop* loop, sockaddr_in addr)
: loop_(loop),
channel_(loop,get_socket_fd())
{

    // SO_REUSEADDR 允许一个套接字强制绑定到一个已被其他套接字使用的端口。
    // SO_REUSEPORT 允许同一主机上的多个套接字绑定到相同的端口号。
    // SO_KEEPALIVE 启用在已连接的套接字上定期传输消息,如果另一端没有响应，则认为连接已断开并关闭
    int opt=1;
    setsockopt(channel_.get_fd(),SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    setsockopt(channel_.get_fd(),SOL_SOCKET,SO_REUSEPORT,&opt,sizeof(opt));

    if(0>::bind(channel_.get_fd(),(sockaddr*)&addr,sizeof(addr)))
    {
        LOG_FATAL<<"bind error";
    }
    channel_.set_callback_read([this](){handle_accept();});
}

Acceptor::~Acceptor()
{
    channel_.disable_all();
}

void Acceptor::handle_accept()
{
    sockaddr_in addr;
    socklen_t len=sizeof(addr);
    int connfd=accept4(channel_.get_fd(),reinterpret_cast<sockaddr*>(&addr),&len,SOCK_NONBLOCK|SOCK_CLOEXEC);
    if(connfd<0)
    {
        LOG_ERROR<<"accept error";
        if (errno == EMFILE)
        {
            LOG_ERROR<<"sockfd reached limit";
        }
    }
    else
    {
        if(connect_callback_)
        {
            connect_callback_(connfd,addr);
        }
        else
        {
            LOG_FATAL<<"connect_callback_ is null";
        }
    }

}

int Acceptor::get_socket_fd()
{
    int fd=socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC,IPPROTO_TCP);

    if(fd<0)
    {
        LOG_ERROR<<"getsocket error";
    }
    return fd;
}

void Acceptor::listen()
{
    if(0>::listen(channel_.get_fd(),SOMAXCONN))
    {
        LOG_ERROR<<"listen error";
    }
    LOG_INFO<<"listening fd:"<<channel_.get_fd();
    channel_.enable_read();//注册到epoll上
}

void Acceptor::set_connect_callback(std::function<void(int,sockaddr_in)> callback)
{
    connect_callback_ = callback;
}