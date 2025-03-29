#include "Tcpserver.h"
Tcpserver::Tcpserver(sockaddr_in addr)
:loop_(std::make_shared<Loop>()),acceptor_(loop_,addr)
{
}

Tcpserver::~Tcpserver()
{
}

void Tcpserver::set_callback_connect(const callback_connect& callback)
{
    callback_connect_ = callback;
}

void Tcpserver::set_callback_message(const callback_message& callback)
{
    callback_message_ = callback;
}

void Tcpserver::handle_connect(int fd,sockaddr_in addr)
{

}
