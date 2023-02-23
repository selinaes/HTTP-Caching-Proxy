#include "response.h"



int Response::get_length(std::vector<char> input_in) {
    // find the request line
    std::string input = std::string(input_in.begin(), input_in.end());
    if (input.find("Content-Length: ") != std::string::npos) {
        auto content_length_start = input.find("Content-Length: ");
        auto content_length_string_start = input.substr(content_length_start + 16);
        auto content_length_end_pos = content_length_string_start.find_first_of("\r\n");
        auto content_length_string = content_length_string_start.substr(0, content_length_end_pos);
        length = stoi(content_length_string);
        return length;
    }
    else {
        std::cerr << "Content-Length not found" << std::endl;
        return 0;
    }
}




