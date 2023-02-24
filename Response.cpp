#include "Response.h"

void printbody(std::vector<char> input_vec, int body_length) {
    std::string input(input_vec.begin(), input_vec.end());
    auto header_end_pos = input.find("\r\n\r\n");
    std::cerr << "Response Body: " << input.substr(header_end_pos+4, body_length) <<std::endl;
}

int Response::get_body_length(std::vector<char> input_vec) {
    // find the request line
    std::string input(input_vec.begin(), input_vec.end());
    if (input.find("Content-Length: ") != std::string::npos) {
        auto content_length_start = input.find("Content-Length: ");
        auto content_length_string_start = input.substr(content_length_start + 16);
        auto content_length_end_pos = content_length_string_start.find_first_of("\r\n");
        auto content_length_string = content_length_string_start.substr(0, content_length_end_pos);
        int content_length = stoi(content_length_string);
        printbody(input_vec, content_length);
        return content_length;
    }
    else {
        std::cerr << "Content-Length not found" << std::endl;
        return -1;
    }
}

std::string Response::get_line(){
    return line;
}

void Response::set_line(std::vector<char> input_vec) {
    std::string input(input_vec.begin(), input_vec.end());
    auto line_end_pos = input.find("\r\n");
    if (line_end_pos != std::string::npos) {
        std::cerr << "Response Line: " << input.substr(0, line_end_pos) <<std::endl;
        line = input.substr(0, line_end_pos);
    }
    else {
        std::cerr << "Line not found" << std::endl;
        line = "";
    } 
}

int Response::get_header_length(std::vector<char> input_vec) {
    // get header length by finding the first occurence of \r\n\r\n
    std::string input(input_vec.begin(), input_vec.end());
    auto header_end_pos = input.find("\r\n\r\n");
    if (header_end_pos != std::string::npos) {
        std::cerr << "Response Header: " << input.substr(0, header_end_pos + 4) <<std::endl;
        return header_end_pos + 4;
    }
    else {
        std::cerr << "Header not found" << std::endl;
        return -1;
    }
} 




