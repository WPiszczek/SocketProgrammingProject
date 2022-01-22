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
#include <cstring>

using namespace std;


// std::unordered_map<int, User> users;

constexpr char words[3][20] = {
    "datagram",
    "password",
    "protocol"
};

// struct Client{
//     std::string username;
//     int remaining_lives = 9;
//     int socket;
// } clientss;

struct HangmanGame{
    bool is_on = 0;
    int num_of_players = 0;
    int current_round_number = 0;
    int num_of_rounds = 0;
    char word_to_guess[20];
} game;

struct Player{
    std::string username = "";
    int remaining_lives = 2;

};

std::vector<string> usernames;
bool check_username(std::string name){
    if(std::find(usernames.begin(), usernames.end(), name) != usernames.end()) {
        return true;
    }
    usernames.push_back(name);
    return false;
}


class Client;

int servFd;
int epollFd;


std::unordered_set<Client*> clients;
std::unordered_set<Player*> players;



void ctrl_c(int);

void sendToAllBut(int fd, char * buffer, int count);

uint16_t readPort(char * txt);

void setReuseAddr(int sock);

struct Handler {
    virtual ~Handler(){}
    virtual void handleEvent(uint32_t events) = 0;
};

class Client : public Handler {
    int _fd;
    std::string username;
    int remaining_lives = 2;
    int gamestate = 0;


public:
    Client(int fd) : _fd(fd) {

        // players.insert();
        char s[] = "Podaj nazwe uzytkownika: ";
        write(s, strlen(s));
        epoll_event ee {EPOLLIN|EPOLLRDHUP, {.ptr=this}};
        epoll_ctl(epollFd, EPOLL_CTL_ADD, _fd, &ee);
    }
    virtual ~Client(){
        epoll_ctl(epollFd, EPOLL_CTL_DEL, _fd, nullptr);
        shutdown(_fd, SHUT_RDWR);
        close(_fd);
    }
    int fd() const {return _fd;}


    virtual void handleEvent(uint32_t events) override {
        if(events & EPOLLIN) {
            char buffer[256];
            ssize_t count = read(_fd, buffer, 256);
            if(count > 0){
                // std::cout<<buffer;


            if(gamestate == 0){ // stan podania nazwy uzytkownika
                if (!check_username(buffer)){
                    username = buffer;
                    cout << "here";
                    cout << username;
                    gamestate++;
                }

                else{
                    char s[] = "Podano istniejaca nazwe uzytkownika\nPodaj inna nazwe uzytkownika:";
                    write(s, strlen(s));
                }

            } 



                sendToAllBut(_fd, buffer, count); // chat - wyslij kazdemu co wyslal klient
            }
                
            else
                events |= EPOLLERR;
        }
        if(events & ~EPOLLIN){
            remove();
        }
    }
    void write(char * buffer, int count){
        if(count != ::write(_fd, buffer, count))
            remove();
        
    }

    void remove() {
        printf("removing %d\n", _fd);
        clients.erase(this);
        delete this;
    }

    void ask_nick(){
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
