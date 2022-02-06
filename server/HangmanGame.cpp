#include "HangmanGame.h"
#include "Client.h"


HangmanGame::HangmanGame(string room){
    this->room_name = room;
    this->num_of_players = 0;
    this->is_on = false;
    this->current_round_number = 1;
    this->num_of_rounds = 0;
    this->max_order_id = 0;
}

HangmanGame::~HangmanGame(){};

std::string HangmanGame::getRoomName(){
    return this->room_name;
}

int HangmanGame::getPasswordSetterFd(){
    return this-> password_setter_fd;
}

void HangmanGame::setPasswordSetterFd(int clientFd){
    this->password_setter_fd = clientFd;
}

// careful-  increments max_order by default!
int HangmanGame::getOrder(){
    int tmp = max_order_id;
    this->max_order_id++;
    return tmp;
}


void HangmanGame::addPlayer(int clientFd, Client* newplayer){
    if (players_in_game.empty()) {
        setHost(newplayer);
    }
    players_in_game.insert(std::pair<int, Client*>(clientFd, newplayer));
    newplayer->setOrder(getOrder());
    num_of_players++;

    // if the client reconnected to the game -> overwrite his round results
    auto it = leavers_round_results.begin();
    while(it!=leavers_round_results.end()){
        if(it->first == clientFd){
            leavers_round_results.erase(clientFd);
        }
        it++;
    }
}

void HangmanGame::setGameStatus(bool status){
    this->is_on = status;
}

//sends message "Host"
void HangmanGame::setHost(Client* newhost){
    this->host = newhost;
    this->host->setAmihost(true);

    std::string msg("Host");
    this->host->write(msg.c_str(), msg.length());
}


void HangmanGame::setRoundNumber(int rnum){
    this->num_of_rounds = rnum;
}

int HangmanGame::getRoundNumber(){
    return this->num_of_rounds;
}

int HangmanGame::getCurrentRoundNumber(){
    return this->current_round_number;
}

void HangmanGame::removePlayer(int clientFd){
    if(players_in_game[clientFd]->getAmihost()){
        int min_order = MAX_ORDER-1;
        Client* host_candidate;
        auto it = players_in_game.begin();
        while(it!=players_in_game.end()){
            if(it->first != clientFd){
                Client* player = (it->second);
                int tmp = player->getOrder();
                if (tmp < min_order){
                    host_candidate = it->second;
                    min_order = player->getOrder();                    
                }
            }            
            it++;            
        }
        setHost(host_candidate);
    }

    // save player's round result
    if(is_on && (clientFd != password_setter_fd)){
        std::string s; 
        s.append(players_in_game[clientFd]->getUsername());
        s.append(std::string(";score;"));
        s.append(to_string(players_in_game[clientFd]->getScore()));
        s.append(";lives;");
        s.append(to_string(players_in_game[clientFd]->getRemainingLives()));
        s.append(";disconnected;");
        leavers_round_results.insert(std::pair<int, std::string>(clientFd, s));

    }
   
    players_in_game.erase(clientFd);
    num_of_players--;

    // jezeli gra sie toczy, a w pokoju pozostal tylko host i jeden gracz - ustaw graczom gamestate poczekalni
    if(host->getGamestate() == 4 && num_of_players <= 1){
        is_on = false;
        
        auto it2 = players_in_game.begin();
        while(it2!=players_in_game.end()){   
            it2->second->setGamestate(3);      
            it2++;
        }

        sendToAll(std::string("Gamestate 3"));
    }  
}

void HangmanGame::showPeopleInGame(){
    std::string s("PeopleInTheRoom;");
    auto it = players_in_game.begin();
    while(it!=players_in_game.end()){
        Client* player = (it->second);
        it++;
        s.append(player->getUsername());
        s.append(";");
    }
    cout << s << endl;
    sendToAll(s);
}

void HangmanGame::sendToAll(std::string s){
    auto it = players_in_game.begin();
    while(it!=players_in_game.end()){
        Client* player = (it->second);
        it++;
        player->write(s.c_str(), s.length());
    }
}

void HangmanGame::sendTo(std::string s, int clientFd){
    auto it = players_in_game.begin();
    while(it!=players_in_game.end()){
        Client* player = (it->second);
        it++;
        if(player->fd()==clientFd){
            player->write(s.c_str(), s.length());
        }       
    }
}

int HangmanGame::getPlayerCount(){
    return this->num_of_players;
}

bool HangmanGame::getGameStatus(){
    return this->is_on;
}

void HangmanGame::setPassword(Password new_password){
    this->password = new_password;

    std::string s("Gamestate 4");
    auto it = players_in_game.begin();
    while(it!=players_in_game.end()){
        Client* player = (it->second);
        player->setGamestate(4);
        player->write(s.c_str(), s.length());
        it++;
    }
}

void HangmanGame::printGame(){
    std::string s("Scores;Round;");
    s.append(to_string(current_round_number));
    s.append(";of;");
    s.append(to_string(num_of_rounds));
    s.append(";");
    auto it = players_in_game.begin();
    while(it!=players_in_game.end()){
        if(it->first == password_setter_fd){
            s.append(it->second->getUsername());
            s.append(";host;;;;");
        }
        else{
            Client* player = (it->second);
            s.append(player->getUsername());
            s.append(std::string(";score;"));
            s.append(to_string(player->getScore()));
            s.append(";lives;");
            s.append(to_string(player->getRemainingLives()));
            s.append(";");
        }
        it++;          
    }

    s.append("Password;");
    s.append(password.underscore);
    s.append(";");
    cout << "GAME " << s << endl;
    sendToAll(s);

}

void HangmanGame::setNewHostAfterGame(){
    if(password_setter_fd != MAX_ORDER){
        int setter_order = players_in_game[password_setter_fd]->getOrder();
        int min_order = MAX_ORDER-1;

        Client* host_candidate;
        auto it = players_in_game.begin();
        while(it!=players_in_game.end()){
            int player_order = it->second->getOrder();
            if(player_order > setter_order && player_order < min_order){
                min_order = player_order;
                host_candidate = it->second;
            }
            
            it++;
            
        }

        if (min_order == MAX_ORDER-1){
            auto it2 = players_in_game.begin();
            while(it2!=players_in_game.end()){
                if(it2->first != password_setter_fd){
                    Client* player = (it2->second);
                    int tmp = player->getOrder();
                    if (tmp < min_order){
                        host_candidate = it2->second;
                        min_order = player->getOrder();
                        
                    }
                }
                
                it2++;
                
            }
        }
        players_in_game[password_setter_fd]->setPasswordSetterStatus(false);
        setHost(host_candidate);
        
    }
}


void HangmanGame::guess_letter(int clientFd, char letter){
    int cnt = 0;
    for(int i=0; i < password.num_of_letters ;i++){
        if(password.correct_pass[i] == letter){              
            password.underscore[i] = password.correct_pass[i];
            cnt++;             
        }
    }
    bool letter_is_already_guessed = (password.guessed_letters.find(letter) != std::string::npos);
    if(!letter_is_already_guessed){
        password.guessed_letters.push_back(letter);
        password.guessed_letters_correctly = password.guessed_letters_correctly+cnt;
    }
    else{
        cnt = 0;
    }
    
    if(cnt == 0){
            int lives = players_in_game[clientFd]->getRemainingLives() - 1;
            players_in_game[clientFd]->setRemainingLives(lives);
    }
    else if(cnt > 0){
        int score_before_guess = players_in_game[clientFd]->getScore();
        int score_after_guess = score_before_guess + cnt;
        players_in_game[clientFd]->setScore(score_after_guess);
    }

    int lives_sum = 0;
    auto it = players_in_game.begin();
    while(it!=players_in_game.end()){
        if(it->first != password_setter_fd){
            int tmp = it->second->getRemainingLives();
            lives_sum += tmp;
        }
        it++;          
    }
    cout << "Curr round number: " << current_round_number << " last round:" << num_of_rounds << endl;
    cout << "Guessed correctly atm: " << password.guessed_letters_correctly << " total: "<< password.num_of_letters<<endl;
    
    if(lives_sum==0 || password.guessed_letters_correctly == password.num_of_letters){
        endRound();
    }
    else {
        printGame();
    }
}

void HangmanGame::endRound() {
    if(current_round_number<num_of_rounds){
        is_on = false;
        sendToAll(std::string("Round over"));
        sendToAll(roundResults());
        cleanAfterRound();
        current_round_number++;
    }
    else{
        is_on = false; // pause game until the host sets a new password
        sendToAll(std::string("Game over"));
        setNewHostAfterGame();
        sendToAll(roundResults());
        cleanAfterRound();
        current_round_number = 1;
    }
}


std::string HangmanGame::roundResults(){
    std::string s("Results;Round;");
    s.append(to_string(current_round_number));
    s.append(";of;");
    s.append(to_string(num_of_rounds));
    s.append(";");
    auto it = players_in_game.begin();
    while(it!=players_in_game.end()){
        if(it->first == password_setter_fd){
            s.append(it->second->getUsername());
            s.append(";host;;;;;");
        }
        else{
            Client* player = (it->second);
            s.append(player->getUsername());
            s.append(std::string(";score;"));
            s.append(to_string(player->getScore()));
            s.append(";lives;");
            s.append(to_string(player->getRemainingLives()));
            s.append(";;"); 

        }

        it++;          
    }

    s.append("Password;");
    s.append(password.correct_pass);
    s.append(";");

    auto it2 = leavers_round_results.begin();
    while(it2!=leavers_round_results.end()){
        s.append(it2->second);
        it2++;          
    }
    
    game_results.insert(std::pair<int, std::string>(current_round_number, s));

    return s;

}

void HangmanGame::showGameResults(){
    std::string result("");
    auto it = game_results.begin();
    while(it!=game_results.end()){
        result.append(it->second);
        it++;          
    }
    sendToAll(result);
}


void HangmanGame::cleanAfterRound(){
    auto it = players_in_game.begin();
    while(it!=players_in_game.end()){
        Client* player = (it->second);
        player->setGamestate(3);
        player->setScore(0);
        player->setRemainingLives(MAX_LIVES);
        it++;          
    }

    // erase leaver list after the round
    auto it2 = leavers_round_results.begin();
    while (it2!= leavers_round_results.end()){
        leavers_round_results.erase(it2->first);
        it2++;
    }
}