
# include <vector>
# include <iostream>
# include <string>


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

inline std::string parse_header(std::vector<char> input_vec) {
    // get header length by finding the first occurence of \r\n\r\n
    std::string input(input_vec.begin(), input_vec.end());
    auto header_end_pos = input.find("\r\n\r\n");
    if (header_end_pos != std::string::npos) {
        // std::cerr << "Response Header: " << input.substr(0, header_end_pos + 4) <<std::endl;
        std::string output = input.substr(0, header_end_pos);
        return output;
    }
    else {
        std::cerr << "Header not found" << std::endl;
        return "";
    }
}

inline std::string parse_body(std::vector<char> input_vec) {
    // get header length by finding the first occurence of \r\n\r\n
    std::string input(input_vec.begin(), input_vec.end());
    auto header_end_pos = input.find("\r\n\r\n");
    if (header_end_pos != std::string::npos) {
        // std::cerr << "Response Header: " << input.substr(0, header_end_pos + 4) <<std::endl;
        std::string output = input.substr(header_end_pos + 4);
        return output;
    }
    else {
        std::cerr << "Body not found" << std::endl;
        return "";
    }
}