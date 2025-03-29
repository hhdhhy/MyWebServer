#pragma once

#include "Acceptor.h"
#include "Tcpconnection.h"
#include <map>
class Tcpserver
{
public:
    typedef Tcpconnection::callback_connect callback_connect;
    typedef Tcpconnection::callback_message callback_message;


    Tcpserver(sockaddr_in addr);
    ~Tcpserver();
    
    void set_callback_connect(const callback_connect &callback);
    void set_callback_message(const callback_message &callback);

    void handle_connect(int fd,sockaddr_in addr);//创建Tcpconnection
private:
    
    Acceptor acceptor_;
    std::shared_ptr<Loop> loop_;

    std::map<int,std::shared_ptr<Tcpconnection>> connections_;
    int next_connection_id_;
    callback_connect callback_connect_;
    callback_message callback_message_;
};

