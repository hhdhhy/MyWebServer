#include "Timequeue.h"
#include <functional>
#include <sys/timerfd.h>
#include <cstring>
#include <unistd.h>
#include "Loger.h"
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
    add_timer(std::move(Timer(timeout,callback,interval)));
}
void Timequeue::add_timer(Timer timer)
{
    loop_->add_run_callback([this,timer=std::move(timer)]() {handle_add(timer);});
}
void Timequeue::handle_add(Timer timer)
{
    if(!loop_->is_loop_in_thread())
    {
        throw std::runtime_error("loop is not in thread");
    }
    add(timer);

}

void Timequeue::handle_timer()//读回调
{
    uint64_t read_byte;
    ssize_t readn = read(timer_channel_.get_fd(), &read_byte, sizeof(read_byte));
    if(readn != sizeof(read_byte))
    {
        LOG_ERROR << "read timerfd error";
    }

    Timer::Timeus now = Timer::get_now();

    std::vector<value_type> timeout_timer(std::move(get_timeout_timer()));
    for(auto &timer:timeout_timer)
    {
        timer.second.run();
    }
    reset(timeout_timer,now);
}

void Timequeue::reset(std::vector<Timequeue::value_type> &timeout_timer,Timer::Timeus now)
{
    
    for(auto &timer:timeout_timer)
    {
        if(timer.second.is_reapeat())
        {
            timer.second.reset_timeout(now);
            add(std::move(timer.second));
        }
    }
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
    if(dif_time_ms<0)
    {
        dif_time_ms = 1000;
    }
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

void Timequeue::add(Timer timer)
{
    Timer::Timeus timeout=timer.get_timeout();
    std::pair<Timer::Timeus,Timer> timer_pair({timeout,std::move(timer)});
    timer_set_.insert(timer_pair);
    if((timer_set_.begin())->first==timeout)
    reset_timeout(timeout);
}
