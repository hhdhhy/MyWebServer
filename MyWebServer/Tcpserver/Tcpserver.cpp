#include "Tcpserver.h"
Tcpserver::Tcpserver(sockaddr_in addr)
:loop_(),acceptor_(loop_,addr)
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
    std::shared_ptr<Tcpconnection> connection = std::make_shared<Tcpconnection>(loop_,next_connection_id_,fd);
    connections_[next_connection_id_]= connection;
    next_connection_id_++;
    connection->set_callback_connect(callback_connect_);
    connection->set_callback_message(callback_message_);
}
