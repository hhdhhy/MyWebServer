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
    typedef std::function<void (std::shared_ptr<Tcpconnection>, Buffer&,int)>  callback_message;
    typedef std::function<void (std::shared_ptr<Tcpconnection>, size_t)> callback_highwater_write;
    typedef std::function<void (std::shared_ptr<Tcpconnection>)> callback_complete_write;
    Tcpconnection(int connect_id,Loop* Loop,int socket_fd);
    ~Tcpconnection();

    void set_callback_connect(const callback_connect &callback);
    void set_callback_message(const callback_message &callback);
    void set_callback_close(const callback_connect &callback);
    void set_callback_highwater_write(const callback_highwater_write &callback);
    void set_callback_complete_write(const callback_complete_write &callback);
    
    void send(std::string str);
    void handle_connect();
    void handle_send(std::string str);
    void handle_write();
    void handle_read();
    void handle_close();
    void handle_error(); 
    void handle_destroy();
    void handle_close_write();
    void write_close();

    void set_state(State state);

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
    callback_highwater_write callback_highwater_write_;
    callback_complete_write callback_complete_write_;
    Buffer in_buffer_;
    Buffer out_buffer_;
};

