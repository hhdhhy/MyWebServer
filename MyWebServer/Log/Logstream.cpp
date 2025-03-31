#include"Logstream.h"
#include<cstring>
#include"Loger.h"

Logstream::Logstream(int level, char *file, int line, char *func)
:idx_(0)
{
    push_time();
    operator<<("__");
    push_level();
    operator<<("__");
    operator<<((cstr){file,strlen(file)});
    operator<<("__");
    operator<<(line);
    operator<<("__");
    operator<<((cstr){func,strlen(func)});
    operator<<("__:");
}
Logstream::~Logstream()
{
    operator<<("\n");
    Loger::get_instance().push(data_,idx_);
}

void Logstream::push_time()
{
    time_t now = time(NULL);
    tm time;
    localtime_r(&now,&time);

    int len=sprintf(get_buffer_addr(),"%d-%d-%d %d:%d:%d",time.tm_year+1900,time.tm_mon+1,time.tm_mday,time.tm_hour,time.tm_min,time.tm_sec);

    buffer_pushed(len);
}

void Logstream::push_level()
{

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
}
Logstream& Logstream::operator<<(const char *str)
{
    int len=sprintf(get_buffer_addr(),"%s",str);
    buffer_pushed(len);
}
Logstream& Logstream::operator<<(const cstr &str)
{
    int len=sprintf(get_buffer_addr(),"%.*s",str.len,str.str);
    buffer_pushed(len);
}
Logstream& Logstream::operator<<(const std::string& str)
{
    int len=sprintf(get_buffer_addr(),"%s",str.c_str());
    buffer_pushed(len);
}
Logstream& Logstream::operator<<(const int data)
{
    int len=sprintf(get_buffer_addr(),"%d",data);
    buffer_pushed(len);
}
Logstream& Logstream::operator<<(const double data)
{
    int len=sprintf(get_buffer_addr(),"%f",data);
    buffer_pushed(len);
}

