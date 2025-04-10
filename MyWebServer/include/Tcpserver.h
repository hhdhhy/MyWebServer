#pragma once

#include "Acceptor.h"
#include "Tcpconnection.h"
#include "Loopthreadpool.h"
#include <map>
class Tcpserver
{
public:
    typedef Tcpconnection::callback_connect callback_connect;
    typedef Tcpconnection::callback_message callback_message;
    typedef Tcpconnection::callback_highwater_write callback_highwater_write;
    typedef Tcpconnection::callback_complete_write callback_complete_write;

    Tcpserver(Loop* loop,sockaddr_in addr);
    ~Tcpserver();
    
    void set_callback_connect(const callback_connect &callback);
    void set_callback_message(const callback_message &callback);
    void set_callback_close(const callback_connect &callback);
    void set_callback_highwater_write(const callback_highwater_write &callback);
    void set_callback_complete_write(const callback_complete_write &callback);

    void handle_connect_iothread(const std::shared_ptr<Tcpconnection> &conn);
    void handle_connect(int fd,sockaddr_in addr);//创建Tcpconnection
    void handle_close(const std::shared_ptr<Tcpconnection> &conn);
    void handle_close_mainthread(const std::shared_ptr<Tcpconnection> &conn);
    void start();

private:
    std::atomic_int started_;
    Acceptor acceptor_;
    Loop* loop_;

    std::map<int,std::shared_ptr<Tcpconnection>> connections_;
    int next_connection_id_;

    callback_connect callback_connect_;
    callback_message callback_message_;
    callback_connect callback_close_;
    callback_highwater_write callback_highwater_write_;
    callback_complete_write callback_complete_write_;

    Loopthreadpool* threadpool_; 
};

