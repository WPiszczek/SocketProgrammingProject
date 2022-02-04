#ifndef NAMESPACE_H
#define NAMESPACE_H

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
#include <signal.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <map>
#include <memory>
#include <type_traits>
#include <iomanip>
#include <sstream>

using namespace std;

class HangmanGame;
class Client;

namespace HangmanGameNamespace {

    template<typename Base, typename T>
    inline bool instanceof(const T*) {
    return is_base_of<Base, T>::value;
    }

    #define PROMPT(x)\
            std::string msg(x);\
            write(msg.c_str(), msg.length());\

    #define PROMPT_STR(x)\
            write(x.c_str(), x.length());\


    #define MAX_ORDER 100001
    #define MAX_LIVES 2
    #define MIN_PLAYERS_IN_GAME 3

    extern std::vector<string> usernames;
    extern std::unordered_set<Client*> clients;
    extern std::vector<string> roomnames;
    extern std::unordered_map<std::string, HangmanGame*> rooms;

    bool check_username(std::string name);
    bool check_join_roomname(std::string name);
    bool check_create_roomname(std::string name);
    
    extern int servFd;
    extern int epollFd;

    void ctrl_c(int);
    uint16_t readPort(char * txt);
    void setReuseAddr(int sock);

    string packMessage(const char* message);
    string unpackMessage(int _fd);

    struct Password{
        std::string correct_pass;
        int num_of_letters;
        std::string underscore;
        std::string guessed_letters;
        int guessed_letters_correctly;
    };

    struct Handler {
        virtual ~Handler(){}
        virtual void handleEvent(uint32_t events) = 0;
    };
}

#endif