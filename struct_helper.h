#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>


struct Request {
    std::string method;
    std::string host;
    std::string port;
    std::vector<char> fullmsg;
    std::string line;
};


struct ConnParams {
    int conn_id;
    int client_fd;
    int server_fd;
    Request* requestp;
};


inline void request_init(Request* r, std::string method_in, std::string host_in, std::string port_in, std::vector<char> fullmsg_in, std::string line_in) {
    r->method = method_in;
    r->host = host_in;
    r->port = port_in;
    r->fullmsg = fullmsg_in;
    r->line = line_in;
}

inline void request_print(Request* r) {
    std::cerr << "Method is: " << r->method << std::endl;
    std::cerr << "Host is: " << r->host << std::endl;
    std::cerr << "Port is: " << r->port << std::endl;
}
