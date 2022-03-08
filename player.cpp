#include "server.hpp"
#include "potato.hpp"
#include<algorithm>
class Player:Server{
private:
    int player_num;
    int player_id;
    int playerRingFd;
    int playerServer_fd;
    int playerClient_fd;
    int playerServerPlayer_fd;
    char *ring_hostname;
    char *ring_port;
    int neighbor_port;
    char neighbor_ip[100];
    
public:
    Player(char *argv1, char *argv2):Server(), ring_hostname(argv1), ring_port(argv2){memset(&neighbor_ip, 0, sizeof(neighbor_ip));}
    int build_client(const char *server_hostname, const char *server_port);
    void communication_ring();
    uint16_t getPortNum(int playerSerevr_fd);
    void player_client();
    void player_server();
    void playPotato();
    void shutDownPlayerFd();
    ~Player(){}
};

int Player::build_client(const char *server_hostname, const char *server_port){
    struct addrinfo player_info;
    struct addrinfo *player_info_list;
    int status = 0;
    memset(&player_info, 0, sizeof(player_info));
    player_info.ai_family   = AF_UNSPEC;   //ipV4和ipV6
    player_info.ai_socktype = SOCK_STREAM; //TCP
    
    status = getaddrinfo(server_hostname, server_port, &player_info, &player_info_list);
    int player_fd = socket(player_info_list->ai_family,player_info_list->ai_socktype,player_info_list->ai_protocol);
    
    if (player_fd == -1) {
        perror("Fail to create the socket\n");
        exit(EXIT_FAILURE);
    }
    char host[100]="";
    getnameinfo(player_info_list->ai_addr, player_info_list->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
    //player request connection to master
    if(connect(player_fd, player_info_list->ai_addr, player_info_list->ai_addrlen)==-1){
        std::cerr << "Error: cannot connect to socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    return player_fd;
}

void Player::communication_ring(){
    //using this fd to communicate with ring
    playerRingFd = build_client(ring_hostname, ring_port);
    player_id=0;
    //receive the current player's ID
    if(recv(playerRingFd, &player_id, sizeof(player_id),0)==-1){
        perror("Cannot receive the player's id from ring\n");
        exit(EXIT_FAILURE);
    }
    player_num=0;
    //receive the number of players
    if(recv(playerRingFd, &player_num, sizeof(player_num),0)==-1){
        perror("Cannot receive the player's number from ring\n");
        exit(EXIT_FAILURE);
    }
    std::cout<<"Connected as player "<<player_id<<" out of "<<player_num<<" total players"<<std::endl;
    //when the player is the server, send the port number to the master, this port is used for listen
    //1. player as a server  //using this fd for listen
    playerServer_fd = create_socket("65533");
    //2. find the port number when the player is a server
    uint16_t port = getPortNum(playerServer_fd);
    int playerServer_portNumber = ntohs(port);
    //3. send the port number to the ringmaster
    if(send(playerRingFd, &playerServer_portNumber, sizeof(playerServer_portNumber),0)==-1)
    {
        perror("Cannot send the player's port number to the ring\n");
        exit(EXIT_FAILURE);
    }
    //receive the neighbor ID and port from ringmaster
    int resp=recv(playerRingFd, &neighbor_port, sizeof(neighbor_port), 0);
    int ipLen = 0;
    //receive the ip len from ringmaster
    recv(playerRingFd, &ipLen, sizeof(ipLen), 0);
    recv(playerRingFd, neighbor_ip, ipLen+1,0);
   

}

uint16_t Player::getPortNum(int playerServer_fd){
     struct sockaddr_in playerServer_port;
     socklen_t playerServer_portLen = sizeof(playerServer_port);
     if(getsockname(playerServer_fd, (struct sockaddr *)&playerServer_port, &playerServer_portLen)==-1){
         perror("Cannot get the player's port number\n");
         exit(EXIT_FAILURE);
     }
    return playerServer_port.sin_port;
}


void Player::player_client(){
    //the current player is a client, connect with the former player(i-1)
    char convert_neighbor_port[9];
    sprintf(convert_neighbor_port, "%d", neighbor_port);
    
    playerClient_fd = build_client(neighbor_ip,convert_neighbor_port);
}



void Player::player_server(){
    //the current is a server, waiting for the later(i+1) player to connect; accept the request
    socket_info playerServerPlayer= accept_request(playerServer_fd);
    playerServerPlayer_fd = playerServerPlayer.sockfd;
    
}

void Player::playPotato(){
    Potato potato;
    fd_set allfd;
    srand((unsigned int)time(NULL) + player_id);
    int allFd[3]={0,0,0};
    allFd[0] = playerRingFd;
    allFd[1] = playerClient_fd;
    allFd[2] = playerServerPlayer_fd;
    //send the potato to the former one:client
    int ClientFd = (player_id - 1 + player_num) % player_num;
    //send the potato to the latter one:server
    int ServerFd = (player_id + 1) % player_num;
    //find the max fd of these three first argu for select
    int maxfd = *std::max_element(allFd,allFd+3);
    srand((unsigned int)time(NULL) + player_id);
    int resp = 0;
    int result = 0;
    while(1){
        FD_ZERO(&allfd);
        for(int i = 0; i<3; i++){
            FD_SET(allFd[i], &allfd);
        }
    
        result = select(maxfd+1, &allfd, NULL, NULL, NULL);
        if(result==-1){
            perror("Player:The return value of select is wrong\n");
            exit(EXIT_FAILURE);
        }
        for(int i=0; i<3; i++){
            if(FD_ISSET(allFd[i], &allfd)){
                resp = recv(allFd[i], &potato, sizeof(potato), MSG_WAITALL);
                if(resp==-1){
                    perror("Cannot receive the potato from others\n");
                    exit(EXIT_FAILURE);
                }
		    break;
            }
        }
        // if the hops = 0, stop the while loop, do not send this potato to others.
        if(potato.hops==0) break;
        //如何处理
        if(resp == 0){
            FD_CLR(playerRingFd, &allfd);
            break;
            //close(playerRingFd)
        }
        
        //send the potato to other neighbor
        if(potato.hops != 1){
            --potato.hops;
            potato.path[potato.cnt]=player_id;
            ++potato.cnt;
            int random = rand()%2;
            if(random == 0){
                //send the potato to the former one:client
                if(send(playerClient_fd, &potato, sizeof(potato),0)==-1){
                    perror("Cannot send the potato to the former one\n");
                    exit(EXIT_FAILURE);
                }
                std::cout<<"Sending potato to "<<ClientFd<<std::endl;
            }
            else{
                //send the potato to the latter one:server
                if(send(playerServerPlayer_fd, &potato, sizeof(potato),0)==-1){
                    perror("Cannot send the potato to the latter one\n");
                    exit(EXIT_FAILURE);
                }
                std::cout<<"Sending potato to "<<ServerFd<<std::endl;
            }
        }
        else{
            //send the potato to the ringmaster when the hops is 1
            potato.path[potato.cnt]=player_id;
            ++potato.cnt;
            --potato.hops;
            if(send(playerRingFd, &potato, sizeof(potato), 0)==-1){
                perror("Fail to send the potato to the ringmaster\n");
                exit(EXIT_FAILURE);
            }
            std::cout<<"I'm it"<<std::endl;
        }
    }
}

void Player::shutDownPlayerFd(){
    close(playerClient_fd);
    close(playerServerPlayer_fd);
    close(playerRingFd);
}




int main(int argc, char *argv[]){
    if(argc!=3){
        perror("The number of argument in player is wrong\n");
        exit(EXIT_FAILURE);
    }
    Player p(argv[1], argv[2]);
    p.communication_ring();
    p.player_client();  //client
    p.player_server();   //server
    p.playPotato();
    p.shutDownPlayerFd();   
}

