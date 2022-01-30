#ifndef CLIENT_H
#define CLIENT_H

#include "Handler.h"

using namespace std;

class Client : public Handler {
    int _fd;
    std::string username;
    std::string in_roomname;


    bool amihost;
    int order;
    int remaining_lives;
    int gamestate;
    int score;

    bool password_setter;


public:
    Client(int fd);
    virtual ~Client();
    int fd() const;


    virtual void handleEvent(uint32_t events) override;
    bool check_player_joining_game(string buf);

    void write(const char * buffer, int count);

    void remove();
    void setRoomname(std::string roomname);
    void setOrder(int ord);
    void setAmihost(bool b);
    void setGamestate(int state);
    void showLobbies();
    void setScore(int points);
    void setRemainingLives(int lives);
    std::string getRoomname();
    std::string getUsername();
    int getOrder();
    bool getAmihost();
    int getScore();
    int getRemainingLives();
    int getGamestate();
    void quit_game();
    void ask_nick();
    void prepare_and_set_password(std::string password);
    void setPasswordSetterStatus(bool status);
};

#endif