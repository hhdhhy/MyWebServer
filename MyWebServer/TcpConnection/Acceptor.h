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
    
    void accept();

    static int get_socket_fd();
     
private:
    /* data */
    
    void listen();
    void bind();

    int socket_fd;
    std::shared_ptr<Channel> channel_;
    std::shared_ptr<Loop> loop_;

};

