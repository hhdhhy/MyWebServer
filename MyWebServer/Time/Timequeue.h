#pragma once
#include "Timer.h"
#include "Loop.h"
#include <set>
#include <utility>
#include <memory>
class Timequeue
{
public:
    typedef std::pair<Timer::Timeus, std::shared_ptr<Timer>> value_type;
    Timequeue(Loop *loop);
    ~Timequeue();
    void add_timer(Timer::Timeus timeout, Timer::callback_function callback, Timer::Timeus interval = 0);
    void add_timer(std::shared_ptr<Timer> timer);
    void handle_timer();
    static int get_timerfd();
    void reset_timeout(Timer::Timeus);
    std::vector<value_type> get_timeout_timer();

private:
    
    Timer::Timeus timeout_;
    Channel timer_channel_;
    std::set<value_type> timer_set_;
    Loop *loop_;
    
};


