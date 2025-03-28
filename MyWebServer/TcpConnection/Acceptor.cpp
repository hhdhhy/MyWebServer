#include "Acceptor.h"
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>

Acceptor::Acceptor(std::shared_ptr<Loop> loop, sockaddr_in addr)
: loop_(loop),
channel_(std::make_shared<Channel>(loop,get_socket_fd()))

{
}

Acceptor::~Acceptor()
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
