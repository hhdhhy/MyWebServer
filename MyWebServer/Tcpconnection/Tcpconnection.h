#pragma once
#include "Buffer.h"
#include "Acceptor.h"
#include <functional>
#include "Timer.h"
class Tcpconnection 
: public std::enable_shared_from_this<Tcpconnection>
{

public:
    enum State
    {
        CONNECTING,
        CONNECTED,
        CLOSING,
        CLOSED
    };

    typedef std::function<void(std::shared_ptr<Tcpconnection>)> callback_connect;
    typedef std::function<void (std::shared_ptr<Tcpconnection>, std::shared_ptr<Buffer>,int)>  callback_message;
   
    Tcpconnection(int connect_id,Loop* Loop,int socket_fd);
    ~Tcpconnection();

    void set_callback_connect(const callback_connect &callback);
    void set_callback_message(const callback_message &callback);
    void set_callback_close(const callback_connect &callback);

    void handle_write();
    void handle_read();
    void handle_close();
    void handle_error();
    void handle_close_write();
    void handle_destroy();
    void write_close();


    int get_connect_id();
    Loop* get_loop();
private:
    
    int connect_id_;
    Channel channel_;
    std::atomic<State> state_;
    Loop* loop_;

    callback_connect callback_connect_;
    callback_message callback_message_;
    callback_connect callback_close_;

    std::shared_ptr<Buffer> in_buffer_;
    std::shared_ptr<Buffer> out_buffer_;
};

