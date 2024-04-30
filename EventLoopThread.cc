#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
    const std::string& name)
    : loop_(nullptr)
    , exiting_(false)
    , thread_(std::bind(&EventLoopThread::threadFunc, this), name)
    , mutex_()
    , cond_()
    , callback_(cb)
{

}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    thread_.start(); // 启动底层新线程

    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // 使用while避免虚假唤醒
        while ( loop_ == nullptr )
        {
            // 使当前线程进入阻塞状态，并释放互斥锁 lock
            // 当其他线程通过 cond_.notify_one() 或 cond_.notify_all() 唤醒等待的线程时
            // 当前线程会重新获取互斥锁 lock，然后从 cond_.wait() 调用中返回，并继续执行后续的代码。
            cond_.wait(lock);
        }
        loop = loop_;
        
    }
    return loop;
}

// 下面这个方法，是在单独的新线程里面运行的
void EventLoopThread::threadFunc()
{
    // 创建一个独立的EventLoop，和上面的线程是一一对应的
    // one loop per thread
    EventLoop loop;

    if (callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop(); // EventLoop loop => Poller.poll
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}