#ifndef CLIENT_CPP
#define CLIENT_CPP

#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include <unordered_set>

#include "Client.h"


Client::Client(int _fd) {
    this->fd = _fd;
}

Client::~Client(){
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr);
    shutdown(fd, SHUT_RDWR);
    close(fd);
}

void Client::handleEvent(uint32_t events) {
    if(events & EPOLLIN) {
        char buffer[256];
        ssize_t count = read(fd, buffer, 256);
        if(count > 0)
            // sendToAllBut(fd, buffer, count);
            std::cout << buffer << std::endl;
        else
            events |= EPOLLERR;
    }
    if(events & ~EPOLLIN){
        remove();
    }
}

void Client::write(char * buffer, int count) {
    if(count != ::write(fd, buffer, count))
        remove();
}

void Client::remove() {
    printf("removing %d\n", fd);
    clients.erase(this);
    delete this;
}

int Client::getFd() const {
    return this->fd;
}



#endif