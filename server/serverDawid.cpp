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

#include "Handler.h"
#include "Client.h"

using namespace std;


template<typename Base, typename T>
inline bool instanceof(const T*) {
   return is_base_of<Base, T>::value;
}

#define PROMPT(x)\
        std::string msg(x);\
        write(msg.c_str(), msg.length());\

#define PROMPT_STR(x)\
        write(x.c_str(), x.length());\



constexpr char words[3][20] = {
    "datagram",
    "password",
    "protocol"
};


std::vector<string> usernames;
bool check_username(std::string name){
    if(std::find(usernames.begin(), usernames.end(), name) != usernames.end()) {
        return true;
    }
    usernames.push_back(name);
    return false;
}

class HangmanGame;

std::unordered_set<Client*> clients;
std::unordered_set<HangmanGame*> games;

std::vector<string> roomnames;
bool check_create_roomname(std::string name){
    if(std::find(roomnames.begin(), roomnames.end(), name) != roomnames.end()) {
        return true;
    }
    roomnames.push_back(name);
    return false;
}

bool check_join_roomname(std::string name){
    if(std::find(roomnames.begin(), roomnames.end(), name) != roomnames.end()) {
        return true;
    }
    return false;
}

std::unordered_map<std::string, HangmanGame*> rooms;

int servFd;
int epollFd;

void ctrl_c(int);

void sendToAllBut(int fd, char * buffer, int count);

uint16_t readPort(char * txt);

void setReuseAddr(int sock);

class HangmanGame{
        string room_name;
        std::string word_to_guess;
        // deskryptor plikow to klucz w mapie
        std::unordered_map<int, Client*> players_in_game;
        // std::vector<Client*> players_in_game;
        Client* host;

        int num_of_players;
        bool is_on;
        int current_round_number;
        int num_of_rounds;
        int max_order_id; // order of players joining game set as the attribute of the player
 
    public:
        HangmanGame(string room){
            this->room_name = room;
            this->num_of_players = 0;
            this->is_on = 0;
            this->current_round_number = 0;
            this->num_of_rounds = 0;
            this->max_order_id = 0;
        }

        ~HangmanGame(){};

        std::string getRoomName(){
            return this->room_name;
        }

        // careful-  increments max_order by default!
        int getOrder(){
            int tmp = this-> max_order_id;
            this->max_order_id++;
            return tmp;
        }


        void addPlayer(int fd, Client* newplayer){
            if (players_in_game.empty()) {
                setHost(newplayer);
            }
            players_in_game.insert(std::pair<int, Client*>(fd, newplayer));
            newplayer->setOrder(getOrder());
            num_of_players++;
        }

        void setHost(Client* newhost){
            this->host = newhost;
            this->host->setAmihost(true);
        }

        void removePlayer(int clientFd){
            // std::shared_ptr<Client> player = players_in_game[clientFd];
            players_in_game.erase(clientFd);
            num_of_players--;
        }

        void showPeopleInGame(){
            std::string s("\n=== People in the room: === \n");
            auto it = players_in_game.begin();
            while(it!=players_in_game.end()){
                Client* player = (it->second);
                it++;
                s.append(player->getUsername());
            }

            sendToAll(s);
        }

        void sendToAll(std::string s){
            auto it = players_in_game.begin();
            while(it!=players_in_game.end()){
                Client* player = (it->second);
                it++;
                player->write(s.c_str(), s.length());
            }
        }

        int getPlayerCount(){
            return this->num_of_players;
        }


        // std::unordered_map<int, std::shared_ptr<Client>> getPlayers(){
        //     return this->players_in_game;
        // }

        // void setHost(std::shared_ptr<Client> newhost){
        //     this->host = newhost;
        // }

        // void addPlayer(int fd, std::shared_ptr<Client> newplayer){
        //     if (players_in_game.empty()) {
        //         setHost(newplayer);
        //     }
        //     players_in_game.insert(std::pair<int, std::shared_ptr<Client>>(fd, newplayer));
        // }

        // void removePlayer(std::shared_ptr<Client> player){
        //     players_in_game.erase(std::remove(players_in_game.begin(),players_in_game.end(), player), players_in_game.end());
        // }

};


class : Handler {
    public:
    virtual void handleEvent(uint32_t events) override {
        if(events & EPOLLIN){
            sockaddr_in clientAddr{};
            socklen_t clientAddrSize = sizeof(clientAddr);
            
            auto clientFd = accept(servFd, (sockaddr*) &clientAddr, &clientAddrSize);
            if(clientFd == -1) error(1, errno, "accept failed");
            
            printf("new connection from: %s:%hu (fd: %d)\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), clientFd);
            
            clients.insert(new Client(clientFd));
        }
        if(events & ~EPOLLIN){
            error(0, errno, "Event %x on server socket", events);
            ctrl_c(SIGINT);
        }
    }
} servHandler;


int main(int argc, char ** argv){
    if(argc != 2) error(1, 0, "Need 1 arg (port)");
    auto port = readPort(argv[1]);
    
    servFd = socket(AF_INET, SOCK_STREAM, 0);
    if(servFd == -1) error(1, errno, "socket failed");
    
    signal(SIGINT, ctrl_c);
    signal(SIGPIPE, SIG_IGN);
    
    setReuseAddr(servFd);
    
    sockaddr_in serverAddr{.sin_family=AF_INET, .sin_port=htons((short)port), .sin_addr={INADDR_ANY}};
    int res = bind(servFd, (sockaddr*) &serverAddr, sizeof(serverAddr));
    if(res) error(1, errno, "bind failed");
    
    res = listen(servFd, 1);
    if(res) error(1, errno, "listen failed");

    epollFd = epoll_create1(0);
    
    epoll_event ee {EPOLLIN, {.ptr=&servHandler}};
    epoll_ctl(epollFd, EPOLL_CTL_ADD, servFd, &ee);
    
    while(true){
        if(-1 == epoll_wait(epollFd, &ee, 1, -1)) {
            error(0,errno,"epoll_wait failed");
            ctrl_c(SIGINT);
        }
        ((Handler*)ee.data.ptr)->handleEvent(ee.events);
    }
}

uint16_t readPort(char * txt){
    char * ptr;
    auto port = strtol(txt, &ptr, 10);
    if(*ptr!=0 || port<1 || (port>((1<<16)-1))) error(1,0,"illegal argument %s", txt);
    return port;
}

void setReuseAddr(int sock){
    const int one = 1;
    int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if(res) error(1,errno, "setsockopt failed");
}

void ctrl_c(int){
    for(Client * client : clients)
        delete client;
    close(servFd);
    printf("Closing server\n");
    exit(0);
}

void sendToAllBut(int fd, char * buffer, int count){
    auto it = clients.begin();
    while(it!=clients.end()){
        Client * client = *it;
        it++;
        if(client->fd()!=fd)
            client->write(buffer, count);
    }
}

//////////////////////////////////////////////////////////////////////////////////////

Client::Client(int fd) : _fd(fd) {

    // players.insert();
    amihost = false;
    PROMPT("Write your username: ");
    epoll_event ee {EPOLLIN|EPOLLRDHUP, {.ptr=this}};
    epoll_ctl(epollFd, EPOLL_CTL_ADD, _fd, &ee);
}
Client::~Client(){
    epoll_ctl(epollFd, EPOLL_CTL_DEL, _fd, nullptr);
    shutdown(_fd, SHUT_RDWR);
    close(_fd);
}

int Client::fd() const {return _fd;}

void Client::handleEvent(uint32_t events) {
    if(events & EPOLLIN) {
        char buf[256];
        ssize_t count = read(_fd, buf, 256);
        if(count > 0){
            string buffer = string(buf).substr(0, count);
            if(gamestate == 0){ // stan podania nazwy uzytkownika
                if (!check_username(buffer)){ 
                    username = buffer;
                    gamestate = 2;
                    cout<< username << "> in gamestep: " << gamestate<<endl;
                    PROMPT("Correct username! \n Join, or create a room:");

                }
                else{
                    PROMPT("Username exists, please write a different username: ");
                }

            }   
            // gamestate 1 - choose between create and join room, only on client side
            // gamestate 2 - check if correct joining/creating
            // gamestate 3 - client in a room with a game - started or not, can quit
            else if(gamestate == 2){ 
                // cout << buffer.length();
                // cout << buffer <<endl;
                string tmp = buffer.substr(0, 4);
                if (buffer.substr(0, 6) == "create") {
                    string roomname = buffer.substr(7);
                    cout << roomname << endl;
                    if (!check_create_roomname(roomname)) {
                        // heap allocated - memory needs to be free'd with 'delete'
                        HangmanGame* gameroom = new HangmanGame(roomname);
                        this->in_roomname = roomname;
                        gameroom->addPlayer(fd(), this);
                        // gameroom->setHost(this); not necessary addPlayer to empty container handles setting the first player as the host
                        order = gameroom->getOrder();
                        rooms[roomname] = gameroom;
                        gamestate = 3;
                        cout<< username << "> in gamestep: " << gamestate<<endl;
                        PROMPT("Correct create roomname");


                        
                    } else {
                        PROMPT("Not correct create roomname");
                        
                    }


                } else if (buffer.substr(0, 4) == "join") {
                    string roomname = buffer.substr(5);
                    cout << roomname << endl;
                    if (check_join_roomname(roomname)) {
                        
                        auto it = rooms.find(roomname);
                        auto &gameroom = it->second;
                        gameroom->addPlayer(fd(), this); // add player at the same time - sets the creator of the room as the host
                        gamestate = 3;
                     
                        in_roomname =roomname;
                        cout << username << "> in gamestep: " << gamestate<<endl;
                        PROMPT("Joined room sucessfully!\n PRESS QUIT: ");
                        
                        
                        // show people in the room
                        gameroom->showPeopleInGame();

                    

                    } else {
                        PROMPT("The room doesnt exist, join or create a different room:");

                    }


                } 

            
            }
            else if(gamestate == 3){

                if (buffer.substr(0, 4) == "quit") {
                    // remove this player from the gameroom, 
                    // change host if the host left 
                    // and check how many players are in the game, if less than 1 player - delete room 

                    if(amihost){
                        cout << username << " - host wants to quit the lobby"<< endl;
                        int playerCount = rooms.find(in_roomname)->second->getPlayerCount();
                        cout << playerCount << endl;
                        if(playerCount <= 1){
                            auto it = rooms.find(in_roomname);
                            cout << it->first;
                            if(it!=rooms.end()){
                                delete it->second;
                                rooms.erase(it);
                                
                            }          
                        }
                    }
                    else{
                        rooms[in_roomname]->removePlayer(fd());
                        rooms[in_roomname]->showPeopleInGame();
                    }

                    gamestate = 2;
                    this->setRoomname("");
                    
                    
                    PROMPT("Join, or create a room:");
                    // clientLeftTheGameInfo();
                } 

            }


        }
        else
            events |= EPOLLERR;
    }
    if(events & ~EPOLLIN){
        // remove player from room here
        remove();
    }
}

bool Client::check_player_joining_game(string buf){
    // buf.append("\0");
    // cout << buf.length();
    if(string("join ").compare(buf.substr(0,5)) == 0){
        // if()
        return true;
        // string substring = buf.substr(5, )
    }

    return false;
}

void Client::write(const char * buffer, int count){
    if(count != ::write(_fd, buffer, count))
        remove();
    
}

void Client::remove() {
    printf("removing %d\n", _fd);

    // usun gracza z gry
    if (in_roomname != ""){
        HangmanGame* &room = rooms[in_roomname];
        room->removePlayer(_fd);
    }
    // usun klienta z listy klientow
    clients.erase(this);

    // usun nazwe klienta
    usernames.erase(std::remove(usernames.begin(), usernames.end(), this->username), usernames.end());
    delete this;
}

void Client::setRoomname(std::string roomname){
    this->in_roomname = roomname;
}

void Client::setOrder(int ord){
    this->order = ord;
}

void Client::setAmihost(bool b){
    this->amihost = b;
}

std::string Client::getRoomname(){
    return this->in_roomname;
}

std::string Client::getUsername(){
    return this->username;
}

void Client::ask_nick(){
    char buffer[256];
    ssize_t count = read(_fd, buffer, 256);
    bool keep_asking = true;
    while(keep_asking){
        char s[] = "Podaj nazwe uzytkownika: ";
        write(s, strlen(s));

        if (!check_username(s)){
            username = s;
            cout << "here";
            keep_asking = false;
        }

        else{
            char nazwa_istnieje[] = "Nazwa juz istnieje";
            write(nazwa_istnieje, strlen(nazwa_istnieje));
        }

    }
}