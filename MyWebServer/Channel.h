#pragma once

#include<sys/epoll.h>
#include<functional>

//封装fd与其关注事件与发生事件 和回调函数
class Channel
{
private:
    /* data */
    typedef std::function<void()> callback_function;

    int fd_;
    int events_;
    int revents_;

    callback_function callback_read;
    callback_function callback_write;
    callback_function callback_close;
    callback_function callback_error;


public:
    Channel(int fd,int event);
    ~Channel();

    void set_callback_read(callback_function callback);
    void set_callback_write(callback_function callback);
    void set_callback_close(callback_function callback);
    void set_callback_error(callback_function callback);

    void set_events(const int &event);
    void set_revents(const int &revent);

};

