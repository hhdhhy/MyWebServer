#include "Timequeue.h"
#include <functional>
#include <sys/timerfd.h>
#include <cstring>
Timequeue::Timequeue(Loop* loop)
:loop_(loop),timer_channel_(loop,get_timerfd())
{
    timer_channel_.enable_read();
    timer_channel_.set_callback_read([this](){handle_timer();});
}

Timequeue::~Timequeue()
{

}

void Timequeue::add_timer(Timer::Timeus timeout, Timer::callback_function callback, Timer::Timeus interval = 0)
{
    add_timer(std::make_shared<Timer>(timeout,callback,interval));
}
void Timequeue::add_timer(std::shared_ptr<Timer> timer)
{
    loop_->add_run_callback(std::bind(&Timequeue::handle_add,this,timer));
}
void Timequeue::handle_add(std::shared_ptr<Timer> timer)
{
    if(!loop_->is_loop_in_thread())
    {
        throw std::runtime_error("loop is not in thread");
    }
    add(timer);
}

void Timequeue::handle_timer()//读回调
{
    
}

int Timequeue::get_timerfd()
{
    int timerfd = timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0)
    {
        throw std::runtime_error("timerfd_create error");
    }
    return timerfd;
}

void Timequeue::reset_timeout(Timer::Timeus new_timeout)
{
    itimerspec new_spec;
    memset(&new_spec,0,sizeof(new_spec));
    //itimerspec.it_interval：指定定时器的周期。如果定时器是单次触发，这个值应为 0。
    //itimerspec.it_value：指定定时器的初始启动时间。

    Timer::Timeus dif_time_ms= new_timeout - timeout_;
    new_spec.it_value.tv_sec = dif_time_ms/1000000;
    new_spec.it_value.tv_nsec = (dif_time_ms%1000000)*1000;

    if (timerfd_settime(timer_channel_.get_fd(), 0, &new_spec, NULL))
    {
        throw std::runtime_error("timerfd_settime error");
    }
    timeout_ = new_timeout;
}

std::vector<Timequeue::value_type> Timequeue::get_timeout_timer()
{
    std::vector<value_type> timeout_timer;

    for(auto it = timer_set_.begin();it!=timer_set_.end();)
    {
        if(it->first <= Timer::get_now())
        {
            timeout_timer.push_back(*it);
            it = timer_set_.erase(it);
        }
        else
        {
            break;
        }
    }
    return timeout_timer;
}

void Timequeue::add(std::shared_ptr<Timer> timer)
{
    std::pair<Timer::Timeus,std::shared_ptr<Timer>> timer_pair({timer->get_timeout(),timer});
    timer_set_.insert(timer_pair);

    if(*timer_set_.begin()==timer_pair)
    reset_timeout(timer->get_timeout());
}
