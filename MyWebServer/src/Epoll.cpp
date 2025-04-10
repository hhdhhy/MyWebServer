#include"Epoll.h"

#include<vector>
#include<exception>
#include <system_error>
#include<sys/epoll.h>
#include<unistd.h>
#include "Loger.h"
#include "Channel.h"
Epoll::Epoll()
: epoll_fd_(epoll_create1(EPOLL_CLOEXEC)),revents_(init_revent_size),channels_()
{
    if(epoll_fd_ < 0)
    {
        LOG_FATAL<<"epoll_create1 error";
    }
}

Epoll::~Epoll()
{
    close(epoll_fd_);   
}

void Epoll::poll(int timeout_ms,std::vector<Channel*>&active_channels)
{
    int num_events=0;
    do
    {
        num_events=epoll_wait(epoll_fd_, revents_.data(), revents_.size(), timeout_ms);
    }while (num_events==-1&&errno==EINTR);


    if(num_events < 0 && errno != EINTR)
    {
        LOG_ERROR << "epoll wait error";
    }

    LOG_INFO << "epoll wait num_events:" << num_events;
    for (int i = 0; i < num_events; i++)
    {
        int fd=revents_[i].data.fd;
        channels_[fd]->set_revents(revents_[i].events);
        active_channels.push_back(channels_[fd]);
    }

    
    if(num_events == revents_.size())
    {
        revents_.resize(revents_.size()*2);
        LOG_INFO << "revents_ resize to " << revents_.size() ;
    }

}

void Epoll::delete_channel(Channel* channel)
{
    if(channels_.count(channel->get_fd())==0)
    {
        LOG_WARN<<"not find channel";
    }
    else
    {
        if(epoll_ctl(epoll_fd_,EPOLL_CTL_DEL,channel->get_fd(),NULL) < 0)
        {
            LOG_ERROR << "epoll_ctl del error";
        }
        else
        {
            channels_.erase(channel->get_fd());
            channel->is_in_epoll_=0;
        }
    }
}

void Epoll::update_channel(Channel* channel)
{
    
    if(channel->is_in_epoll()==0)
    {
        if(channel->get_events())
        add_channel(channel);
    }
    else if(channel->get_events() == 0)
    {
        delete_channel(channel);
    }
    else
    {
         epoll_event ev;  
         ev.events=channel->get_events();
         ev.data.fd=channel->get_fd();
         if(epoll_ctl(epoll_fd_,EPOLL_CTL_MOD,channel->get_fd(),&ev) < 0)
         {
                LOG_ERROR << "epoll_ctl add error";
         }
    }
}

void Epoll::add_channel(Channel* channel)
{
    if(channels_.find(channel->get_fd())==channels_.end())
    {
        epoll_event ev;
        ev.events=channel->get_events();
        ev.data.fd=channel->get_fd();
        if(epoll_ctl(epoll_fd_,EPOLL_CTL_ADD,channel->get_fd(),&ev) < 0)
        {
            LOG_ERROR << "epoll_ctl add error";
        }
        else
        {
            channels_[channel->get_fd()]=channel;
            channel->is_in_epoll_=1;
        }
    }
    else
    {
        LOG_WARN<<"channel is in epoll";
    }
}
