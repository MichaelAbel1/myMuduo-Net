#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <stdio.h>


int main() {
    int evfd = eventfd(10, EFD_CLOEXEC | EFD_NONBLOCK);
    uint64_t wdata = 0;
    uint64_t rdata = 0;

    if (eventfd_read(evfd, &rdata) == -1) {
        perror(NULL);
        if (errno != EAGAIN)return 0;
    }
    std::cout << "Init read : " << rdata << std::endl;  //读计数器初始值

    wdata = 20;

    if (eventfd_write(evfd, wdata) == -1) //父进程写20
    {
        perror(NULL);
        return 0;
    }
    std::cout << "parent write : " << wdata << std::endl;

    if (fork() == 0) {
        wdata = 30;
        if (eventfd_read(evfd, &rdata) == -1) //子进程读计数器
        {
            perror(NULL);
            return 0;
        }
        std::cout << "child read : " << rdata << std::endl;
        if (eventfd_write(evfd, wdata) == -1)  //子进程写30
        {
            perror(NULL);
            return 0;
        }
        std::cout << "child write : " << wdata << std::endl;
        exit(0);
    }
    wait(NULL);
    if (eventfd_read(evfd, &rdata) == -1)   //父进程读计数器
    {
        perror(NULL);
        return 0;
    }
    std::cout << "parent read : " << rdata << std::endl;

    return 0;
}

