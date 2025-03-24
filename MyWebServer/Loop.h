#pragma once
#include "Epoll.h"

class Loop
{
private:
    /* data */
    Epoll epoll_;
    bool quit_;// 是否退出
    bool running_;// 是否正在运行
    
    std::vector<Channel*> active_channels; // 活跃的channel
    std::vector<Channel*> channels;// 拥有的channel

    
public:
    Loop(/* args */);
    ~Loop();

    void run();// 运行loop

    void add_channel(Channel* channel);
    void remove_channel(Channel* channel);
    void update_channel(Channel* channel);

};

