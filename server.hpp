//
//  server.hpp
//  project3_650
//
//  Created by Elaine on 2022/2/20.
//

#ifndef server_hpp
#define server_hpp

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
#include <netdb.h>
#include <cstring>
#include <vector>

struct socket_info{
    int sockfd;
    struct sockaddr_in addrinfo;
};

class Server{
protected:
    struct sockaddr_in clientaddr;
    struct addrinfo addr_info, *addr_info_list;
    char *addr_client;
    struct sockaddr_in server_addr;
        
public:
    Server();
    int create_socket(const char *port);
    socket_info accept_request(int socketfd);
    ~Server();
};




#endif /* server_hpp */

