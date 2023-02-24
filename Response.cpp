#include "Response.h"

void printbody(std::vector<char> input_vec, int body_length) {
    std::string input(input_vec.begin(), input_vec.end());
    auto header_end_pos = input.find("\r\n\r\n");
    std::cerr << "Response Body: " << input.substr(header_end_pos+4, body_length) <<std::endl;
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





