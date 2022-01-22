#ifndef STRUCTS_H
#define STRUCTS_H

#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <unordered_set>
#include <unordered_map>
#include <signal.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstring>


using namespace std;


struct HangmanGame{
    bool is_on = 0;
    int num_of_players = 0;
    int current_round_number = 0;
    int num_of_rounds = 0;
    char word_to_guess[20];
};

struct Player{
    std::string username = "";
    int remaining_lives = 2;
};

struct Handler {
    virtual ~Handler(){}
    virtual void handleEvent(uint32_t events) = 0;
};

#endif