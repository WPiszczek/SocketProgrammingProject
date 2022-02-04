#include "namespace.h"
#include "Client.h"

int HangmanGameNamespace::servFd;
int HangmanGameNamespace::epollFd;
std::vector<string> HangmanGameNamespace::usernames;
std::unordered_set<Client*> HangmanGameNamespace::clients;
std::vector<string> HangmanGameNamespace::roomnames;
std::unordered_map<std::string, HangmanGame*> HangmanGameNamespace::rooms;

bool HangmanGameNamespace::check_username(std::string name){
    if(std::find(usernames.begin(), usernames.end(), name) != usernames.end()) {
        return true;
    }
    usernames.push_back(name);
    return false;
}

bool HangmanGameNamespace::check_join_roomname(std::string name){
    for(auto it = rooms.begin(); it != rooms.end(); ++it){
        cout << "Checking: " <<it->first <<endl;
        if(name.compare(it->first) == 0){
            return true;
        }
    }
    return false;
}

bool HangmanGameNamespace::check_create_roomname(std::string name){
    for(auto it = rooms.begin(); it != rooms.end(); ++it){
        if(name.compare(it->first) == 0){
            return true;
        }
    }
    return false;
}

uint16_t HangmanGameNamespace::readPort(char * txt){
    char * ptr;
    auto port = strtol(txt, &ptr, 10);
    if(*ptr!=0 || port<1 || (port>((1<<16)-1))) error(1,0,"illegal argument %s", txt);
    return port;
}

void HangmanGameNamespace::setReuseAddr(int sock){
    const int one = 1;
    int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if(res) error(1,errno, "setsockopt failed");
}

void HangmanGameNamespace::ctrl_c(int){
    for(Client * client : clients) {
        delete client;
    }        
    close(servFd);
    printf("Closing server\n");
    exit(0);
}

string HangmanGameNamespace::packMessage(const char* message){
    string mess = string(message);
    int length = mess.length();
    std::ostringstream ss;
    ss << std::setw(3) << std::setfill('0') << length;
    string packed_mess = ss.str() + mess;
    return packed_mess;
}

string HangmanGameNamespace::unpackMessage(int _fd){
    char buf[999] = {0};
    ssize_t count = read(_fd, buf, 3);
    int mess_length = atoi(buf);
    count = read(_fd, buf, mess_length);
    buf[mess_length] = '\0';
    return string(buf);
}