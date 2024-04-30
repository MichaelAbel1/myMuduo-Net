#pragma once

#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>

class EventLoop;
// class Timestamp; // 如果后面用Timestamp对象就不能只声明，因为不能确定对象大小，指针就可以
/**
 * 理清楚 EventLoop， Channel，Poller之间的关系 <<= Reactor模型上对应Demultiplex
 * Channel 理解为通道，封装了sockfd和其感兴趣的event，如EPOLLIN,EPOLLOUT事件
 * 还绑定了poller放回的具体事件
*/
class Channel : noncopyable
{
public:
    // typedef std::function<void()> EventCallback;
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    // fd得到poller通知后，处理事件
    void handleEvent(Timestamp receiveTime);

    // 设置回调函数对象
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    //防止手动remove channel后，还在使用channel
    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) 
    { 
        // 检查 Channel 对象是否为空
        if (this == nullptr) {
            throw std::invalid_argument("Channel object is null in set_revents()");
        }
        revents_ = revt; 
    }
    bool isNoneEvent() const { return events_ == KNoneEvent; }
    
    // 设置fd相应的事件状态
    void enableReading() { events_ |= KReadEvent; update(); }
    void disableReading() { events_ &= ~KReadEvent; update(); }
    void enableWriting() { events_ |= KWriteEvent; update(); }
    void disableWriting() { events_ &= ~KWriteEvent; update(); }
    void disableAll() { events_ = KNoneEvent; update(); }
    bool isWriting() const { return events_ & KWriteEvent; }
    bool isReading() const { return events_ & KReadEvent; }


    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    // one loop per thread
    EventLoop* ownerLoop() { return loop_; }
    void remove();


private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);
    
    static const int KNoneEvent;
    static const int KReadEvent;
    static const int KWriteEvent;
    
    EventLoop *loop_; // 事件循环
    const int fd_; // fd, Poller监听的对象
    int events_; // 注册fd感兴趣的事件
    int revents_; // poller放回的具体发生的事件
    int index_;


    std::weak_ptr<void> tie_; //防止手动remove channel后，还在使用channel
    bool tied_;

    // 因为channel通道里面能够获取fd最终发生的具体的事件revents，所以它负责调用具体事件的回调操作
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
    
};