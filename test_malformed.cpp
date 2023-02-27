#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <regex>
#include <algorithm>
#include <iterator>


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
    std::cout << "request_line_match.size() = " << request_line_match.size() << std::endl;
    if (request_line_match.size() != 4) {
        std::cout << "Request line has wrong number of capture groups, request is malformed" << std::endl;
        return true; // Request line has wrong number of capture groups, request is malformed
    }

    // Check for incomplete or missing information
    if (request.find("\r\n\r\n") == std::string::npos) {
        std::cout << "Request headers are missing, request is malformed" << std::endl;
        return true; // Request headers are missing, request is malformed
    }
    if (request_line_match[1].str().empty() || request_line_match[2].str().empty() || request_line_match[3].str().empty()) {
        std::cout << "Request method, path, or HTTP version is missing, request is malformed" << std::endl;
        return true; // Request method, path, or HTTP version is missing, request is malformed
    }
    
    // Check for invalid encoding
    std::regex invalid_encoding_regex(R"([\x00-\x08\x0b\x0c\x0e-\x1f])");
    if (std::regex_search(request, invalid_encoding_regex)) {
        std::cout << "Request contains invalid characters, request is malformed" << std::endl;
        return true; // Request contains invalid characters, request is malformed
    }
    std::cout << "Request is valid" << std::endl;
    // If all checks pass, the request is likely valid
    return false;
}


int main() {
    std::string malformed_request = "GE /index.html HTTP/1.1\r\n"
                                    "Host: example.com\r\n"
                                    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n"
                                    "\r\n";
    std::string malformed_request1 = "GET /index.html HTTP/1.1\r\n"
                                    "Host: example.com\r\n"
                                    "\r\n"
                                    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n";
    std::string malformed_request2 = "POST /login.php HTTP/1.1\r\n"
                                    "Host: example.com\r\n"
                                    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n"
                                    "Content-Type: application/x-www-form-urlencoded\r\n"
                                    "\r\n"
                                    "username=john\r\n";
    std::string malformed_request3 = "GET /search?q=%XY%3F HTTP/1.1\r\n"
                                    "Host: example.com\r\n"
                                    "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n";

    std::string valid_request = "GET /index.html HTTP/1.1\r\n"
                                "Host: example.com\r\n"
                                "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3\r\n"
                                "\r\n";



    std::cout << is_malformed_request(malformed_request) << std::endl;
    std::cout << is_malformed_request(malformed_request1) << std::endl;
    std::cout << is_malformed_request(malformed_request2) << std::endl;
    std::cout << is_malformed_request(malformed_request3) << std::endl;
    // std::regex request_line_regex(R"((GET|POST|CONNECT)\s+(.*)\s+(HTTP/\d+\.\d+))");
    // std::smatch request_line_match;
    // if (!std::regex_match(valid_request, request_line_match, request_line_regex)) {
    //     std::cout << "Request line is not in the expected format, request is malformed" << std::endl;
    //     return true; // Request line is not in the expected format, request is malformed
    // }
    std::cout << is_malformed_request(valid_request) << std::endl;

    return 0;
}