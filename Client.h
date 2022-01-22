#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include "Handler.h"

using namespace std;

class Client : public Handler {
    int fd;
    std::string username;

public:
    Client(int _fd);
    virtual ~Client();

    int getFd() const;
    int getEpollFd();
    void write(char * buffer, int count);
    void remove();

    virtual void handleEvent(uint32_t events) override;

};

extern std::unordered_set<Client*> clients;
extern int epollFd;

#endif