#pragma once
#include <functional>
#include <sys/time.h>
//封装回调函数及其超时时间，时间间隔（重复定时器）

class Timer {
    typedef std::function<void()> callback_function;
public:
    typedef long long Timeus;

    static Timeus get_now()
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        return (long long)tv.tv_sec*1000000ll+(long long)tv.tv_usec;
    }

    Timer(Timeus timeout, callback_function callback, Timeus interval = 0);
    void reset_timeout();
    void reset_timeout(Timeus timeout);

private:

    callback_function callback_;

    Timeus timeout_;//超时时间
    Timeus interval_;//超时间隔
    bool is_repeat_;//是否重复

};
