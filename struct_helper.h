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
    std::string line;
    std::vector<char> fullmsg;
};

struct ConnParams {
    int conn_id;
    int client_fd;
    int server_fd;
    Request* requestp;
};


void request_init(Request* r, std::string method_in, std::string host_in, std::string port_in, vector<char> fullmsg_in, std::string line_in) {
    r->method = method_in;
    r->host = host_in;
    r->port = port_in;
    r->fullmsg = fullmsg_in;
    r->line = line_in;
}

void request_print(Request* r) {
    cerr << "Method is: " << r->method << endl;
    cerr << "Host is: " << r->host << endl;
    cerr << "Port is: " << r->port << endl;
}
