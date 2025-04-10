#pragma once

#include<vector>
#include<string>
#include<atomic>

class Buffer
{

public:
    Buffer();
    ~Buffer();

    std::size_t write_capacity();
    std::size_t read_capacity();
    std::size_t prepend_capacity();

    int fd_read(int fd, int &err);
    void cstr_read(const char *str, size_t len);
    int fd_write(int fd, int &err);
    void prepend(const char *str,std::size_t len);//向前追加

    void clear();
    std::string to_string();
private:

    void append(const char *str,std::size_t len);//向后追加

    char *get_addr(std::size_t idx);//获取地址
    char *read_addr();
    char *write_addr();
 

    void commit_read(size_t len);
    void commit_write(size_t len);

    /* data */
    static const std::size_t INIT_BUFFER_SIZE = 1024*8;
    static const std::size_t STA_BUFFER_SIZE = 65536;
    static const std::size_t BEGIN_IDX = 8;

    std::vector<char> buffer_;

    std::atomic<std::size_t> read_idx_;
    std::atomic<std::size_t> write_idx_;

};

