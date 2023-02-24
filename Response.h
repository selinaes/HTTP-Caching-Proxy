#include <string>
#include <vector>
#include <iostream>

class Response {
    public:
        // int get_body_length(std::vector<char> input_in);
        // int get_header_length(std::vector<char> input_in);
        std::string get_line();
        void set_line(std::vector<char> input_in);

    private:
        std::string line;
    
};
