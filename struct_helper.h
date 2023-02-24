#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "Response.h"


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
    Response* responsep;
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

inline int get_body_length(std::vector<char> input_vec) {
    // find the request line
    std::string input(input_vec.begin(), input_vec.end());
    if (input.find("Content-Length: ") != std::string::npos) {
        auto content_length_start = input.find("Content-Length: ");
        auto content_length_string_start = input.substr(content_length_start + 16);
        auto content_length_end_pos = content_length_string_start.find_first_of("\r\n");
        auto content_length_string = content_length_string_start.substr(0, content_length_end_pos);
        int content_length = stoi(content_length_string);
        return content_length;
    }
    else {
        std::cerr << "Content-Length not found" << std::endl;
        return -1;
    }
}


inline int get_header_length(std::vector<char> input_vec) {
    // get header length by finding the first occurence of \r\n\r\n
    std::string input(input_vec.begin(), input_vec.end());
    auto header_end_pos = input.find("\r\n\r\n");
    if (header_end_pos != std::string::npos) {
        // std::cerr << "Response Header: " << input.substr(0, header_end_pos + 4) <<std::endl;
        return header_end_pos + 4;
    }
    else {
        std::cerr << "Header not found" << std::endl;
        return -1;
    }
} 
