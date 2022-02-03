#ifndef HANGMANGAME_H
#define HANGMANGAME_H

#include "namespace.h"

using namespace HangmanGameNamespace;

class Client;

class HangmanGame{
    std::string room_name;
    // deskryptor plikow to klucz w mapie
    std::unordered_map<int, Client*> players_in_game;
    Client* host;


    int num_of_players;
    bool is_on;

    int max_order_id; // order of players joining game set as the attribute of the player
    
    int current_round_number;
    int num_of_rounds;
    int password_setter_fd;

    Password password;
    
    std::unordered_map<int, std::string> leavers_round_results;
    std::map<int, std::string> game_results;
 
public:
    HangmanGame(string room);
    ~HangmanGame();

    std::string getRoomName();

    int getPasswordSetterFd();

    void setPasswordSetterFd(int clientFd);

    // careful-  increments max_order by default!
    int getOrder();

    void addPlayer(int clientFd, Client* newplayer);

    void setGameStatus(bool status);

    void setHost(Client* newhost);

    void setRoundNumber(int rnum);

    int getRoundNumber();

    int getCurrentRoundNumber();

    void removePlayer(int clientFd);

    void showPeopleInGame();
    
    void sendToAll(std::string s);
    
    void sendTo(std::string s, int clientFd);
    
    int getPlayerCount();
    
    bool getGameStatus();
    
    void setPassword(Password new_password);
    
    void printGame();
    
    void printGameWhenJoining(int clientFd);
    
    void setNewHostAfterGame();
    
    void guess_letter(int clientFd, char letter);
    
    std::string roundResults();
    
    void showGameResults();
    
    void cleanAfterRound();
};

#endif