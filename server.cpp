//
//  server.cpp
//  project3_650
//
//  Created by Elaine on 2022/2/20.
//

#include "server.hpp"

Server::Server(){}

int Server::create_socket(const char *port){
    int status = 0;
    const char *hostname = NULL;
    memset(&addr_info, 0, sizeof(addr_info));
    addr_info.ai_family   = AF_UNSPEC;   //ipV4和ipV6
    addr_info.ai_socktype = SOCK_STREAM; //TCP
    addr_info.ai_flags    = AI_PASSIVE;  //inaddr_an
    status = getaddrinfo(hostname, port, &addr_info, &addr_info_list);
    if (strcmp(port, "65533") == 0) {
        struct sockaddr_in * addr_in = (struct sockaddr_in *)(addr_info_list->ai_addr);
        addr_in->sin_port = 0;
    }
    int sockfd = socket(addr_info_list->ai_family,addr_info_list->ai_socktype, addr_info_list->ai_protocol);
    if (sockfd == -1) {
        perror("Fail to create the socket\n");
        exit(EXIT_FAILURE);
    }
    int yes = 1;
    //reuse the port
    status = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(sockfd, addr_info_list->ai_addr, addr_info_list->ai_addrlen);
    if (status == -1) {
        perror("Fail to bind the IP address and port number\n");
        exit(EXIT_FAILURE);
    }

    status = listen(sockfd, 100);
    if (status == -1) {
        perror("Fail to listen\n");
        exit(EXIT_FAILURE);
    }
    if (status != 0) {
        perror("Fail to get address information\n");
        exit(EXIT_FAILURE);
    }
    
    return sockfd;
}

socket_info Server::accept_request(int socketfd){
    struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    int accept_fd = accept(socketfd, (struct sockaddr*)&clientaddr, &addrlen);
    if(accept_fd == -1){
        std::cerr << "Error: cannot accept connection" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    socket_info sockInfo;
    sockInfo.sockfd = accept_fd;
    sockInfo.addrinfo = clientaddr;
    return sockInfo;
}

Server::~Server(){}


