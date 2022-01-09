/* 
Guide to network programming: https://beej.us/guide/bgnet/html/
*/


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
#define MAX_CLIENTS 2 // to be changed to an arbitrary number

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
    printf("Starting server...\n");
    sockaddr_in serverAddress{
        .sin_family = AF_INET,
        .sin_port   = htons(PORT),
        .sin_addr   = {htonl(INADDR_ANY)}
    };

    int serverSock;
    if(serverSock = socket(AF_INET, SOCK_STREAM, 0) < 0)
        error(1, errno, "Creating socket failed!");

    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if(bind(serverSock, (sockaddr*) &serverAddress, sizeof(serverAddress)) < 0)
        error(1,errno,"Bind failed!");
    

    if(listen(serverSock, 1) < 0)
        error(1,errno,"Listen failed!");

    while(true) {
        accept(serverSock, nullptr, nullptr);
        printf("Accepted a new connection, ignoring it.\n");
    }
}
