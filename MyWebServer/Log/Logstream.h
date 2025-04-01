#pragma once
#include <string>
#include <memory>

class Logstream
{
public:
    enum LogLevel
    {
        TRACE,  //详细流程跟踪
        DEBUG,  //开发调试信息
        INFO,   //常规运行状态
        WARN,   //预期外但可恢复的问题
        ERROR,  //严重错误但进程存活
        FATAL,  //致命错误需终止程序
        LEVEL_COUNT,
    };
    static const char* level_str[LEVEL_COUNT]; 
    
    struct cstr
    {
        const char *str;
        std::size_t len;
    };

    Logstream(LogLevel level,const char *file,int line,const char *func);
    ~Logstream();
    Logstream &operator<<(const char data);
    Logstream& operator<<(const char *str);
    Logstream& operator<<(const cstr &str);
    Logstream& operator<<(const std::string &str);
    Logstream& operator<<(int data);
    Logstream &operator<<(long long data);
    Logstream &operator<<(std::size_t data);
    Logstream& operator<<(const double data);

    
    static const std::size_t BUFFER_SIZE = 256;
private:
    
    LogLevel level_;
    void push_time();
    char data_[BUFFER_SIZE];
    std::size_t idx_ =0;
    char *get_buffer_addr();
    void buffer_pushed(size_t);

};
