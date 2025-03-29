#pragma once

#include "Acceptor.h"
#include "Tcpconnection.h"
#include <map>
class Tcpserver
{
public:
    typedef std::function<void(int,sockaddr_in)> callback_connect;
    typedef std::function<void(int,sockaddr_in)> callback_message;

    Tcpserver(/* args */);
    ~Tcpserver();
    
    void set_callback_connect(callback_connect callback);
    void set_callback_message(callback_message callback);

private:
    
    Acceptor acceptor_;
    std::shared_ptr<Loop> loop_;

    std::map<int,std::shared_ptr<Tcpconnection>> connections_;
    int next_connection_id_;
    callback_connect callback_connect_;
    callback_message callback_message_;
};

