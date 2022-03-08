//
//  ringmaster.cpp
//  project3_650
//
//  Created by Elaine on 2022/2/19.
//

#include "server.hpp"
#include <string>
#include "potato.hpp"

class Ringmaster:Server{
private:
    const char *port;
    int players_num;
    int hops_num;
    std::vector<int> player_fd;
    std::vector<std::string> player_ip;
    std::vector<int> player_port;
    
public:
    Ringmaster(char *argv1,char *argv2, char *argv3);
    void print_first();
    //struct addrinfo *create_socket(const char *hostname);
    void ring_connect_player();
    void ring_neighbor_player();
    void start_games();
    ~Ringmaster();
};

Ringmaster::Ringmaster(char *argv1, char *argv2, char *argv3):Server(), port(argv1),players_num(atoi(argv2)), hops_num(atoi(argv3)){}

void Ringmaster::print_first(){
    std::cout<<"Potato Ringmaster"<<std::endl;
    std::cout<<"Players = "<<players_num<<std::endl;
    std::cout<<"Hops = "<<hops_num<<std::endl;
}

void Ringmaster::ring_connect_player(){
    int sockfd = create_socket(port);
    socket_info clientInfo;
    //accept the request connection from player
    for(int i=0; i<players_num; i++){
        clientInfo = accept_request(sockfd);
        std::cout<<"Player "<<i<<" is ready to play"<<std::endl;
        send(clientInfo.sockfd, &i, sizeof(i), 0);
        send(clientInfo.sockfd, &players_num, sizeof(players_num), 0);
        player_fd.push_back(clientInfo.sockfd);
        player_ip.push_back(inet_ntoa(clientInfo.addrinfo.sin_addr));
        player_port.push_back(ntohs(clientInfo.addrinfo.sin_port));
    }
    
    //ringmaster send the player neighbor's IP and port number
    for(int i=0; i<players_num; i++){
        if(i==0){
            send(player_fd[i],&player_port[players_num-1],sizeof(player_port[players_num-1]),0);
            send(player_fd[i], player_ip[players_num-1].c_str(), player_ip[players_num-1].length(), 0);
        }
        else{
            send(player_fd[i],&player_port[i-1],sizeof(player_port[i-1]),0);
            send(player_fd[i], player_ip[i-1].c_str(), player_ip[i-1].length(), 0);
        }
    }
}

//start the game, ring give to potato to random player
void Ringmaster::start_games(){
    Potato potato;
    potato.hops = hops_num;
    if(potato.hops!=0){
        srand((unsigned int)time(NULL)+players_num);
        int random_player = rand() % players_num;
        send(player_fd[random_player], &potato, sizeof(potato), 0);
        std::cout << "Ready to start the game, sending potato to player " << random_player << std::endl;
    }
}


Ringmaster::~Ringmaster(){}

int main(int argc, char *argv[]){
    /*if(argc!=4){
        perror("The number of argument in ringmaster is wrong\n");
        exit(EXIT_FAILURE);
    }*/
    if(atoi(argv[2])<2 || atoi(argv[3])<0 || atoi(argv[3])>512){
        perror("The scope of argument is wrong");
        exit(EXIT_FAILURE);
    }
    
    Ringmaster r(argv[1], argv[2], argv[3]);
    r.print_first();
    r.ring_connect_player();
}


/*player.o: player.cpp
        g++ $(CPPFLAGS) -c player.cpp
player: player.o server.o
        g++ -o player player.o server.o

all: ringmaster*/

