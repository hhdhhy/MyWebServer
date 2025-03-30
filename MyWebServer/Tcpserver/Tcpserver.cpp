#include "Tcpserver.h"
Tcpserver::Tcpserver(sockaddr_in addr)
:loop_(),acceptor_(loop_,addr)
{
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

    std::shared_ptr<Tcpconnection> connection = std::make_shared<Tcpconnection>(ioloop,next_connection_id_,fd);
    connections_[next_connection_id_]= connection;
    next_connection_id_++;
    connection->set_callback_connect(callback_connect_);
    connection->set_callback_message(callback_message_);
    connection->set_callback_highwater_write(callback_highwater_write_);
    connection->set_callback_complete_write(callback_complete_write_);
    connection->set_callback_close([this](const std::shared_ptr<Tcpconnection> &conn){handle_close(conn);});
    ioloop->add_run_callback([&connection](){connection->handle_connect();});//io线程中执行
}


void Tcpserver::handle_close(const std::shared_ptr<Tcpconnection> &conn)
{
    loop_->add_run_callback([this,&conn](){handle_close_mainthread(conn);});//先从主线程删除conn
}

void Tcpserver::handle_close_mainthread(const std::shared_ptr<Tcpconnection> &conn)
{
    connections_.erase(conn->get_connect_id());//先从主线程删除conn
    conn->get_loop()->add_run_callback([conn](){ conn->handle_destroy();});//在io线程删除conn
}
