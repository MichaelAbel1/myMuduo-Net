#pragma once

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"

#include <memory>
#include <string>
#include <atomic>

class Channel;
class EventLoop;
class Socket;

/**
 * TcpServer => Acceptor => 有一个新用户连接，通过accept函数拿到connfd
 * 打包成TcpConnection 设置相应的回调 => channel => Poller => Channel的回调操作
*/
class TcpConnection : noncopyable,
                public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop,
                const std::string& nameArg,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    /**
     * const std::string&
        这个部分表示函数返回的是一个指向 const std::string 的引用。
        这意味着你不能通过这个返回的引用来修改 name_。
        const 在函数声明的末尾
        这个 const 修饰整个函数，表示这个是一个常量成员函数。
        这就意味着这个函数不会（也不能）修改类的任何成员变量。
    */
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }

    bool connected() const { return state_ == kConnected; }
    void send(const void* message, int len);
    void send(const std::string& buf);
    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb)
    {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallback_ = cb;
    }

    void setCloseCallback(const CloseCallback& cb)
    {
        closeCallback_ = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        writeCompleteCallback_ = cb;
    }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb)
    {
        highWaterMarkCallback_ = cb;
    }


    // 连接建立
    void connectEstablished();
    // 连接销毁
    void connectDestroyed();

    
private:
    enum StateE {kDisconnected, kConnecting, kConnected, kDisconnecting};

    void setState(StateE state)
    {
        state_ = state;
    }
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* data, size_t len);

    void shutdownInLoop();

    // 这里绝对不是baseLoop， 因为TcpConnection都是在subloop里管理的
    EventLoop* loop_; 
    const std::string name_;
    InetAddress localAddr_;
    InetAddress peerAddr_;
    std::atomic_int state_;
    bool reading_;

    // 这里和Acceptor类似 Acceptor => mainloop   TcpConnection => subLoop
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    ConnectionCallback connectionCallback_; // 有新连接时回调
    MessageCallback messageCallback_; // 有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成以后的回调
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;

    size_t highWaterMark_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;
};