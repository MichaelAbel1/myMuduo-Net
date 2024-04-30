#include "Socket.h"
#include "Logger.h"
#include "InetAddress.h"


#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>
#include <errno.h>


Socket::~Socket()
{
    close(sockfd_);
}

void Socket::bindAddress(const InetAddress& localAddr)
{
    if (0 != ::bind(sockfd_, (sockaddr*)localAddr.getSockAddr(), sizeof(sockaddr_in)))
    {
        LOG_FATAL("Socket::bindAddress bind sockfd:%d fail  errno:%d\n ", sockfd_, errno);
    }
}

// 使得 socket 进入 listening state，以便接收新的连接请求。
void Socket::listen()
{
    if (0 != ::listen(sockfd_, 1024))
    {
        LOG_FATAL("listen sockfd:%d fail\n", sockfd_);
    }
}

/**
 * 实现了一个 TCP 服务器在有新的客户端连接到来时，
 * 接受这个连接并将新连接的相关信息保存到传入的 InetAddress 对象中。
 * 同时，返回新连接的文件描述符。
*/
int Socket::accept(InetAddress* peeraddr)
{
    // 定义一个地址结构体来接收 accept 接口返回的地址信息
    sockaddr_in addr;
    socklen_t len = sizeof addr;
    bzero(&addr, sizeof addr);
    // 用 accept 函数接受新的连接，并将新连接的文件描述符保存在 connfd 中
    int connfd = ::accept4(sockfd_, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd >= 0)
    {
        // 将新连接的地址设置到 peeraddr 对象中
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    if (::shutdown(sockfd_, SHUT_WR) < 0)
    {
        LOG_ERROR("sockets::shutdownWrite error");
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    if (0 != ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval))
    {
        LOG_FATAL("Socket::setTcpNoDelay fail errno:%d\n", errno);
    }
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    if (0 != ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval))
    {
        LOG_FATAL("Socket::setReuseAddr fail errno:%d\n", errno);
    }
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    if (0 != ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval))
    {
        LOG_FATAL("Socket::setReusePort fail errno:%d\n", errno);
    }
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    if (0 != ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval))
    {
        LOG_FATAL("Socket::setKeepAlive setKeepAlive fail errno:%d\n", errno);
    }
}
