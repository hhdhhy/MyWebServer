#pragma once
#include "Channel.h"
#include "Loop.h"
#include <memory>
#include <netinet/in.h>
class Acceptor
{

public:
    Acceptor(Loop* loop,sockaddr_in addr);
    ~Acceptor();
    
    void handle_accept();

    static int get_socket_fd();
    
    void set_connect_callback(std::function<void(int,sockaddr_in)> callback);
private:
    /* data */
    
    void listen();

    Channel channel_;
    Loop* loop_;

    std::function<void(int,sockaddr_in)> connect_callback_;
};

