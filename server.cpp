#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>	
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "error.h"

#define PORT 8080
#define MAX_CLIENTS 2

std::vector<std::string> usernames;
// std::unordered_map<int, User> users;

constexpr char words[3][20] = {
    "datagram",
    "password",
    "protocol"
};

struct Client{
    std::string username;
    int remaining_lives = 9;
    int socket;
} clients[MAX_CLIENTS];

struct HangmanGame{
    bool is_on = 0;
    int num_of_players = 0;
    int current_round_number = 0;
    int num_of_rounds = 0;
    char word_to_guess[20];
} game;

class Player{
    private:
        //socket
        int socket;

    public:
        std::string username;
        int remaining_lives;
};


bool check_username(std::string name){
    if(std::find(usernames.begin(), usernames.end(), name) != usernames.end()) {
        return true;
    }
    usernames.push_back(name);
    return false;
}

constexpr const int one = 1;

int main(int argc, char ** argv){
    if(argc!=2)
        error(1,0,"Usage: %s <port>", argv[0]);
    
    sockaddr_in localAddress{
        .sin_family = AF_INET,
        .sin_port   = htons(atoi(argv[1])),
        .sin_addr   = {htonl(INADDR_ANY)}
    };
    
    int servSock = socket(PF_INET, SOCK_STREAM, 0);
    setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    
    if(bind(servSock, (sockaddr*) &localAddress, sizeof(localAddress)))
        error(1,errno,"Bind failed!");
    
    listen(servSock, 1);

    while(true) {
        accept(servSock, nullptr, nullptr);
        printf("Accepted a new connection, ignoring it.\n");
    }
}
