#pragma once
#include "Epoll.h"
#include<atomic>
#include <thread>
#include <memory>
class Loop
{
public:
    Loop(/* args */);
    ~Loop();

    void run();// 运行loop

    void add_channel(Channel* channel);
    void remove_channel(Channel* channel);
    void update_channel(Channel* channel);

    bool is_loop_in_thread();

    bool is_channel_in_loop(Channel *channel);

    bool is_in_thread();
private:
    static const int INIT_CHANNELS_NUM = 100;   
    static const int MAX_EPOLL_TIME = 1000;

    std::atomic<bool> running_;// 是否正在运行
    std::atomic<bool> stop_;// 是否要停止

    const std::thread::id thread_id_;

    std::unique_ptr<Epoll> epoll_;
    std::vector<Channel*> active_channels; // 活跃的channel
    std::vector<Channel*> channels;// 拥有的channel
    
};

