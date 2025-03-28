#pragma once
#include <sys/epoll.h>
#include <vector>
#include "Channel.h"
class Epoll
{

public:
    Epoll(/* args */);
    ~Epoll();

    void poll(int timeout_ms, std::vector<std::shared_ptr<Channel>> &active_channels);

    void delete_channel(std::shared_ptr<Channel> channel);
    void update_channel(std::shared_ptr<Channel> channel);
    void add_channel(std::shared_ptr<Channel> channel);


    void remove_all();//释放channel

private:

    /* data */
    static const  int init_revent_size = 32;

    int epoll_fd_;
    
    std::vector<epoll_event> revents_;//存放epoll_wait返回的就绪事件
    
    
    std::unordered_map<int,std::shared_ptr<Channel>> channels_;//存放所有fd对应的channel    
};


