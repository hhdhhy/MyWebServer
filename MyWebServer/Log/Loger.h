#pragma once
#include"Buffer.h"
#include"Block_deque.h"
#include"Logstream.h"
#include <memory>
#include <thread>

class Loger
{

public:

    ~Loger();
    void push(Logstream &logstream);
    static Loger& get_instance();
    void run();
    void run_thread();
private:   


    Loger();
    bool open_file(tm &day);
    bool close_file();

    static const std::size_t MAX_FILE_SIZE=1024*1024;

    Block_deque<Logstream::log_data> deque_;
    Buffer buffer_;

    int fd_;

    tm day_;
    std::size_t file_num_;
    std::size_t file_size_;

    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> stop_;
};


