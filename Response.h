#include <string>
#include <vector>
#include <ctime>
#include <iostream>
#include "parse.hpp"

class Response {
    public:
        // int get_body_length(std::vector<char> input_in);
        // int get_header_length(std::vector<char> input_in);
        std::string get_line();
        void set_line(std::vector<char> input_in);
        void set_header(std::vector<char> input_in);
        void set_body(std::vector<char> input_in);
        std::vector<char> get_header();
        std::vector<char> get_body();
        bool need_cache();
        void parse_cache_control();
        void parse_expires();
        void parse_last_modified();
        void parse_etag();
        void parse_time();
        void parse_all_attributes(std::vector<char> input_in);
        std::vector<char> modify_header_revalidate(std::vector<char> message);
        // called when retrieving cache
        bool check_stale();
        bool need_revalidation();

    private:
        std::vector<char> header; // header
        std::vector<char> body; // body only
        std::string line; // response line
        bool no_cache;
        bool no_store;
        bool must_revalidate;
        bool proxy_revalidate;
        bool private_;
        bool public_;
        int max_age;
        int max_stale;
        // int min_fresh;
        std::string expires;
        std::string last_modified;
        std::string etag; 
        
        // save the time of response
        time_t save_time;
};
