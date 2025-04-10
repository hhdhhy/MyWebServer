#include <string>
#include <Tcpserver.h>
#include <Loger.h>
#include <sys/stat.h>
#include <sstream>
#include <iostream>


class EchoServer
{
public: 
    typedef std::shared_ptr<Tcpconnection> TcpConnectionPtr;
    EchoServer(Loop *loop, const sockaddr_in &addr, const std::string &name)
        : server_(loop, addr)
        , loop_(loop)
    {
        // 注册回调函数
        server_.set_callback_connect([this](TcpConnectionPtr ptr){onConnection(ptr);});
        server_.set_callback_message([this](TcpConnectionPtr ptr, Buffer &buf){onMessage(ptr, buf);});
    }
    void start()
    {
        server_.start();
    }

private:
    // 连接建立或断开的回调函数
    void onConnection(const TcpConnectionPtr &conn)   
    {
        if (conn->connected())
        {
            LOG_INFO<<"Connection UP :"<<conn->get_addr();
        }
        else
        {
            LOG_INFO<<"Connection DOWN :"<<conn->get_addr();
        }
    }

    // 可读写事件回调
    void onMessage(const TcpConnectionPtr &conn, Buffer &buf)
    {
        std::string msg=std::move(buf.to_string());
        conn->send(std::move(msg));
        // conn->shutdown();   // 关闭写端 底层响应EPOLLHUP => 执行closeCallback_
    }
    Tcpserver server_;
    Loop *loop_;

};

int main(int argc,char *argv[]) 
{
    Loger::get_instance().run_thread();
    Loger::set_log_level(Logstream::DEBUG);

    //第三步启动底层网络模块
    Loop loop;
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    LOG_DEBUG<<"Server Start";
    EchoServer server(&loop, addr, "EchoServer");
    server.start();
 // 主loop开始事件循环  epoll_wait阻塞 等待就绪事件(主loop只注册了监听套接字的fd，所以只会处理新连接事件)
    std::cout << "================================================Start Web Server================================================" << std::endl;
    loop.run();
    std::cout << "================================================Stop Web Server=================================================" << std::endl;

}