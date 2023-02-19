#include "Proxy.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>



// 1. Need a while loop to listen requests (accept())
// 2. Create a socket after recv the reqeust

void Proxy::runProxy() {
    // Create Proxy's socket, used to loop and accept requests
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results

    struct sockaddr_storage their_addr; // connector's address information
    socklen_t addr_size; // size of the address
    int sockfd; // listen on sockfd, new connection on client_fd
    int client_fd; 

    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    
    if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
    
    // make a socket, bind it, and listen on it:
    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sockfd == -1) {
        std::cerr << "Create socket error when initializing socket";
        exit(1);
    }
    int bind_status = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (bind_status == -1) {
        std::cerr << "Bind error when initializing socket";
        exit(1);
    }
    
    int listen_status = listen(sockfd, 100); // 100 is the backlog
    if (listen_status == -1) {
        std::cerr << "Listen error when initializing socket";
        exit(1);
    }
    // now accept incoming connections:
    addr_size = sizeof their_addr;
    client_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    freeaddrinfo(servinfo);
    
    char request[4096];
    int byte_count;
    // all right! now that we're connected, we can receive some data!
    byte_count = recv(client_fd, request, sizeof(request), 0); // receive request from client
    
    if (byte_count <= 0) {
        std::cerr << "Doesn't receive anything";
    }

    std::cout << byte_count;
    

    
}