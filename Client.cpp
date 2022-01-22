#ifndef CLIENT_CPP
#define CLIENT_CPP
#include "Client.h"

Client::Client(int fd) : _fd(fd) {
    // players.insert();
    char s[] = "Podaj nazwe uzytkownika: ";
    write(s, strlen(s));
    epoll_event ee {EPOLLIN|EPOLLRDHUP, {.ptr=this}};
    epoll_ctl(epollFd, EPOLL_CTL_ADD, _fd, &ee);
}

Client::~Client() {
    epoll_ctl(epollFd, EPOLL_CTL_DEL, _fd, nullptr);
    shutdown(_fd, SHUT_RDWR);
    close(_fd);
};

int Client::fd() const{
    return _fd;
}

void Client::handleEvent(uint32_t events) {
    if(events & EPOLLIN) {
        char buffer[256];
        ssize_t count = read(_fd, buffer, 256);
        if(count > 0){
            
            if(gamestate == 0){ // stan podania nazwy uzytkownika
                    if (!check_username(buffer)){ 
                        username = buffer;
                        cout << username;
                        gamestate++;
                        cout << gamestate << endl;
                    }

                    else{
                        std::string msg("Podano istniejaca nazwe uzytkownika\nPodaj inna nazwe uzytkownika:");
                        write(msg.c_str(), msg.length());
                    }
                }   

            if(gamestate == 1){ // lobby - create a room or join one
                std::string msg("Gamestate 1\n");
                write(msg.c_str(), msg.length());

                std::vector<Client*> v;
                v.push_back(this);
                rooms.emplace(std::pair<std::string,vector<Client*>>(std::string("testroom"), v));
                rooms.emplace(std::pair<std::string,vector<Client*>>(std::string("testroom2"), v));
                showAllRooms();
            
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

void Client::write(const char* buffer, int count){
    if(count != ::write(_fd, buffer, count))
        remove();   
}

void Client::remove() {
    printf("removing %d\n", _fd);
    clients.erase(this);
    delete this;
}

void Client::askNick() {
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

void Client::showAllRooms() {
    std::string msg("Available game rooms: \n");
    for (auto v : rooms){
        msg.append(v.first);
        msg.append(string("\n"));
    }
    write(msg.c_str(), msg.length());
}

#endif