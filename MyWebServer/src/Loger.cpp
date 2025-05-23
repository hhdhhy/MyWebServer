#include "Loger.h"
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <iostream>
std::atomic<Logstream::LogLevel> Loger::LOG_LEVEL;
FILE* Loger::get_fp(time_t &time_day,std::size_t &file_num,std::size_t &file_size, std::string&file_name)
{

    time(&time_day);
    tm day;
    localtime_r(&time_day,&day);

    file_name="webserver-";
    file_name+=std::to_string(day.tm_year+1900);
    file_name+="_";
    file_name+=std::to_string(day.tm_mon+1);
    file_name+="_";
    file_name+=std::to_string(day.tm_mday);
    file_name+="-";
    file_name+=std::to_string(file_num);
    file_name+=".log";
    FILE* fp=fopen(file_name.c_str(),"a+");

    struct stat stat_buf;
    stat(file_name.c_str(),&stat_buf);
    file_size=stat_buf.st_size;
    return fp;
}
struct close_file
{
    void operator()(FILE*fp)
    {
        if(fp)
        {
            fflush_unlocked(fp);
            if(fclose(fp)==EOF)
            {
                LOG_ERROR<<"Loger::get_fp()";
            }
        }
    }
};
Logstream::LogLevel Loger::get_log_level()
{
    return LOG_LEVEL.load();
}
void Loger::set_log_level(Logstream::LogLevel level)
{
    LOG_LEVEL.store(level);
}
Loger::Loger()
:file_num_(1),
file_size_(0),
stop_(0),
fp_(nullptr,close_file()),
cur_buffer_(new Logbuffer),
nex_buffer_(new Logbuffer),
buffer_n1_(new Logbuffer),
buffer_n2_(new Logbuffer)
{
    set_log_level(Logstream::INFO);
    fp_.reset(get_fp(time_day_,file_num_,file_size_,file_name_));
    if(!fp_)
    {
        LOG_FATAL<<"Loger::get_fp()";
    }
    buffers_write_.reserve(12);
    setbuffer(fp_.get(),out_buffer_,OUT_BUFFER_SIZE);//设置写文件缓冲区
}

Loger::~Loger()
{
    if(thread_)
    {
        stop_=1;
        thread_->join();
    }
}
void Loger::push(char *data,std::size_t len,Logstream::LogLevel level)//可跨线程
{
    std::lock_guard<std::mutex> lock(mutex_);
    if(level==Logstream::FATAL)
    {
        fflush(fp_.get());
        fwrite(data,1,len,stdout);
        abort();
    }
    if(cur_buffer_->avail()>=len)
    {
        cur_buffer_->push(data,len);
    }
    else
    {
        buffers_.push_back(std::move(cur_buffer_));
        if(nex_buffer_)
        {
            swap(nex_buffer_,cur_buffer_); 
        }
        else
        {
            cur_buffer_.reset(new Logbuffer);
        }
        cur_buffer_->push(data,len);
        cond_.notify_all();
    }
}
Loger& Loger::get_instance()
{
    static Loger loger;
    return loger;
}
void Loger::loop()
{
    int err;
    bool timeout=0;
    
    while(!stop_)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if(buffers_.empty())
            {
                cond_.wait_for(lock,std::chrono::seconds(2));//等两秒
            }
            buffers_.push_back(std::move(cur_buffer_));
     
            cur_buffer_=std::move(buffer_n1_);
            if(!nex_buffer_)
            {
                nex_buffer_=std::move(buffer_n2_);
            }
            swap(buffers_write_,buffers_);
        }
        write_to_file();
    }
    fflush_unlocked(fp_.get());
}

void Loger::write_to_file()
{
    time_t now_day;
    time(&now_day);

    if(now_day/DAY_SECONDS != time_day_/DAY_SECONDS)
    {
        file_num_=1;
        file_size_=0;
        time_day_=now_day;
        fp_.reset(get_fp(time_day_,file_num_,file_size_,file_name_));
        if(!fp_)
        {
            LOG_FATAL<<"Loger::get_fp()";
        }
    }
    if(file_size_ >= MAX_FILE_SIZE)
    {
        file_num_++;
        file_size_=0;
        fp_.reset(get_fp(now_day,file_num_,file_size_,file_name_));
        if(!fp_)
        {
            LOG_FATAL<<"Loger::get_fp()";
        }
    }
    for(auto &buffer:buffers_write_)
    {
        if(buffer->size())
        {
            fwrite_unlocked(buffer->data(),1,buffer->size(),fp_.get());//不用考虑线程安全

            file_size_+=buffer->size();
        }
    }
    
    if(buffers_write_.size()>2)
    {
        buffers_write_.resize(2);
    }
    if(!buffer_n1_)
    {
        std::swap(buffer_n1_,buffers_write_.back());
        buffers_write_.pop_back();
        buffer_n1_->clear();
    }
    if(!buffer_n2_)
    {
        std::swap(buffer_n2_,buffers_write_.back());
        buffers_write_.pop_back();
        buffer_n2_->clear();
    }
    fflush_unlocked(fp_.get());
}

void Loger::run_thread()
{
    thread_.reset(new std::thread([this](){loop();}));
}

