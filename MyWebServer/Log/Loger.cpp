#include "Loger.h"
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
Loger::Loger()
:file_num_(1),file_size_(0),stop_(0)
{
    time_t time(NULL);
    localtime_r(&time,&day_);
    open_file(day_);
}

bool Loger::open_file(tm &day)
{
    std::string file_name="webserver-";
    
    file_name+=std::to_string(day.tm_year+1900);
    file_name+="/";
    file_name+=std::to_string(day.tm_mon+1);
    file_name+="/";
    file_name+=std::to_string(day.tm_mday);
    file_name+="-";
    file_name+=std::to_string(file_num_);
    file_name+=".log";
    fd_=open(file_name.c_str(),O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR);
    if(fd_<0)
    return false;

    return true;
}

bool Loger::close_file()
{
    int ret=close(fd_);
    if(ret<0)
    return false;
    return true;
}

Loger::~Loger()
{
    if(thread_)
    {
        stop_=1;
        thread_->join();
    }
    if(close_file()==NULL)
    {
        // 日志
    }
}

void Loger::push(Logstream &logstream)
{
    deque_.push_back(std::move(logstream.data_));
}

void Loger::run()
{
    int err;
    bool timeout=0;
    while(!stop_)
    {
        Logstream::log_data log;
        if(!deque_.pop_front(log,300))
        timeout=1;
        else
        buffer_.cstr_read(log.buffer_.get(),log.len_);

        if(stop_||timeout||buffer_.write_capacity() < Logstream::BUFFER_SIZE)
        {
            tm now_day;
            time_t now_time(NULL);
            localtime_r(&now_time,&now_day);

            if(now_day.tm_mday != day_.tm_mday||now_day.tm_mon != day_.tm_mon||now_day.tm_year != day_.tm_year)
            {
                int ret=open_file(now_day);
                if(ret<0)
                {
                    throw std::runtime_error("Loger::run()");
                }
            }
            int len=buffer_.fd_write(fd_,err);
            if(len<0)
            {
                throw std::runtime_error("Loger::run()");
            }

            buffer_.clear();
            file_size_+=len;
            
            if(file_size_ >= MAX_FILE_SIZE)
            {
                file_size_=0;  
                file_num_++;
                int ret=open_file(now_day);
                if(ret<0)
                {
                    throw std::runtime_error("Loger::run()");
                }
            }
        }
    }


}

void Loger::run_thread()
{
    thread_.reset(new std::thread(&Loger::run,this));
    thread_->detach();
}

