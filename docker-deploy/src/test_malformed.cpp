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

bool is_malformed_response(std::string response) {
    std::string response_line = response.substr(0, response.find("\r\n"));
    std::regex response_line_regex(R"(HTTP/\d+\.\d+\s+(\d+)\s+(.*))");
    std::smatch response_line_match;
    if (!std::regex_match(response_line, response_line_match, response_line_regex)) {
        std::cout << "Response line is not in the expected format, response is malformed" << std::endl;
        return true; // Response line is not in the expected format, response is malformed
    }
    // Check for incomplete or missing information
    // find status code
    std::string status_code = response_line_match[1];


    if (status_code == "200" && response.find("\r\n\r\n") == std::string::npos) {
        std::cout << "Response body/header delimeter missing, response is malformed" << std::endl;
        return true; 
    }

    std::cout << "Response is valid" << std::endl;
    // If all checks pass, the response is likely valid
    return false;
}


int main() {
    std::string valid_response = "HTTP/1.1 200 OK\r\n"
                                    "Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n"
                                    "Server: Apache/2.2.14 (Win32)\r\n"
                                    "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
                                    "Content-Length: 88\r\n"
                                    "Content-Type: text/html\r\n"
                                    "Connection: Closed\r\n\r\n"
                                    "<html>\r\n"
                                    "<body>\r\n"
                                    "<h1>Hello, World!</h1>\r\n"
                                    "</body>\r\n"
                                    "</html>\r\n";
    std::string malformed_response = "HTTP/1.1 OK\r\n"
                                    "Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n"
                                    "Server: Apache/2.2.14 (Win32)\r\n"
                                    "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
                                    "Content-Length: 88\r\n"
                                    "Content-Type: text/html\r\n"
                                    "Connection: Closed\r\n\r\n"
                                    "<html>\r\n"
                                    "<body>\r\n"
                                    "<h1>Hello, World!</h1>\r\n"
                                    "</body>\r\n"
                                    "</html>\r\n";

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
    std::cout << is_malformed_request(valid_request) << std::endl;

    std::cout << is_malformed_response(malformed_response) << std::endl;
    std::cout << is_malformed_response(valid_response) << std::endl;

    return 0;
}