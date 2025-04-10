#include "Logbuffer.h"

Logbuffer::Logbuffer()
: idx_(0)
{
}

Logbuffer::~Logbuffer()
{
}

void Logbuffer::push(const char *data, std::size_t size)
{
    if(idx_<BUFFER_SIZE)
    {
        size=std::min(size,BUFFER_SIZE-idx_);
        std::copy(data,data+size,buffer_+idx_);
        idx_+=size;
    }
}

void Logbuffer::swap(Logbuffer &other)
{
    std::swap(buffer_, other.buffer_);
    std::swap(idx_, other.idx_);
}
std::size_t Logbuffer::size()
{
    return idx_;
}

std::size_t Logbuffer::avail()
{
    return BUFFER_SIZE-idx_;
}

void Logbuffer::clear()
{
    idx_=0;
}

char * Logbuffer::data()
{
    return buffer_;
}
