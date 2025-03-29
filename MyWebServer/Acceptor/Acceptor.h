#pragma once
#include "Channel.h"
#include "Loop.h"
#include <memory>
#include <netinet/in.h>
class Acceptor
{

public:
    Acceptor(std::shared_ptr<Loop> loop,sockaddr_in addr);
    ~Acceptor();
    
    void handle_accept();

    static int get_socket_fd();
    
    void set_connect_callback(std::function<void(int,sockaddr_in)> callback);
private:
    /* data */
    
    void listen();

    int socket_fd;
    std::shared_ptr<Channel> channel_;
    std::shared_ptr<Loop> loop_;

    std::function<void(int,sockaddr_in)> connect_callback_;
};

