#include "Tcpserver.h"
#include "Loger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
Tcpserver::Tcpserver(Loop* loop,sockaddr_in addr)
:loop_(loop),acceptor_(loop,addr), started_(0),threadpool_(new Loopthreadpool(4))
{
    acceptor_.set_connect_callback([this](int fd,sockaddr_in addr){handle_connect(fd,addr);});
}

Tcpserver::~Tcpserver()
{
}

void Tcpserver::set_callback_connect(const callback_connect& callback)
{
    callback_connect_ = callback;
}

void Tcpserver::set_callback_message(const callback_message& callback)
{
    callback_message_ = callback;
}

void Tcpserver::set_callback_close(const callback_connect &callback)
{
    callback_close_ = callback;
}

void Tcpserver::set_callback_highwater_write(const callback_highwater_write &callback)
{
    callback_highwater_write_ = callback;
}

void Tcpserver::set_callback_complete_write(const callback_complete_write &callback)
{
    callback_complete_write_ = callback;
}

void Tcpserver::handle_connect(int fd, sockaddr_in addr)
{
    
    Loop * ioloop= threadpool_->get_next_loop();

    char addr_cstr[20];
    inet_ntop(AF_INET,&addr.sin_addr,addr_cstr,sizeof(addr_cstr));
    std::shared_ptr<Tcpconnection> connection 
    = std::make_shared<Tcpconnection>(next_connection_id_,ioloop,fd,std::string(addr_cstr,strlen(addr_cstr)));
    LOG_INFO<<"accept in main loop... fd:"<<fd<<" addr:"<<addr_cstr;
    connections_[next_connection_id_]= connection;
    next_connection_id_++;
    next_connection_id_%=4;
    connection->set_callback_connect(callback_connect_);
    connection->set_callback_message(callback_message_);
    connection->set_callback_highwater_write(callback_highwater_write_);
    connection->set_callback_complete_write(callback_complete_write_);
    connection->set_callback_close([this](const std::shared_ptr<Tcpconnection> &conn){handle_close(conn);});
    LOG_DEBUG<<"io loop:"<<ioloop->wakeup_channel_.get_fd()<<" handle_connect()";
    LOG_DEBUG<<"loop:"<<loop_->wakeup_channel_.get_fd()<<" handle_connect()";
    ioloop->add_run_callback([connection](){LOG_DEBUG<<"handle_connect()";    connection->handle_connect();});//io线程中执行
}


void Tcpserver::handle_close(const std::shared_ptr<Tcpconnection> &conn)
{
    loop_->add_run_callback([this,&conn](){handle_close_mainthread(conn);});//先从主线程删除conn
}

void Tcpserver::handle_close_mainthread(const std::shared_ptr<Tcpconnection> &conn)
{
    LOG_INFO<<"close in main loop... conn:"<<conn->get_connect_id();
    connections_.erase(conn->get_connect_id());//先从主线程删除conn
    conn->get_loop()->add_run_callback([conn](){ conn->handle_destroy();});//在io线程删除conn
}

void Tcpserver::start()
{
    if (started_.fetch_add(1) == 0)    // 防止一个TcpServer对象被start多次
    {
        threadpool_->start();    // 启动底层的loop线程池
        loop_->add_run_callback([&](){acceptor_.listen();});

    }
}