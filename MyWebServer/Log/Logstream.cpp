#include"Logstream.h"
#include<cstring>
#include"Loger.h"
#include"Timer.h"
#include<sstream>
#include<string>

static thread_local std::string THREAD_ID;
time_t Logstream::LAST_TIME=NULL;
char Logstream::LAST_TIME_STR[30];
const char* Logstream::level_str[LogLevel::LEVEL_COUNT] = {"TRACE","DEBUG","INFO","WARN","ERROR","FATAL"}; 

Logstream::Logstream(LogLevel level,const char *file, int line,const char *func)
:idx_(0),
level_(level)
{
    if(THREAD_ID.empty())
    {
        std::stringstream o;
        o<<std::this_thread::get_id();
        o>>THREAD_ID;
    }
    int len=sprintf(get_buffer_addr(),"%s__",THREAD_ID.c_str());
    buffer_pushed(len);
    push_time();
    len=sprintf(get_buffer_addr(),"__%s__%s__%d__%s__:",level_str[level],file,line,func);
    buffer_pushed(len);
}
Logstream::~Logstream()
{
    operator<<("\n");
    Loger::get_instance().push(data_,idx_,level_);
}

void Logstream::push_time()
{
    time_t now = time(NULL);
    if(LAST_TIME!=now)
    {
        LAST_TIME=now;
        tm time;
        localtime_r(&now,&time);
        strftime(LAST_TIME_STR,sizeof(LAST_TIME_STR),"%Y-%m-%d %H:%M:%S",&time);
    }
    Timer::Timeus now_timeus = Timer::get_now()%1000000;
    int len=sprintf(get_buffer_addr(),"%s %lldus",LAST_TIME_STR,now_timeus);
    buffer_pushed(len);
}

char* Logstream::get_buffer_addr()
{
    return data_+idx_;
}

void Logstream::buffer_pushed(std::size_t len)
{
    idx_+=len;
}

Logstream& Logstream::operator<<(const char data)
{
    int len=sprintf(get_buffer_addr(),"%c",data);
    buffer_pushed(len);
    return *this;
}
Logstream& Logstream::operator<<(const char *str)
{
    int len=sprintf(get_buffer_addr(),"%s",str);
    buffer_pushed(len);
    return *this;
}
Logstream& Logstream::operator<<(const cstr &str)
{
    int len=sprintf(get_buffer_addr(),"%.*s",str.len,str.str);
    buffer_pushed(len);
    return *this;
}
Logstream& Logstream::operator<<(const std::string& str)
{
    int len=sprintf(get_buffer_addr(),"%s",str.c_str());
    buffer_pushed(len);
    return *this;
}
Logstream& Logstream::operator<<(int data)
{
    int len=sprintf(get_buffer_addr(),"%d",data);
    buffer_pushed(len);
    return *this;
}
Logstream &Logstream::operator<<( std::size_t data)
{
    int len=sprintf(get_buffer_addr(),"%ld",data);
    buffer_pushed(len);
    return *this;
}
Logstream &Logstream::operator<<(const double data)
{
    int len=sprintf(get_buffer_addr(),"%f",data);
    buffer_pushed(len);
    return *this;
}
