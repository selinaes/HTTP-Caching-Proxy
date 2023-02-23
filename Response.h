#include <string>
#include <vector>
#include <iostream>

class Response {
    public:
        Response();
        Response(std::string length);
        int get_length(std::vector<char> input_in);
        void setLength(std::string length);

    private:
        int length;
    
};
