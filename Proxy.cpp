#include "Proxy.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <typeinfo>
#include <unordered_map>
#include <pthread.h>


 using namespace std;
 
 pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// helper functions

struct Request {
    std::string method;
    std::string host;
    std::string port;
};

void request_init(Request* r, std::string method_in, std::string host_in, std::string port_in) {
    r->method = method_in;
    r->host = host_in;
    r->port = port_in;
}

void request_print(Request* r) {
    cerr << r->method << ", " << r->host << ", " << r->port << endl;
}

Request request_parse(std::string input) {
   
    string method_in;
    string host_in;
    string port_in;
    
    // find the method 
    auto method_pos = input.find_first_of(" ");
    method_in = input.substr(0, method_pos);

    // find the host:port
    auto host_pos = input.substr(input.find_first_of("Host: ")+6).find_first_of("\r\n");
    string host_port = input.substr(input.find_first_of("Host: ")+6, host_pos);
    
    // find the host
    // find the port
    auto splitter = host_port.find_first_of(":");
    host_in = host_port.substr(input.find_first_of("Host: ")+6, splitter);
    port_in = host_port.substr(splitter+1);
    
    Request r;
    request_init(&r, method_in, host_in, port_in);
    return r;
}



// 1. Need a while loop to listen requests (accept())
// 2. Create a socket after recv the reqeust
void Proxy::runProxy() {
    // Create Proxy's socket, used to loop and accept requests
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results
    int sockfd; // listen on sockfd, new connection on client_fd

    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    
    std::cout << hostname << std::endl << port << std::endl;
    std::cout << typeid(port).name() << endl;
    std::string port_change = std::string(port);
    if ((status = getaddrinfo(NULL, "12345", &hints, &servinfo)) != 0) {
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
        // print the error message
        fprintf(stderr, "bind error: %s\n", gai_strerror(bind_status));
        exit(1);
    }
    
    int listen_status = listen(sockfd, 100); // 100 is the backlog
    if (listen_status == -1) {
        std::cerr << "Listen error when initializing socket";
        exit(1);
    }

    freeaddrinfo(servinfo); // all done with this structure
    
    int connID = 0; // connection id, will increase by 1 for each new request
    while (1) {
        int client_fd = acceptRequest(sockfd);

        ConnParams* conn;
        conn->conn_id = connID;
        conn->client_fd = client_fd;
        conn->server_fd = sockfd; // is this necessary?

        pthread_mutex_lock(&lock);
        connID++;
        pthread_mutex_unlock(&lock);
        
        pthread_t thread;
        pthread_create(&thread, NULL, threadProcess, conn);
    }

    
}

int Proxy::acceptRequest(int proxyfd) {
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t addr_size; // size of the address
    int client_fd; 

    // now accept incoming connections:
    addr_size = sizeof their_addr;
    client_fd = accept(proxyfd, (struct sockaddr *)&their_addr, &addr_size);
    
    
    char request[4096];
    int byte_count;
    // all right! now that we're connected, we can receive some data!
    byte_count = recv(client_fd, request, sizeof(request), 0); // receive request from client
    
    if (byte_count <= 0) {
        std::cerr << "Doesn't receive anything";
    }
    std::string input_str =  std::string(request, byte_count);
    
    // std::cout << byte_count;
    // std::cerr << input_str;
    Request r = request_parse(input_str);
    request_print(&r);
    return 0;
}

void * Proxy::threadProcess(void* params) {
    // 1. Parse the request
    // 2. Create a socket to connect to the server
    // 3. Send the request to the server
    // 4. Receive the response from the server
    // 5. Send the response to the client
    // 6. Close the socket

    // 1. Receive the request from the client
    // 2. Parse the request
    // 3. Build a socket to connect to the host server
    // 4. Handle different types of requests (GET/POST/CONNECT) - with helper functions
    // 5. Close the sockets
    return NULL;
}

void Proxy::handleGET() {
    // 1. Create a socket to connect to the server
    // 2. Send the request to the server
    // 3. Receive the response from the server
    // 4. Send the response to the client
}