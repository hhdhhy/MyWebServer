#include "Timer.h"
#include <sys/time.h>

Timer::Timeus Timer::get_now()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return static_cast<long long>(tv.tv_sec)*1000000ll+static_cast<long long>(tv.tv_usec);
}


Timer::Timer(Timeus timeout, callback_function callback, Timeus interval = 0)
    : timeout_(timeout),
      callback_(callback),
      interval_(interval)
{
  is_repeat_=(interval_!=0);
}

void Timer::reset_timeout()
{
    if(is_repeat_)
    {
        timeout_=get_now()+interval_;
    }
}

void Timer::reset_timeout(Timeus timeout)
{
    if(is_repeat_)
    {
        timeout_=timeout+interval_;
    }
}
