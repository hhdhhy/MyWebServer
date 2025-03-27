#pragma once
#include <string>
#include <memory>

class Logstream
{

public:
    struct cstr
    {
        const char *str;
        std::size_t len;
    };
    struct log_data
    {
        std::unique_ptr<char[]> buffer_;
        std::size_t len_;
    };
    Logstream(int level,char * file,int line,char * func);
    ~Logstream();
    Logstream& operator<<(const char data);
    Logstream& operator<<(const char *str);
    Logstream& operator<<(cstr str);
    Logstream& operator<<(const std::string str);
    Logstream& operator<<(const int data);
    Logstream& operator<<(const double data);

    log_data data_;
    static const std::size_t BUFFER_SIZE = 256;
private:
    
    void push_time();
    void push_level();

    char *get_buffer_addr();
    void buffer_pushed(size_t);
    
};

