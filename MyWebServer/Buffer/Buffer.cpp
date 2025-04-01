#include"Buffer.h"
#include<unistd.h>
#include <stdexcept>
#include <sys/uio.h>
Buffer::Buffer()
: read_idx_(BEGIN_IDX),
write_idx_(BEGIN_IDX),
buffer_(INIT_BUFFER_SIZE)
{
}

Buffer::~Buffer()
{
}

std::size_t Buffer::write_capacity()
{
    return buffer_.size()-write_idx_;
}

std::size_t Buffer::read_capacity()
{
    return write_idx_-read_idx_;
}
std::size_t Buffer::prepend_capacity()
{
    return read_idx_;
}
char *Buffer::read_addr()
{
    return  get_addr(read_idx_);
}

char *Buffer::write_addr()
{
    return  get_addr(write_idx_);
}
char *Buffer::get_addr(std::size_t idx)
{
    if(idx>buffer_.size())
    return nullptr;
    return buffer_.data()+idx;
}

void Buffer::commit_read(size_t len)
{
    read_idx_+=len;
    if(read_idx_==write_idx_)
    {
        clear();
    }
}

void Buffer::commit_write(size_t len)
{
    write_idx_+=len;
}

int Buffer::fd_read(int fd,int &err)
{
    char sta_buf[STA_BUFFER_SIZE];

    iovec vec[2];
    vec[0].iov_base=write_addr();
    vec[0].iov_len=write_capacity();
    vec[1].iov_base=sta_buf;
    vec[1].iov_len=STA_BUFFER_SIZE;
    int len=::readv(fd,vec,2);
    if(len<0)
    {
        err=errno;
    }
    else if(len<=write_capacity())
    {
        commit_write(len);
    }
    else
    {
        commit_write(write_capacity());
        append(sta_buf,len-write_capacity());
    }

    return len;
}   

void Buffer::cstr_read(const char *str,size_t len)
{
    if(len<=write_capacity())
    {
        std::copy(str,str+len,write_addr());
        commit_write(len);
    }
    else
    {
        std::copy(str,str+write_capacity(),write_addr());
        commit_write(write_capacity());
        append(str,len-write_capacity());
    }
}

void Buffer::append(const char *str, std::size_t len)
{
    if(read_idx_<write_idx_)
    std::copy(read_addr(),write_addr(),BEGIN_IDX);

    write_idx_=BEGIN_IDX+read_capacity();
    read_idx_=BEGIN_IDX;

    if(len>write_capacity())
    {
        buffer_.resize(buffer_.size()+len-write_capacity());
    }   
    std::copy(str,str+len,write_addr());
    commit_write(len);

}
int Buffer::fd_write(int fd,int& err)
{
    int len=::write(fd,read_addr(),read_capacity());
    if(len<0)
    {
        err=errno;
    }
    else
    commit_write(len);

    return len;
}

void Buffer::prepend(const char *str,std::size_t len)
{
    if(len>prepend_capacity())
    {
        throw std::runtime_error("Buffer::prepend error");
    }
    std::copy(str,str+len,read_addr()-len);
    read_idx_-=len;
    
}
void Buffer::clear()
{
    read_idx_=BEGIN_IDX;
    write_idx_=BEGIN_IDX;
}