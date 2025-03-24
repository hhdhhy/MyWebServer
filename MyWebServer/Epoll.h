#pragma once
#include <sys/epoll.h>
#include <vector>
#include "Channel.h"
class Epoll
{
private:

    /* data */
    const int init_revent_size = 32;

    int epollfd_;
    
    std::vector<epoll_event> revents_;//存放epoll_wait返回的就绪事件
    
    std::unordered_map<int, Channel*> channels_;//存放所有fd对应的channel

public:
    Epoll(/* args */);
    ~Epoll();
    
    void delete_channel(Channel* channel);
    void update_channel(Channel* channel);
    void add_channel(Channel* channel);

    void poll(std::vector<Channel*>& active_channels);
    
};


