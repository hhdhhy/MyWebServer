#include"Epoll.h"
#include"Channel.h"
#include<vector>
#include<exception>
#include <system_error>
#include<sys/epoll.h>
#include<unistd.h>
Epoll::Epoll()
: epollfd_(epoll_create1(EPOLL_CLOEXEC)),revents_(init_revent_size)
{
    if(epollfd_ < 0)
    {
        throw std::exception();
    }
}


Epoll::~Epoll()
{
    close(epollfd_);   
}

void Epoll::poll(std::vector<Channel*>&active_channels)
{
    int num_events=0;
    do
    {
        num_events=epoll_wait(epollfd_, revents_.data(), revents_.size(), -1);
    }while (num_events==-1&&errno==EINTR);


    if(num_events < 0 && errno != EINTR)
    {
        throw std::exception();
    }

    for (int i = 0; i < num_events; i++)
    {
        int fd=revents_[i].data.fd;
        channels_[fd]->set_revents(revents_[i].events);
        active_channels.push_back(channels_[fd]);
    }

    if(num_events == revents_.size())
    {
        revents_.resize(revents_.size()*2);
    }

}

