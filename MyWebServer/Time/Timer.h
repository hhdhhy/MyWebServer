#pragma once
#include <functional>
#include <sys/time.h>
//封装回调函数及其超时时间，时间间隔（重复定时器）

class Timer {
    
public:
    typedef long long Timeus;//微秒
    typedef std::function<void()> callback_function;
    static Timeus get_now();//获取当前时间（微秒）

    Timer(Timeus timeout, callback_function callback, Timeus interval = 0);
    void reset_timeout();
    void reset_timeout(Timeus timeout);
    Timeus get_timeout();
    void run();
    bool is_reapeat();
    Timer::Timeus get_interval();
private:

    
    callback_function callback_;
    Timeus timeout_;//超时时间
    Timeus interval_;//超时间隔
    bool is_repeat_;//是否重复

};
