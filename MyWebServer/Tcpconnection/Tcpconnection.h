#pragma once
#include "Buffer.h"
#include "Acceptor.h"
#include <functional>
#include "Timer.h"
class Tcpconnection
{

public:
    enum State
    {
        CONNECTING,
        CONNECTED,
        CLOSING,
        CLOSED
    };

    typedef std::function<void(int,sockaddr_in)> callback_connect;
    typedef std::function<void (std::shared_ptr<Tcpconnection>, std::shared_ptr<Buffer>, Timer::Timeus)>  callback_message;
   
    Tcpconnection(int connect_id,std::shared_ptr<Loop> Loop,int socket_fd);
    ~Tcpconnection();

    void set_callback_connect(const callback_connect &callback);
    void set_callback_message(const callback_message &callback);

private:
    
    int connect_id_;
    std::shared_ptr<Channel> channel_;
    std::atomic<State> state_;
    std::shared_ptr<Loop> Loop_;

    callback_connect callback_connect_;
    callback_message callback_message_;

    std::shared_ptr<Buffer> in_buffer_;
    std::shared_ptr<Buffer> out_buffer_;
};

