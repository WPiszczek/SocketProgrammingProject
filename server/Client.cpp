#include "Client.h"
#include "HangmanGame.h"

using namespace HangmanGameNamespace;


Client::Client(int fd) : _fd(fd) {

    amihost = false;
    score = 0;
    in_roomname = "";
    remaining_lives = MAX_LIVES;
    gamestate = 0;
    order = MAX_ORDER+1;
    password_setter = false;

    PROMPT("Gamestate 0");
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
        string buffer = unpackMessage(_fd);
        
        if(gamestate == 0){ // stan podania nazwy uzytkownika
            if (!check_username(buffer)){ 
                username = buffer;
                gamestate = 2;
                cout<< username << "> in gamestep: " << gamestate<<endl;
                // showLobbies();
                PROMPT("Correct username");
            }
            else{
                PROMPT("Not corrrect username");
            }
        }

        // gamestate 1 - choose between create and join room, only on client side
        // gamestate 2 - check if correct joining/creating
        // gamestate 3 - client in a room with a game - started or not, can quit
        // gamestate 4 - the host provided the password, game is on

        else if(gamestate == 2){ 

            string tmp = buffer.substr(0, 4);
            if (buffer.substr(0, 6) == "create") {
                string roomname = buffer.substr(7);
                if (!check_create_roomname(roomname)) {
                    // heap allocated - memory needs to be free'd with 'delete'
                    HangmanGame* gameroom = new HangmanGame(roomname);
                    this->in_roomname = roomname;
                    gameroom->addPlayer(fd(), this);
                    order = gameroom->getOrder();
                    rooms[roomname] = gameroom;
                    gamestate = 3;
                    cout<< username << "> in gamestep: " << gamestate<<endl;
                    PROMPT("Correct create roomname");
                    gameroom->showPeopleInGame();
                    
                } else {
                    PROMPT("Not correct create roomname");                    
                }


            } else if (buffer.substr(0, 4) == "join") {
                std::string roomname = buffer.substr(5);
                cout << username <<"Attempting to join room: "<<roomname << endl;
                if (check_join_roomname(roomname)) {
                    
                    auto it = rooms.find(roomname);
                    auto &gameroom = it->second;
                    gameroom->addPlayer(fd(), this); // add player at the same time - sets the creator of the room as the host
                    
                    
                    in_roomname =roomname;
                    cout << username << "> in gamestep: " << gamestate<<endl;
                    PROMPT("Correct join roomname");

                    gameroom->showPeopleInGame();
                    // joined a game that's already on 
                    if(rooms[in_roomname]->getGameStatus()){
                        gamestate = 4;
                    }

                    // game hasnt started, wait for players
                    else{
                        gamestate = 3;
                    }
                    
                } else {
                    PROMPT("Not correct join roomname");
                    // showLobbies();

                }

            }
        
        }

        // gamestate - players are in a room waiting for the host to start the game by choosing number of rounds and setting a password 
        else if(gamestate == 3){ 

            if (buffer.substr(0, 4) == "quit") {
                // remove this player from the gameroom, 
                // change host if the host left 
                // and check how many players are in the game, if less than 1 player - delete room 

                quit_game();
                PROMPT("Correct quit");
            }

            else if (buffer.substr(0, 5) == "round" && amihost && !password_setter){
                std::string rounds = buffer.substr(6,1); // assuming single digit round number
                int round_number;

                if ( !(rounds.empty()) && isdigit(rounds[0]) )
                {
                    round_number = std::stoi(rounds);
                    cout << "Round num:" << round_number <<" endhere" <<endl;
                    rooms[in_roomname]->setRoundNumber(round_number);
                    PROMPT("Correct round number");
                }
                else{
                    PROMPT("Not correct round number");
                }

            }

            else if(buffer.substr(0, 3) == "set" && amihost){

                if(rooms[in_roomname]->getPlayerCount()>=MIN_PLAYERS_IN_GAME){    
                    password_setter = true;
                    rooms[in_roomname]->setPasswordSetterFd(fd());       
                    prepare_and_set_password(buffer.substr(4));
                }

                else{
                    PROMPT("Wait for more people");
                }

            } 

        }
        //  GUESSING, game is on, host that set the password (password_setter doesn't participate)
        else if(gamestate == 4 && !password_setter){
            if (buffer.substr(0, 6) == "letter"){
                if(remaining_lives == 0){
                    PROMPT("Dead");
                }
                else{
                    rooms[in_roomname]->guess_letter(fd(), tolower(buffer[7]));
                }
            }

            else if(buffer.substr(0, 4) == "quit") {
                quit_game();
            }
            else{
                PROMPT("Not a letter or a quit keyword!\n");
            }

        }
        else {
            events |= EPOLLERR;
        }
            
    }
    if(events & ~EPOLLIN){
        // remove player from room here
        remove();
    }
}

bool Client::check_player_joining_game(string buf){
    if(string("join ").compare(buf.substr(0,5)) == 0){
        return true;
    }

    return false;
}

void Client::write(const char * buffer, int count){
    string s = packMessage(buffer);
    const char * packed_message = s.c_str();

    cout << "PACKED MESSAGE " << packed_message << " " << strlen(packed_message) << endl;
    if(count + 3 != ::write(_fd, packed_message, strlen(packed_message))) {
        remove();
    }        
}


void Client::remove() {
    printf("removing %d\n", _fd);

    // usun gracza z gry
    if (in_roomname != ""){
        // jezeli jest jedynym graczem w pokoju - usun pokoj, inaczej segfault przy removeplayer i setHost (pusty pokoj)
        if(rooms[in_roomname]->getPlayerCount()<=1 ){
            auto it = rooms.find(in_roomname);
            if(it!=rooms.end()){
                delete it->second;
                rooms.erase(it);
            }
        }
        else{
            rooms[in_roomname]->removePlayer(_fd);
        }
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

void Client::setGamestate(int state){
    this->gamestate = state;
}

void Client::setScore(int points){
    this->score = points;
}

void Client::setRemainingLives(int lives){
    this->remaining_lives = lives;
}

std::string Client::getRoomname(){
    return this->in_roomname;
}

std::string Client::getUsername(){
    return this->username;
}

int Client::getOrder(){
    return this->order;
}

bool Client::getAmihost(){
    return this->amihost;
}

int Client::getScore(){
    return this->score;
}

int Client::getRemainingLives(){
    return this->remaining_lives;
}

int Client::getGamestate(){
    return this->gamestate;
}

void Client::quit_game(){
    if(amihost){
        cout << username << " - host wants to quit the lobby"<< endl;
        int playerCount = rooms[in_roomname]->getPlayerCount();
        // cout << playerCount << endl;


        // if hosts leaves and the room is almost empty -> delete the game
        if(playerCount <= 1){
            auto it = rooms.find(in_roomname);
            cout << it->first;
            if(it!=rooms.end()){
                delete it->second;
                rooms.erase(it);
                
            }          
        }

    // else remove the player normally, removePlayer handles changing the host
        else{
            if(password_setter){
                rooms[in_roomname]->setPasswordSetterFd(MAX_ORDER);
            }

            rooms[in_roomname]->removePlayer(fd());
            rooms[in_roomname]->showPeopleInGame();

        }
            amihost = false;
    }  
    else{
        rooms[in_roomname]->removePlayer(fd());
        rooms[in_roomname]->showPeopleInGame();
    }

    gamestate = 2;
    in_roomname = "";
    order = MAX_ORDER;
    password_setter = false;

    score = 0;
    remaining_lives = MAX_LIVES;

}

void Client::setPasswordSetterStatus(bool status){
    this->password_setter = status;
}

void Client::prepare_and_set_password(std::string new_password){
    // transform into lowercase in place
    transform(new_password.begin(), new_password.end(), new_password.begin(), ::tolower);
    HangmanGameNamespace::Password password;
    password.correct_pass = new_password;
    password.underscore = std::string(new_password.size(), '_');
    password.num_of_letters = new_password.length();
    password.guessed_letters_correctly = 0;

    rooms[in_roomname]->setPassword(password);
    PROMPT("Correct set");
    rooms[in_roomname]->setGameStatus(true);

    rooms[in_roomname]->printGame();

}