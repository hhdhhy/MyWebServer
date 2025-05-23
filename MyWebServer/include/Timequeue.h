#pragma once
#include "Timer.h"
#include <set>
#include <utility>
#include <memory>
#include "Channel.h"
#include <vector>
class Loop;
class Timequeue
{
public:
    typedef std::pair<Timer::Timeus,std::shared_ptr<Timer>> value_type;
    Timequeue(Loop* loop);
    ~Timequeue();

    void add_timer(Timer::Timeus timeout, Timer::callback_function callback, Timer::Timeus interval = 0);
    void add_timer(std::shared_ptr<Timer> timer);// 添加一个定时器（添加过程在loop_（回调函数））
    void handle_add(std::shared_ptr<Timer> timer);

    void handle_timer();

    void reset(std::vector<Timequeue::value_type> &timeout_timer, Timer::Timeus now);
    static int get_timerfd();
    void reset_timeout(Timer::Timeus);
    std::vector<value_type> get_timeout_timer();

private:

    void add(std::shared_ptr<Timer> timer);

    Timer::Timeus timeout_;
    Channel timer_channel_;
    std::set<value_type> timer_set_;
    Loop* loop_;
    
};


