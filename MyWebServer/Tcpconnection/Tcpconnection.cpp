#include "Tcpconnection.h"


Tcpconnection::Tcpconnection(int connect_id, std::shared_ptr<Loop> Loop, int socket_fd)
:connect_id_(connect_id),
Loop_(Loop),
channel_(std::make_shared<Channel>(Loop,socket_fd))
{

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
