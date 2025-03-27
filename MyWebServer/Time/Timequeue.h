#pragma once
#include "Timer.h"
#include "Loop.h"
#include <set>
#include <utility>
#include <memory>
class Timequeue
{
public:
    Timequeue(Loop *loop);
    ~Timequeue();
    void add_timer(std::shared_ptr<Timer> timer);
    void handle_timer();
    static int get_timerfd();

private:
    
    Timer::Timeus timeout_;
    Channel timer_channel_;
    std::set<std::pair<Timer::Timeus, Timer*>> timer_set_;
    Loop *loop_;
    
};


