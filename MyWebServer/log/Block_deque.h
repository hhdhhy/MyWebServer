#pragma once
#include <mutex>
#include <condition_variable>
#include <deque>

template <typename T>
class Block_deque
{
public:
    Block_deque(std::size_t capacity=10000);
    ~Block_deque();

    bool push_front(const T& data);
    bool push_back(const T& data);

    bool pop_back(T& data,int timeout_ms=-1);
    bool pop_front(T& data,int timeout_ms=-1);

    bool front(T& data,int timeout_ms=-1);
    bool back(T& data,int timeout_ms=-1);

    void close();
    void clear();

    std::size_t size();
    bool empty();
    std::size_t capacity();

private:

    std::deque<T> deque_;
    std::size_t capacity_;
    std::mutex mutex_;
    bool close_;
    std::condition_variable cond_consumer_;
    std::condition_variable cond_producer_;
};


template <typename T>
Block_deque<T>::Block_deque(std::size_t capacity)
    : capacity_(capacity),close_(false)
{
    if(capacity_<=0)
    {
        throw std::exception();
    }
}

template <typename T>
Block_deque<T>::~Block_deque()
{
    close();
}

template <typename T>
void Block_deque<T>::close()
{
    std::unique_lock<std::mutex> lock(mutex_);
    deque_.clear();
    close_=true;
    cond_producer_.notify_all();
    cond_consumer_.notify_all();
}

template <typename T>
bool Block_deque<T>::push_front(const T& data)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (close_) 
        return false;
    cond_producer_.wait(lock, [this] { return close_ || deque_.size() < capacity_ ; });
    if (close_) 
        return false;
    

    deque_.push_front(data);

    cond_consumer_.notify_one();

    return true;
}

template <typename T>
bool Block_deque<T>::push_back(const T& data)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (close_) 
        return false;
    cond_producer_.wait(lock, [this] { return close_ || deque_.size() < capacity_ ; });
    if (close_) 
        return false;
    
    deque_.push_back(data);
    cond_consumer_.notify_one();

    return true;
}

template <typename T>
bool Block_deque<T>::pop_back(T& data,int timeout_ms)
{

    std::unique_lock<std::mutex> lock(mutex_);

    if(close_)
        return false;

    if(timeout_ms==-1)
    {
        cond_consumer_.wait(lock,[this]{return  close_ ||!deque_.empty();});

        if(close_)
        return false;
        
        data=deque_.back();
        deque_.pop_back();
        cond_producer_.notify_one();
        return true;
    }

    if(cond_consumer_.wait_for(lock,std::chrono::milliseconds(timeout_ms),
    [this]{return close_ ||!deque_.empty();})==std::cv_status::timeout)
    {
        return false;
    }
    if(close_)
        return false;
    
    data=deque_.back();
    deque_.pop_back();
    cond_producer_.notify_one();  
    return true;
}

template <typename T>
bool Block_deque<T>::pop_front(T& data,int timeout_ms)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if(close_)
        return false;

    if(timeout_ms==-1)
    {
        cond_consumer_.wait(lock,[this]{return  close_ ||!deque_.empty();});

        if(close_)
        return false;
        
        data=deque_.front();
        deque_.pop_front();
        cond_producer_.notify_one();
        return true;
    }

    if(cond_consumer_.wait_for(lock,std::chrono::milliseconds(timeout_ms),
    [this]{return close_ ||!deque_.empty();})==std::cv_status::timeout)
    {
        return false;
    }
    if(close_)
        return false;
    
    data=deque_.front();
    deque_.pop_front();
    cond_producer_.notify_one();  
    return true;
}

template <typename T>
bool Block_deque<T>::front(T &data,int timeout_ms)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if(close_)
        return false;
    

    if(timeout_ms==-1)
    {
        cond_consumer_.wait(lock,[this]{return close_ ||!deque_.empty();});
        if(close_)
        return false;
        data=deque_.front();

        return true;
    }

    if(cond_consumer_.wait_for(lock,std::chrono::milliseconds(timeout_ms),
    [this]{return close_ || !deque_.empty();})==std::cv_status::timeout)
    {
        return false;
    }
    if(close_)
    return false;
    data=deque_.front();

    return true;
}

template <typename T>
bool Block_deque<T>::back(T &data,int timeout_ms)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if(close_)
        return false;
    
    if(timeout_ms==-1)
    {
        cond_consumer_.wait(lock,[this]{return close_ ||!deque_.empty();});
        if(close_)
        return false;
        data=deque_.back();

        return true;
    }

    if(cond_consumer_.wait_for(lock,std::chrono::milliseconds(timeout_ms),
    [this]{return close_ ||!deque_.empty();})==std::cv_status::timeout)
    {
        return false;
    }
    if(close_)
    return false;
    data=deque_.back();
   
    return true;
}

template <typename T>
void Block_deque<T>::clear()
{
    std::unique_lock<std::mutex> lock(mutex_);
    deque_.clear();
    cond_producer_.notify_all();
}

template <typename T>
std::size_t Block_deque<T>::size()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return deque_.size();
}

template <typename T>
bool Block_deque<T>::empty()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return deque_.empty();
}

template <typename T>
std::size_t Block_deque<T>::capacity()
{
    return capacity_;
}
