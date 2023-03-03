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
pthread_mutex_t cache_lock = PTHREAD_MUTEX_INITIALIZER;
int MAX_LENGTH = 65536;
std::string filepath1 = "/var/log/erss/proxy.log";
std::string filepath = "./proxy.log";
Logging logObj = Logging(filepath, lock);

std::unordered_map <std::string, Response> cache; // key is url, value is response object


Request request_parse(ConnParams *conn, vector<char> input_in) {
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
            url_in = host_in + address;
            logObj.noteLog(conn->conn_id, "Relative address: " + url_in);
        } else {
            // absolute address
            url_in = address;
            logObj.noteLog(conn->conn_id, "Absolute address: " + url_in);
        }
    
        Request r;
        request_init(&r, method_in, host_in, port_in, input_in, line_in, url_in);
        return r;
    }
    catch (exception & e) {
        cerr << "Host missing" << endl;
        logObj.errorLog(conn->conn_id, "Host missing");
        host_in = "";
        port_in = "";
        Request r;
        request_init(&r, method_in, host_in, port_in, input_in, line_in, url_in);
        return r;
    }
   
}

bool is_malformed_request(std::string request) {
    // Check for invalid syntax
    // Check for invalid syntax
    // find the first line of the request
    std::string request_line = request.substr(0, request.find("\r\n"));
    std::regex request_line_regex(R"((GET|POST|CONNECT)\s+(.*)\s+(HTTP/\d+\.\d+))");
    std::smatch request_line_match;
    if (!std::regex_match(request_line, request_line_match, request_line_regex)) {
        std::cout << "Request line is not in the expected format, request is malformed" << std::endl;
        return true; // Request line is not in the expected format, request is malformed
    }
    // Check for incomplete or missing information
    // find method
    std::string method = request_line.substr(0, request_line.find(" "));
    if (method == "POST" && request.find("\r\n\r\n") == std::string::npos) {
        std::cout << "Request headers are missing, request is malformed" << std::endl;
        return true; // Request headers are missing, request is malformed
    }

    std::cout << "Request is valid" << std::endl;
    // If all checks pass, the request is likely valid
    return false;
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
        // use perror to print the specific error
        perror(strerror(errno));
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
            logObj.errorLog(conn.conn_id, "Error when detaching thread");
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
        pthread_exit(NULL);
    }
    return client_fd;
}

int Proxy::connectToHost(const char * hostname, const char * port, ConnParams * conn) {
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results
    int hostfd;

    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        logObj.errorLog(conn->conn_id, "getaddrinfo error: " + string(gai_strerror(status)));
        pthread_exit(NULL);
    }

    // make a socket, connect to it:
    hostfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (hostfd == -1) {
        logObj.errorLog(conn->conn_id, "Create socket error when initializing socket");
        pthread_exit(NULL);
    }
    
    // connect to the host
    int connect_status = connect(hostfd, servinfo->ai_addr, servinfo->ai_addrlen);
    if (connect_status == -1) {
        // print the connect error
        logObj.errorLog(conn->conn_id, "connect error: " + string(gai_strerror(connect_status)));
        pthread_exit(NULL);
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
        logObj.errorLog(conn->conn_id, "Doesn't receive anything");
        return NULL; // handle?
    }
    // for debugging: request print out
    std::cout << request.data() << std::endl;
    
    
    // parse request
    Request r = request_parse(conn, request);
    // request_print(&r);

    // add request pointer to conn, so that logObj can use it.
    conn->requestp = &r;
    logObj.clientRequest(conn->conn_id, conn->client_fd, r.line);

    std::cout << std::endl << std::endl;
    
    // 2. Build a socket to connect to the host server
    // if (r.method == "CONNECT" && r.port != "443") {
    //     std::cerr << "Port number is not 443 for CONNECT request";
    //     return NULL;
    // }
    int hostfd = connectToHost(r.host.c_str(), r.port.c_str(), conn);
    
    // 3. Handle different types of requests (GET/POST/CONNECT) - with helper functions
    conn->server_fd = hostfd;
    
    handleResponse(conn, byte_count);
    
    // 4. Close the sockets
    logObj.noteLog(conn->conn_id, "Close the sockets for thread");
    shutdown(conn->client_fd, 2);
    shutdown(hostfd, 2);
    return NULL;
}



void Proxy::handleResponse(ConnParams* params, int cur_pos) {
    // 1. Exclude methods that are not GET, POST, or CONNECT
    vector<char> request_fullmsg = params->requestp->fullmsg;
    std::string request_fullmsg_str(request_fullmsg.begin(), request_fullmsg.end());
    if (is_malformed_request(request_fullmsg_str)) {
        logObj.errorLog(params->conn_id, "Malformed request");
        send(params->client_fd, "HTTP/1.1 400 Bad Request\r\n\r\n", 28, 0);
        std::string responseLine = "HTTP/1.1 400 Bad Request";
        logObj.respondToClient(params->conn_id, responseLine);
        return;
    }

    if (params->requestp->method != "GET" && params->requestp->method != "POST" && params->requestp->method != "CONNECT") {
        logObj.errorLog(params->conn_id, "Method not supported");
        return;
    }
    // 2. Handle GET, POST, and CONNECT
    else if (params->requestp->method == "GET") {
        // lock cache
        pthread_mutex_lock(&cache_lock);
        // if found in cache, handle cache
        if (cache.find(params->requestp->url) != cache.end()) {
            pthread_mutex_unlock(&cache_lock);
            // std::cout << "Response cache get: " << cache[params->requestp->url].get_etag() << std::endl;
            handle_cache(params->requestp->url, params, &cache_lock);
        }
        else {
            pthread_mutex_unlock(&cache_lock);
            logObj.retrieveCacheLog(params->conn_id, 0, ""); // not found in cache
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
    logObj.requestServer(conn->conn_id, conn->requestp->line, conn->requestp->host);
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
        logObj.errorLog(conn->conn_id, "No response from server -- POST");
        return;
    }
    std::cerr<< "Response first data: " << response.data() << std::endl;
    send(conn->client_fd, response.data(), byte_first, 0); // send the amount received

    Response resp;
    resp.set_line(response);
    logObj.serverRespond(conn->conn_id, resp.get_line(), conn->requestp->host);
    
    // 3. Receive the rest of the response from the host server, and send it back to the client
    // check if chunked, handle differently
    logObj.respondToClient(conn->conn_id, resp.get_line());
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
    logObj.respondToClient(conn->conn_id, responseLine);
    
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
            logObj.errorLog(conn->conn_id, "Select error");
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
                    logObj.tunnelCloseLog(conn->conn_id);
                    return;
                }
                int send_status = send(otherfd, &buffer.data()[0], byte_count, 0);
                if (send_status <= 0) {
                    logObj.tunnelCloseLog(conn->conn_id);
                    return;
                }
            }
        }
        
        

    }
    

    // 3. The proxy does not send a CONNECT http message on to the origin server.
}

// Return true if revalidate success & can use cache, false if need to replace cache
bool Proxy::revalidate(Response cached_response, ConnParams* conn) {
    // modify request  
    // logObj.noteLog(conn->conn_id, "Etage: " + cached_response.get_etag());
    vector<char> modified_request = cached_response.modify_header_revalidate(conn->requestp->fullmsg);
    // send modified request to server and check if send successful
    int send_status = send(conn->server_fd, modified_request.data(), modified_request.size(), 0);
    if (send_status > 0) {
        logObj.noteLog(conn->conn_id, "Send Successful");
    }

    // receive response from server
    vector<char> response(MAX_LENGTH, 0);
    int byte_first = recv(conn->server_fd, &response.data()[0], MAX_LENGTH, 0); // receive request from client
    // check if received response
    if (byte_first == 0) {
        logObj.errorLog(conn->conn_id, "No response from server -- revalidate");
        return false;
    }
    // check if 304
    std::string response_str(response.begin(), response.end());
    if (response_str.find("HTTP/1.1 304 Not Modified") != std::string::npos) {
        return true;
    }
    // if 200, update cache
    if (response_str.find("HTTP/1.1 200 OK") != std::string::npos) {
        cached_response.parse_all_attributes(response);
        // check if we need to cache
        if (cached_response.need_cache() != ""){
            logObj.insertCacheLog(conn->conn_id, 0, cached_response.need_cache(), "");
        } else {
            logObj.noteLog(conn->conn_id, "New Content from revalidation. Caching...");
            if (cached_response.log_needRevalidate()){ // cases for no-cache or max-age 0
                logObj.insertCacheLog(conn->conn_id, 2, "", "");
            }
            else if (cached_response.get_expires() != "") { // most other cases, calculte expire from max-age OR expires
                logObj.insertCacheLog(conn->conn_id, 1, "", cached_response.get_expires());
            } else { // very rare cases, like having neither expires nor max-age
                logObj.insertCacheLog(conn->conn_id, 2, "", "");
            }
            // check if chunked, handle differently
            if (checkChunk(response)) {
                std::cout << "Chunked revalidate response" << std::endl;
                handleChunked(conn, response, conn->server_fd, conn->client_fd, byte_first);
            } else {
                std::cout << "Not chunked revalidate response" << std::endl;
                handleNonChunked(conn, response, byte_first, conn->server_fd, conn->client_fd);
            }
            
            cache[conn->requestp->url] = cached_response;
            // logObj.noteLog(conn->conn_id, "Revalidate get etag: " + cache[conn->requestp->url].get_etag());
        }
        
    }
    return false;
}


void Proxy::retrieve_from_cache(std::string url, ConnParams* conn, pthread_mutex_t* cache_lock) {
    // get response from the most updated cache
    pthread_mutex_lock(cache_lock);
    Response response = cache[url];
    pthread_mutex_unlock(cache_lock);
    // send cached response to client
    vector<char> header = response.get_header();
    std::cout << "header: " << header.data() << std::endl;
    vector<char> body = response.get_body();
    std::cout << "body: " << body.data() << std::endl;
    // concatenate header and body
    vector<char> fullmsg;
    fullmsg.insert(fullmsg.end(), header.begin(), header.end());
    // insert "/r/n/r/n" to fullmsg
    std::string delimit = "\r\n\r\n";
    std::vector<char> delimit_v(delimit.begin(), delimit.end());
    fullmsg.insert(fullmsg.end(), delimit_v.begin(), delimit_v.end());
    fullmsg.insert(fullmsg.end(), body.begin(), body.end());
    std::cout << "fullmsg: " << fullmsg.data() << std::endl;
    // ? does it need to do while loop to send all the body?
    int sent_size = send(conn->client_fd, fullmsg.data(),fullmsg.size(), 0);
    std::cout << "sent_size: " << sent_size << std::endl;
}


// if the response is in cache, handle cache
void Proxy::handle_cache(std::string url, ConnParams* conn, pthread_mutex_t* cache_lock) {
    pthread_mutex_lock(cache_lock);
    Response response_cached = cache[url];
    pthread_mutex_unlock(cache_lock);
    // totally expired, no retrieving, directly GET again. Also, delete the cache
    if (response_cached.check_exceed_max_stale()){
        logObj.retrieveCacheLog(conn->conn_id, 1, response_cached.get_expires());
        logObj.warningLog(conn->conn_id, "Cached response exceeded max stale, not usable, delete & re-GET");
        pthread_mutex_lock(cache_lock);
        cache.erase(url);
        pthread_mutex_unlock(cache_lock);
        handleGET(conn); // totally expired, so need to GET again
        return;
    } 
    // See if cache need revalidation, or still fresh
    if (response_cached.need_revalidation()){
        // need validation from the start
        if (response_cached.log_needRevalidate()){
            logObj.retrieveCacheLog(conn->conn_id, 2, "");
        } else {
            logObj.retrieveCacheLog(conn->conn_id, 1, response_cached.get_expires());
        }
        // Go revalidate it
        if (!revalidate(response_cached, conn)) {
            logObj.errorLog(conn->conn_id, "Revalidate failed, replaced cache");
            return;
        }
    } else { // fresh for requester point of view, real fresh + within max-stale
        logObj.retrieveCacheLog(conn->conn_id, 3, "");
    }

    // send the response back to the client
    logObj.noteLog(conn->conn_id, "Retrieving from cache");
    retrieve_from_cache(url, conn, cache_lock);
}



void Proxy::handleChunked(ConnParams* conn, std::vector<char>& message, int recv_fd, int send_fd, int cur_pos) {
    // have one vector that save chunk content for cache
    vector<char> cur_chunk(MAX_LENGTH);
    std::string found_end = "\r\n0\r\n\r\n";
    bool chunk_ended = false;

    // send the first chunk to client
    std::string first_body = parse_body(message);
    int found_end_pos = first_body.find(found_end);
    if (found_end_pos != std::string::npos) {
        chunk_ended = true;
    }
    int sent = send(send_fd, message.data(), cur_pos, 0);
    std::cout << conn->conn_id << " HandleChunk: sent bytes: " << sent << std::endl;
    std::cout << conn->conn_id << " HandleChunk: sent message: " << message.data() << std::endl;
    
    // while loop to receive the remaing response from the host server
    // vector<char> remain_msg(MAX_LENGTH, 0);
     while (!chunk_ended) {
        if (cur_pos >= message.size()) {
            message.resize(message.size() + MAX_LENGTH);
        }
        int byte_count = recv(recv_fd, &cur_chunk.data()[0], MAX_LENGTH, 0); // receive request from client
        std::cout<< conn->conn_id << "HandleChunk: received byte_count: " << byte_count << std::endl;
        std::cout<< conn->conn_id << "Received Chunk message: " << cur_chunk.data() << std::endl;
        
        std::string currChunkstr(cur_chunk.begin(), cur_chunk.end());
          
        if (byte_count == -1) {
            logObj.errorLog(conn->conn_id, "Error: recv error");
            return;
        }
        else if (byte_count == 0) {
            logObj.errorLog(conn->conn_id, "Error: closed connection");
            return;
        }
        if (currChunkstr.find(found_end) != std::string::npos) {
            chunk_ended = true;
        }
       
        // add to message
        message.insert(message.begin()+cur_pos, cur_chunk.begin(), cur_chunk.end());
        cur_pos += byte_count;

        // send current chunk to client
        int sent = send(send_fd, cur_chunk.data(), byte_count, 0);
        std::cout << conn->conn_id << "HandleChunk: sent bytes: " << sent << std::endl;
        std::cout<< conn->conn_id << "HandleChunk sent message: " << cur_chunk.data() << std::endl;
        // reinitialize the cur_chunk

        cur_chunk.clear();
        cur_chunk.resize(MAX_LENGTH);
    }
    
}

void Proxy::handleNonChunked( ConnParams* conn, std::vector<char>& message, int cur_pos, int recv_fd, int send_fd) {
    if (get_body_length(message) == -1 || get_header_length(message) == -1) {
        logObj.errorLog(conn->conn_id, "Error: invalid response");
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
                logObj.errorLog(conn->conn_id, "Error: recv error");
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
        logObj.noteLog(conn->conn_id, "HandleNonChunk: sent bytes: " + std::to_string(sent));
        std::cout << "sent data:" << message.data() << std::endl;
        // logObj.respondToClient(conn, conn->responsep->get_line());
    }

    // std::cerr<< "Length is: " << length << std::endl;
    logObj.respondToClient(conn->client_fd, "HandleNonChunk Sent Full MSG: " + std::to_string(message.size()) + " bytes"); 


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
    logObj.requestServer(conn->conn_id, conn->requestp->line, conn->requestp->host);
    
    // 2. Receive the response from the server
    vector<char> response(MAX_LENGTH, 0);
    int cur_pos = 0;
    int byte_count = recv(conn->server_fd, &response.data()[0], MAX_LENGTH, 0);
    cur_pos += byte_count;
    // check if received response
    if (byte_count == 0) {
        logObj.errorLog(conn->conn_id, "Error: No response from server");
        return;
    }
    std::cout << "Initial Response: " << response.data() << std::endl;
    
    Response resp;
    resp.set_line(response);
    logObj.serverRespond(conn->conn_id, resp.get_line(), conn->requestp->host);


    // check if chunked
    if (checkChunk(response)) {
        handleChunked(conn, response, conn->server_fd, conn->client_fd, cur_pos);
    }
    else {
        handleNonChunked(conn, response, cur_pos, conn->server_fd, conn->client_fd);
    }
    resp.parse_all_attributes(response);
    
    // only cache when response is 200 OK
    if (resp.get_line().find("200 OK") != std::string::npos) {
        if (resp.need_cache() != ""){
            logObj.insertCacheLog(conn->conn_id, 0, resp.need_cache(), "");
        } else {
            if (resp.log_needRevalidate()){ // cases for no-cache or max-age is 0
                logObj.insertCacheLog(conn->conn_id, 2, "", "");
            }
            else if (resp.get_expires() != "") { // most other cases, calculte expire from max-age OR expires
                logObj.insertCacheLog(conn->conn_id, 1, "", resp.get_expires());
            } else { // rare cases, like having neither expires nor max-age
                logObj.insertCacheLog(conn->conn_id, 2, "", "");
            }
            pthread_mutex_lock(&cache_lock);
            cache.insert({conn->requestp->url, resp});
            std::cout << "Initially get: " << cache[conn->requestp->url].get_etag() << std::endl; 
            pthread_mutex_unlock(&cache_lock);     
            std::cout << "Cached entry:" << conn->requestp->url << "-------" << resp.get_line() << std::endl;
        }
        
    }
    
    logObj.noteLog(conn->conn_id, "HandleGet returned");

}