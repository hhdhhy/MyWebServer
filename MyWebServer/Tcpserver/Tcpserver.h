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
    void set_callback_close(const callback_connect &callback);
    void handle_connect(int fd,sockaddr_in addr);//创建Tcpconnection
    void handle_close(const std::shared_ptr<Tcpconnection> &conn);
private:
    
    Acceptor acceptor_;
    Loop* loop_;

    std::map<int,std::shared_ptr<Tcpconnection>> connections_;
    int next_connection_id_;
    callback_connect callback_connect_;
    callback_message callback_message_;
  
};

