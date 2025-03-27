#include "Timequeue.h"
#include <functional>
Timequeue::Timequeue(Loop* loop)
:loop_(loop),timer_channel_(loop,get_timerfd())
{
    timer_channel_.enable_read();
    timer_channel_.set_callback_read(std::bind(&Timequeue::handle_timer,this));
}

Timequeue::~Timequeue()
{
    
}

void Timequeue::add_timer(std::shared_ptr<Timer> timer)
{

}

void Timequeue::handle_timer()
{

}

int Timequeue::get_timerfd()
{

    return 0;
}
