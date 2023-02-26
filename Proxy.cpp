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
std::string filepath = "/var/log/erss/proxy.log";
std::string filepath1 = "./proxy.log";
Logging logObj = Logging(filepath1, lock);

std::unordered_map <std::string, Response*> cache; // key is url, value is response object


Request request_parse(vector<char> input_in) {
    string method_in;
    string host_in;
    string port_in;
    string url_in;

    // find the request line
    std::string input = std::string(input_in.begin(), input_in.end()); 
    auto line_pos = input.find_first_of("\r\n");
    std::string line_in = input.substr(0, line_pos);
    // std::cerr << "Line: " << line_in << endl;

    // find the method
    auto method_pos = input.find_first_of(" ");
    method_in = input.substr(0, method_pos);

    
    try {
        // find the host:port
        auto host_port_start_1 = input.find("Host: ");
        auto host_port_start_2 = input.find("host: ");
        auto host_port_start = host_port_start_1 != string::npos ? host_port_start_1 : host_port_start_2;

        auto host_port_string_start = input.substr(host_port_start + 6);
        auto host_port_end_pos = host_port_string_start.find_first_of("\r\n");
        auto host_port_combined_string = host_port_string_start.substr(0, host_port_end_pos);

        // find host
        // std::cerr << "Combined port + host: " << host_port_combined_string << endl;
        auto delimitter = host_port_combined_string.find_first_of(":");
        port_in = "80";
        if (delimitter != string::npos) {
            port_in = host_port_combined_string.substr(delimitter+1);
        }
        host_in = host_port_combined_string.substr(0, delimitter);

        // find the address (either relative or absolute) + set url
        string no_method_first_line = line_in.substr(method_pos + 1);
        auto space_pos = no_method_first_line.find_first_of(" ");
        string address = no_method_first_line.substr(0, space_pos);
        
        if (address[0] == '/') {
            // relative address
            std::cerr << "Relative address" << endl;
            url_in = host_in + address;
        } else {
            // absolute address
            std::cerr << "Absolute address" << endl;
            url_in = address;
        }
        std::cerr << "URL: " << url_in << endl;
    
        Request r;
        request_init(&r, method_in, host_in, port_in, input_in, line_in, url_in);
        return r;
    }
    catch (exception & e) {
        cerr << "Host missing" << endl;
        host_in = "";
        port_in = "";
        Request r;
        request_init(&r, method_in, host_in, port_in, input_in, line_in, url_in);
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
    //hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    
    
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

        connID++;
        pthread_mutex_unlock(&lock);
        pthread_create(&thread, NULL, threadProcess, (void*)&conn);
        if (pthread_detach(thread) != 0) {
            std::cerr << "Error when detaching thread" << endl;
            shutdown(sockfd, SHUT_RDWR);
            return;
        }
    }
   shutdown(sockfd, SHUT_RDWR);
   return;
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

    // make a socket, connect to it:
    hostfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (hostfd == -1) {
        std::cerr << "Create socket error when initializing socket";
        exit(1);
    }
    
    // connect to the host
    int connect_status = connect(hostfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (connect_status == -1) {
        // print the connect error
        fprintf(stderr, "connect error: %s\n", gai_strerror(connect_status));
        exit(1);
    }
    freeaddrinfo(servinfo); // all done with this structure
    return hostfd;
}

void * Proxy::threadProcess(void* params) {
    ConnParams* conn = (ConnParams*) params;
    // 1. Receive the request from the client, and parse it
    vector<char> request(MAX_LENGTH, 0);
    int byte_count;
    byte_count = recv(conn->client_fd, &request.data()[0], MAX_LENGTH, 0); // receive request from client
    if (byte_count <= 0) {
        pthread_mutex_lock(&lock);
        std::cerr << "Doesn't receive anything";
        pthread_mutex_unlock(&lock);
        return NULL; // handle?
    }
    // for debugging: request print out
    std::cout << request.data() << std::endl;
    
    // parse request
    Request r = request_parse(request);
    // request_print(&r);

    // add request pointer to conn, so that logObj can use it.
    conn->requestp = &r;
    logObj.clientRequest(conn);

    std::cout << std::endl << std::endl;
    
    // 2. Build a socket to connect to the host server
    // if (r.method == "CONNECT" && r.port != "443") {
    //     std::cerr << "Port number is not 443 for CONNECT request";
    //     return NULL;
    // }
    int hostfd = connectToHost(r.host.c_str(), r.port.c_str());
    
    // 3. Handle different types of requests (GET/POST/CONNECT) - with helper functions
    conn->server_fd = hostfd;
    
    handleResponse(conn, byte_count);
    
    // 4. Close the sockets
    std::cout << "Close the sockets for thread:" << conn->conn_id << std::endl;
    shutdown(conn->client_fd, 2);
    shutdown(hostfd, 2);
    return NULL;
}



void Proxy::handleResponse(ConnParams* params, int cur_pos) {
    // 1. Exclude methods that are not GET, POST, or CONNECT
    if (params->requestp->method != "GET" && params->requestp->method != "POST" && params->requestp->method != "CONNECT") {
        std::cerr << "Method not supported";
        return;
    }
    // 2. Handle GET, POST, and CONNECT
    else if (params->requestp->method == "GET") {
        // if found in cache, see if need to revalidate
        if (cache.find(params->requestp->url) != cache.end()) {
            std::cout << "Found in cache" << std::endl;
            handle_cache(params->requestp->url, params);
        }
        else {
            std::cout << "Not found in cache" << std::endl;
            handleGET(params);
        }
        
    }

    else if (params->requestp->method == "CONNECT") {
        handleCONNECT(params);
    }
    
    else if (params->requestp->method == "POST") {
        handlePOST(params, cur_pos);
    }
    std::cout << "HandleResponse returned:" << params->conn_id << std::endl;
    return;
}

void Proxy::handlePOST(ConnParams* conn, int cur_pos) {
    // 1. Receive the rest of the request from the client
    std::cout << "Sending POST request to server" << std::endl;
    logObj.requestServer(conn);
    // check if chunked, handle differently
    if (checkChunk(conn->requestp->fullmsg)) {
        std::cout << "Chunked POST request" << std::endl;
        handleChunked(conn, conn->requestp->fullmsg, conn->client_fd, conn->server_fd, cur_pos);
    } else {
        std::cout << "Not chunked POST request" << std::endl;
        handleNonChunked(conn, conn->requestp->fullmsg, cur_pos, conn->client_fd, conn->server_fd);
    }
    

    // 2. Receive the first response from the host server, and send it back to the client
    vector<char> response(MAX_LENGTH, 0);
    int byte_first = recv(conn->server_fd, &response.data()[0], MAX_LENGTH, 0); // receive request from client
    // check if received response
    if (byte_first == 0) {
        std::cerr << "No response from server -- POST";
        return;
    }
    std::cerr<< "Response first data: " << response.data() << std::endl;
    send(conn->client_fd, response.data(), byte_first, 0); // send the amount received

    Response resp;
    resp.set_line(response);
    conn->responsep = &resp;
    logObj.serverRespond(conn);
    
    // 3. Receive the rest of the response from the host server, and send it back to the client
    // check if chunked, handle differently
    logObj.respondToClient(conn, conn->responsep->get_line());
    if (checkChunk(response)) {
        std::cout << "Chunked POST response" << std::endl;
        handleChunked(conn, response, conn->server_fd, conn->client_fd, byte_first);
    } else {
        std::cout << "Not chunked POST response" << std::endl;
        handleNonChunked(conn, response, byte_first, conn->server_fd, conn->client_fd);
    }
    
}



void Proxy::handleCONNECT(ConnParams* conn) {

    // 1. send an http response of "200 ok" back to the browser
    
    send(conn->client_fd, "HTTP/1.1 200 OK\r\n\r\n" , 19, 0);
    std::string responseLine = "HTTP/1.1 200 OK";
    logObj.respondToClient(conn, responseLine);
    
    // 2. Use IO multiplexing (i.e. select()) from both ports (client and server), simply forwarding messages from one end to another. 
    
    // numfds is the highest-numbered file descriptor in client_fd and server_fd, plus 1.
    int maxfds = conn->client_fd > conn->server_fd ? conn->client_fd : conn->server_fd;
    fd_set readfds;

    // loop until one of the connections is closed
    while (1) {
        // std::cout << "loop" << std::endl;
        FD_ZERO(&readfds); // clear the set
        FD_SET(conn->server_fd, &readfds); // add server_fd to readfds
        FD_SET(conn->client_fd, &readfds); // add client_fd to readfds

        int select_status = select(maxfds+1, &readfds, NULL, NULL, NULL); // return ready to read file descriptors in readfds; don't care about writefds and exceptfds
        if (select_status == -1) {
            std::cerr << "Select error";
            return;
        }
        vector<int> fds = {conn->client_fd, conn->server_fd};
        
        for (auto fd: fds) {
            if (FD_ISSET(fd, &readfds)) {
                int otherfd;
                if (fd == conn->client_fd) {
                    otherfd = conn->server_fd;
                }
                else {
                    otherfd = conn->client_fd;
                }
                vector <char> buffer(MAX_LENGTH, 0);
                int byte_count = recv(fd, &buffer.data()[0], MAX_LENGTH, 0); // receive request from client
                if (byte_count <= 0) {
                    return;
                }
                int send_status = send(otherfd, &buffer.data()[0], byte_count, 0);
                if (send_status <= 0) {
                    return;
                }
            }
        }
        
        

    }
    

    // 3. The proxy does not send a CONNECT http message on to the origin server.
}

// Return true if revalidate success & can use cache, false if need to replace cache
bool Proxy::revalidate(Response* cached_response, ConnParams* conn) {
    // modify request  
    std::cerr << cached_response->get_etag() << std::endl;
    vector<char> modified_request = cached_response->modify_header_revalidate(conn->requestp->fullmsg);
    // send modified request to server and check if send successful
    int send_status = send(conn->server_fd, modified_request.data(), modified_request.size(), 0);
    if (send_status > 0) {
        std::cerr << "Send successful" << std::endl;
    }

    // receive response from server
    vector<char> response(MAX_LENGTH, 0);
    int byte_first = recv(conn->server_fd, &response.data()[0], MAX_LENGTH, 0); // receive request from client
    // check if received response
    if (byte_first == 0) {
        std::cerr << "No response from server -- revalidate";
        return false;
    }
    // check if 304
    std::string response_str(response.begin(), response.end());
    if (response_str.find("HTTP/1.1 304 Not Modified") != std::string::npos) {
        return true;
    }
    // if 200, update cache
    if (response_str.find("HTTP/1.1 200 OK") != std::string::npos) {
        // check if chunked, handle differently
        if (checkChunk(response)) {
            std::cout << "Chunked revalidate response" << std::endl;
            handleChunked(conn, response, conn->server_fd, conn->client_fd, byte_first);
        } else {
            std::cout << "Not chunked revalidate response" << std::endl;
            handleNonChunked(conn, response, byte_first, conn->server_fd, conn->client_fd);
        }
        conn->responsep->parse_all_attributes(response);
        cache[conn->requestp->url] = conn->responsep;
    }
    return false;
}


void Proxy::retrieve_from_cache(std::string url, ConnParams* conn) {
    // get response from the most updated cache
    Response* response = cache[url];
    // send cached response to client
    vector<char> header = response->get_header();
    vector<char> body = response->get_body();
    // concatenate header and body
    vector<char> fullmsg;
    fullmsg.insert(fullmsg.end(), header.begin(), header.end());
    // ? does it need to do while loop to send all the body?
    int send_status = send(conn->client_fd, fullmsg.data(),fullmsg.size(), 0);
    
}


// if the response is in cache, handle cache
void Proxy::handle_cache(std::string url, ConnParams* conn) {
    Response* response_cached = cache[url];
    if (response_cached->need_revalidation()) {
        if (!revalidate(response_cached, conn)) {
            std::cerr << "Revalidate failed, replaced cache" << std::endl;
            return;
        }
    }       
    // send the response back to the client
    std::cout << "Retrieving from cache" << std::endl;
    retrieve_from_cache(url, conn);
    
    
}


void Proxy::handleChunked(ConnParams* conn, std::vector<char>& message, int recv_fd, int send_fd, int cur_pos) {
    // send first message from from_id to to_id
    // send(send_fd, message.data(), message.size(), 0);
    // logObj.respondToClient(conn, conn->responsep->get_line());

    // while loop to receive the remaing response from the host server
    // vector<char> remain_msg(MAX_LENGTH, 0);
    while (1) {
        if (cur_pos >= message.size()) {
            message.resize(message.size() + MAX_LENGTH);
        }
        int byte_count = recv(recv_fd, &message.data()[cur_pos], MAX_LENGTH, 0); // receive request from client
        std::cerr<< conn->conn_id << "HandleChunk: received byte_count: " << byte_count << std::endl;
        if (byte_count == -1) {
            std::cerr << "Error: recv error" << std::endl;
            return;
        }
        if (byte_count == 0) { // end of chunked response
            break;
        }
        cur_pos += byte_count;
    }
    // send sticked together
    int sent = send(send_fd, message.data(), message.size(), 0);
    std::cout << conn->conn_id << "HandleChunk: sent bytes: " << sent << std::endl << "sent data: " << message.data() << std::endl;
}

void Proxy::handleNonChunked( ConnParams* conn, std::vector<char>& message, int cur_pos, int recv_fd, int send_fd) {
    if (get_body_length(message) == -1 || get_header_length(message) == -1) {
        std::cerr << "Error: invalid response" << std::endl;
        return;
    }
    else {
        int body_length = get_body_length(message);
        int header_length = get_header_length(message);
        int total_length = body_length + header_length;
        message.resize(total_length);
        while (cur_pos < total_length) {
            int byte_count = recv(recv_fd, &message.data()[cur_pos], MAX_LENGTH, 0); // receive request from client           
            if (byte_count < 0) {
                std::perror("Error: recv");
                return;
            }
            std::cerr<< conn->conn_id << "HandleNonChunk: received byte_count: " << byte_count << std::endl;
            cur_pos += byte_count;
            if (cur_pos >= total_length) {
                break;
            }
        }
        // send the full response back to the client
        int sent = send(send_fd, message.data(), message.size(), 0);
        std::cout << conn->conn_id << "HandleNonChunk: seny bytes: " << sent << std::endl;
        // logObj.respondToClient(conn, conn->responsep->get_line());
    }

    // std::cerr<< "Length is: " << length << std::endl;
    std::cerr << "HandleNonChunk Sent Full MSG: " << message.data() << std::endl;
 
    return;
}

bool Proxy::checkChunk(std::vector<char> message) {
    std::string message_str(message.begin(), message.end());
    std::string chunked = "chunked";
    if (message_str.find(chunked) != std::string::npos) {
        return true;
    }
    return false;
}


void Proxy::handleGET(ConnParams* conn) {
    
    // 1. Send the request to the server
    send(conn->server_fd, conn->requestp->fullmsg.data(), conn->requestp->fullmsg.size(), 0);
    logObj.requestServer(conn);
    
    // 2. Receive the response from the server
    vector<char> response(MAX_LENGTH, 0);
    int cur_pos = 0;
    int byte_count = recv(conn->server_fd, &response.data()[0], MAX_LENGTH, 0);
    cur_pos += byte_count;
    // check if received response
    if (byte_count == 0) {
        std::cerr << "No response from server";
        return;
    }
    std::cout << "Initial Response: " << response.data() << std::endl;
    
    Response* resp;
    resp->set_line(response);
    conn->responsep = resp;
    logObj.serverRespond(conn);


    // check if chunked
    if (checkChunk(response)) {
        handleChunked(conn, response, conn->server_fd, conn->client_fd, cur_pos);
    }
    else {
        handleNonChunked(conn, response, cur_pos, conn->server_fd, conn->client_fd);
    }
    resp->parse_all_attributes(response);
    
    // only cache when response is 200 OK
    if (resp->get_line().find("200 OK") != std::string::npos) {
        cache.insert({conn->requestp->url, conn->responsep});
        std::cout << "Cached entry:" << conn->requestp->url << "-------" << conn->responsep->get_line() << std::endl;
    }
    
    

    std::cout << "HandleGet returned:" << conn->conn_id << std::endl;

}