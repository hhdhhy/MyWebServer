#pragma once
#include "Epoll.h"
#include<atomic>
#include <thread>
#include <memory>
#include "Timequeue.h"
#include <mutex>
class Loop
{
public:
    Loop(/* args */);
    ~Loop();
    typedef std::function<void()> callback_function; 
    void run();// 运行loop

    void add_channel(Channel* channel);
    void remove_channel(Channel* channel);
    void update_channel(Channel* channel);

    bool is_loop_in_thread();
    bool is_channel_in_loop(Channel *channel);

    void add_timer(Timer::Timeus timeout, Timer::callback_function callback, Timer::Timeus interval);
    
    void add_run_callback(callback_function &cbf);
    void run_wait_callbacks();

    static int get_wakeup_fd();
    void handle_wakeup();
    int wakeup();

private:
    static const int INIT_CHANNELS_NUM = 100;   
    static const int MAX_EPOLL_TIME = 1000;

    std::atomic<bool> running_;// 是否正在运行
    std::atomic<bool> stop_;// 是否要停止


    const std::thread::id thread_id_;

    std::unique_ptr<Epoll> epoll_;
    std::unique_ptr<Timequeue> timequeue_;

    std::vector<Channel*> active_channels_; // 活跃的channel
    std::vector<Channel*> channels_;// 拥有的channel
    
    std::atomic<bool> has_wait_callbacks_;// 是否有等待执行的回调函数
    std::vector<callback_function> wait_callbacks_;// 等待执行的回调函数
    std::mutex wait_callbacks_mutex_;// 等待执行的回调函数栈的锁

    Channel wakeup_channel_;
};

