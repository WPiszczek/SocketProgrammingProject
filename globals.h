#ifndef GLOBALS_H
#define GLOBALS_H

#include "structs.h"

using namespace std;

class Client;

extern int servFd;
extern int epollFd;

constexpr char words[3][20] = {
    "datagram",
    "password",
    "protocol"
};

extern std::vector<string> usernames;
extern std::unordered_set<Client*> clients;
extern std::unordered_set<Player*> players;
extern std::unordered_map<string, vector<Client*>> rooms;

bool check_username(std::string name);

void ctrl_c(int);

void sendToAllBut(int fd, char * buffer, int count);

uint16_t readPort(char * txt);

void setReuseAddr(int sock);

#endif