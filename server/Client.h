#ifndef CLIENT_H
#define CLIENT_H

#include "Handler.h"

using namespace std;

class Client : public Handler {
    int _fd;
    std::string username;
    std::string in_roomname = "";


    int remaining_lives = 2;
    int gamestate = 0;


public:
    Client(int fd);
    virtual ~Client();
    int fd() const;


    virtual void handleEvent(uint32_t events) override;
    bool check_player_joining_game(string buf);

    void write(const char * buffer, int count);

    void remove();
    void setRoomname(std::string roomname);
    std::string getRoomname();

    std::string getUsername();

    void ask_nick();
};

#endif