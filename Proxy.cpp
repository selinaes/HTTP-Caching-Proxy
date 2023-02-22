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
#include <vector>
#include <array>


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
    
    // std::cout << hostname << std::endl << port << std::endl;
    // std::cout << typeid(port).name() << endl;
    // std::string port_change = std::string(port);

    const char * port2 = "12345";
    if ((status = getaddrinfo(NULL, port2, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
    
    // make a socket, bind it, and listen on it:
    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sockfd == -1) {
        std::cerr << "Create socket error when initializing socket";
        exit(1);
    }

    std::cout << servinfo->ai_flags << std::endl << servinfo->ai_addr << std::endl;
    int bind_status = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (bind_status == -1) {
        // print the bind status
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

        pthread_t thread;
        pthread_mutex_lock(&lock);
        ConnParams conn;
        conn.conn_id = connID; // access ID, need lock
        conn.client_fd = client_fd;
        conn.server_fd = sockfd; // is this necessary?

        connID++;
        pthread_mutex_unlock(&lock);
        pthread_create(&thread, NULL, threadProcess, (void*)&conn);

    }

    
}

int Proxy::acceptRequest(int proxyfd) {
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t addr_size; // size of the address
    int client_fd; 

    // now accept incoming connections:
    addr_size = sizeof their_addr;
    client_fd = accept(proxyfd, (struct sockaddr *)&their_addr, &addr_size);
    if (client_fd == -1) {
        std::cerr << "Accept error when initializing socket";
        exit(1);
    }
    return client_fd;
}

void * Proxy::threadProcess(void* params) {
    ConnParams* conn = (ConnParams*) params;
    // 1. Receive the request from the client, and parse it
    vector<char> request2(4096);
    int byte_count;
    // all right! now that we're connected, we can receive some data!
    // std::cout << request2.size();
    byte_count = recv(conn->client_fd, &request2.data()[0], request2.size(), 0); // receive request from client
    std::cout << request2.data() << std::endl;
    if (byte_count <= 0) {
        std::cerr << "Doesn't receive anything";
    }
    std::string input_str =  std::string(request2.begin(), request2.end());
    
    Request r = request_parse(input_str);
    // request_print(&r);
    // 2. Build a socket to connect to the host server

    // 3. Handle different types of requests (GET/POST/CONNECT) - with helper functions
    // 4. Close the sockets
    return NULL;
}

void Proxy::handleGET() {
    // 1. Send the request to the server
    // 2. Receive the response from the server
    // 3. Send the response to the client
}