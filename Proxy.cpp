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
 int MAX_LENGTH = 65536;

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
    cerr << "Method is: " << r->method << endl;
    cerr << "Host is: " << r->host << endl;
    cerr << "Port is: " << r->port << endl;
}

Request request_parse(std::string input) {
    string method_in;
    string host_in;
    string port_in;
    // find the method 
    auto method_pos = input.find_first_of(" ");
    method_in = input.substr(0, method_pos);
    try {
        // find the host:port
        auto host_port_start_1 = input.find("Host: ");
        auto host_port_start_2 = input.find("host: ");
        auto host_port_start = host_port_start_1 != 0 ? host_port_start_1 : host_port_start_2;

        auto host_port_string_start = input.substr(host_port_start + 6);
        auto host_port_end_pos = host_port_string_start.find_first_of("\r\n");
        auto host_port_combined_string = host_port_string_start.substr(0, host_port_end_pos);

        // find host
        auto delimitter = host_port_combined_string.find_first_of(":");
        port_in = "80";
        if (delimitter != string::npos) {
            port_in = host_port_combined_string.substr(delimitter+1);
        }
        host_in = host_port_combined_string.substr(0, delimitter);
    
        Request r;
        request_init(&r, method_in, host_in, port_in);
        return r;
    }
    catch (exception & e) {
        cerr << "Host missing" << endl;
        host_in = "";
        port_in = "";
        Request r;
        request_init(&r, method_in, host_in, port_in);
        return r;
    }
   
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
    
    
    // std::cout << typeid(port).name() << endl;
    // std::string port_change = std::string(port);
    
    const char * host2 = NULL;
    const char * port2 = "12345";
    // std::cerr << hostname.c_str() << std::endl << port.c_str() << std::endl;
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
    vector<char> request(MAX_LENGTH, 0);
    int byte_count;
    // all right! now that we're connected, we can receive some data!
    // std::cout << request2.size();
    byte_count = recv(conn->client_fd, &request.data()[0], MAX_LENGTH, 0); // receive request from client
    
    if (byte_count <= 0) {
        std::cerr << "Doesn't receive anything";
    }
    std::string input_str =  std::string(request.begin(), request.end());
    std::cout << request.data() << std::endl;

    Request r = request_parse(input_str);
    request_print(&r);

    std::cout << std::endl << std::endl;
    // 2. Build a socket to connect to the host server
    int hostfd = connectToHost(r.host.c_str(), r.port.c_str());
    
    // 3. Handle different types of requests (GET/POST/CONNECT) - with helper functions
    conn->server_fd = hostfd;
    // handleResponse(conn, &r);
    // 4. Close the sockets
    return NULL;
}

int Proxy::connectToHost(const char * hostname, const char * port) {
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results
    int hostfd;

    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // make a socket, bind it, and listen on it:
    hostfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (hostfd == -1) {
        std::cerr << "Create socket error when initializing socket";
        exit(1);
    }
    
    // connect to the host
    int connect_status = connect(hostfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (connect_status == -1) {
        // print the bind status
        fprintf(stderr, "connect error: %s\n", gai_strerror(connect_status));
        exit(1);
    }
    freeaddrinfo(servinfo); // all done with this structure
    return 0;
}

// void Proxy::handleResponse(ConnParams* params, Request* r) {
//     // 1. Exclude methods that are not GET, POST, or CONNECT
//     if (r->method != "GET" && r->method != "POST" && r->method != "CONNECT") {
//         std::cerr << "Method not supported";
//         return;
//     }
//     // 2. Handle GET, POST, and CONNECT
//     else if (r->method == "GET") {
//         handleGET();
//     }
//     return;
// }

void Proxy::handleGET() {
    // 1. Send the request to the server

    // 2. Receive the response from the server
    
    // 3. Send the response to the client
    
}