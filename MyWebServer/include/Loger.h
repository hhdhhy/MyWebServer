#pragma once
#include"Logbuffer.h"
#include"Logstream.h"
#include <memory>
#include <thread>
#include <condition_variable>
#include <functional>
#include  <vector>
#include <atomic>
class Loger
{
public:

    static Logstream::LogLevel get_log_level();
    static void set_log_level(Logstream::LogLevel level);

    ~Loger();
    void push(char *data, std::size_t len,Logstream::LogLevel level);
    static Loger& get_instance();
    void loop();
    void run_thread();
private:
    static FILE* get_fp(time_t &time_day,std::size_t &file_num,std::size_t &file_size,std::string&file_name);
    void write_to_file();
    Loger();
    
    static const std::size_t MAX_FILE_SIZE=1024*1024*40;
    static const std::size_t OUT_BUFFER_SIZE=64*1024;

    static const std::size_t DAY_SECONDS=24*60*60;

    std::mutex mutex_;
    std::condition_variable cond_;
    std::vector<std::unique_ptr<Logbuffer>> buffers_;
    std::unique_ptr<Logbuffer> cur_buffer_;
    std::unique_ptr<Logbuffer> nex_buffer_;
    std::unique_ptr<Logbuffer> buffer_n1_;
    std::unique_ptr<Logbuffer> buffer_n2_;
    std::vector<std::unique_ptr<Logbuffer>> buffers_write_;

    int next_buffer_idx_;
    std::unique_ptr<FILE,std::function<void(FILE*)>> fp_;
    char out_buffer_[OUT_BUFFER_SIZE];

    std::string file_name_;
    time_t time_day_;//文件名的日期
    std::size_t file_num_;
    std::size_t file_size_;
    static std::atomic<Logstream::LogLevel> LOG_LEVEL;
    
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> stop_;

    
};
#define LOG_TRACE if (Loger::get_log_level() <= Logstream::TRACE) \
  Logstream(Logstream::TRACE,__FILE__,__LINE__,__FUNCTION__)
#define LOG_DEBUG if (Loger::get_log_level() <= Logstream::DEBUG) \
  Logstream(Logstream::DEBUG,__FILE__,__LINE__,__FUNCTION__)
#define LOG_INFO if (Loger::get_log_level() <= Logstream::INFO) \
  Logstream(Logstream::INFO,__FILE__,__LINE__,__FUNCTION__)
#define LOG_WARN Logstream(Logstream::WARN,__FILE__,__LINE__,__FUNCTION__)
#define LOG_ERROR Logstream(Logstream::ERROR,__FILE__,__LINE__,__FUNCTION__)
#define LOG_FATAL Logstream(Logstream::FATAL,__FILE__,__LINE__,__FUNCTION__)