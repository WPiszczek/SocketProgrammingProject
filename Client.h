#ifndef CLIENT_H
#define CLIENT_H
#include "globals.h"

using namespace std;


class Client : public Handler {
    int _fd;
    std::string username;
    int remaining_lives = 2;
    int gamestate = 0;


public:
    Client(int fd);
    virtual ~Client();
    int fd() const;


    virtual void handleEvent(uint32_t events) override;

    void write(const char * buffer, int count);

    void remove();

    void askNick();

    void showAllRooms();
};

#endif