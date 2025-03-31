#pragma once 
#include <bits/c++config.h>
#include <mutex>
#include <atomic>
class Logbuffer
{
public:
    Logbuffer();
    ~Logbuffer();

    void push(const char *data, std::size_t size);

    void swap(Logbuffer &other);

    std::size_t size();
    std::size_t avail();
    void clear();
    char *data();

private:
    static const std::size_t BUFFER_SIZE = 1024*64;
    char buffer_[BUFFER_SIZE];

    std::size_t idx_;
};

