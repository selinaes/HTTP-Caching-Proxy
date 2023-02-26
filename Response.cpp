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

void::Response::set_header(std::vector<char> input_vec) {
    std::string header_str = parse_header(input_vec);
    header = std::vector<char>(header_str.begin(), header_str.end());
}

void::Response::set_body(std::vector<char> input_vec) {
    std::string body_str = parse_body(input_vec);
    body = std::vector<char>(body_str.begin(), body_str.end());
}

// true for stale, false for fresh
bool Response::check_stale() {
    // get current time
    time_t now = time(0);
    // check max-age, if max-age freshness not passed, then the response is stale
    if (max_age != -1) {
        if (now - save_time < max_age) {
            return false;
        }
        else {
            return true;
        }
    }
    // if doen't have max-age, then check expires; if expires, then the response is stale
    else {
        if (expires == "") {
            return false;
        }
        else {
            // convert expires to time_t
            struct tm tm;
            strptime(expires.c_str(), "%a, %d %b %Y %H:%M:%S %Z", &tm);
            time_t expires_time = mktime(&tm);
            // compare
            if (expires_time < now) {
                return true;
            }
            else {
                return false;
            }
        }
    }
}



// Used when retrieving the response
bool Response::need_revalidation() {
    // first, check no-cache, because no-cache always revalidate
    if (no_cache) {
        return true;
    }
    // check freshness / stale
    else if (!check_stale()) {
        // if fresh, no need to revalidate
        return true;
    }
    // If stale:
    // check must-revalidate or proxy-revalidate, if exist we need revalidation
    else if (must_revalidate || proxy_revalidate) {
        return true;
    }
    // then, check max-stale, if exist and within max-stale, we don't need revalidation
    else if (max_stale != -1) {
        // get current time
        time_t now = time(0);
        // check max-stale
        if (now - save_time < max_stale) {
            return false;
        }
        else {
            return true;
        }
    }
    return false;

}


std::vector<char> Response::modify_header_revalidate(std::vector<char> message) {
    // if exists etag, then add if-none-match

    if (etag != "") {
        // concatenate two string
        std::string to_add;
        std::string a1 = "If-None-Match: ";
        std::string a2 = etag;
        std::string a3 = "\r\n";
        to_add = a1 + a2 + a3;
        // to_add = "If-None-Match: " + etag + "\r\n";
        // loop to insert to_add to the end of header vector
        for (int i = 0; i < to_add.size(); i++) {
            message.push_back(to_add[i]);
        }
    }
    // if exists last-modified, then add if-modified-since
    if (last_modified != "") {
        std::string to_add1;
        std::string a11 = "If-Modified-Since: ";
        std::string a21 = last_modified;
        std::string a31 = "\r\n";
        to_add1 = a11 + a21 + a31;
        // to_add = "If-Modified-Since: " + last_modified + "\r\n";
        // loop to insert to_add to the end of header vector
        for (int i = 0; i < to_add1.size(); i++) {
            message.push_back(to_add1[i]);
        }
    }
    return message;
}

// Used when getting the response
bool Response::need_cache() {
    if (private_ || no_store) {
        return true;
    }
    else {
        return false;
    }
}

void Response::parse_cache_control() {
    std::string header_str(header.begin(), header.end());
    // find the cache-control line
    if (header_str.find("Cache-Control: ") != std::string::npos) {
        auto cache_control_start = header_str.find("Cache-Control: ");
        auto cache_control_string_start = header_str.substr(cache_control_start + 15);
        auto cache_control_end_pos = cache_control_string_start.find_first_of("\r\n");
        auto cache_control_string = cache_control_string_start.substr(0, cache_control_end_pos);
        // set no_cache
        no_cache = true ? cache_control_string.find("no-cache") != std::string::npos : false;
        // set no_store
        no_store = true ? cache_control_string.find("no-store") != std::string::npos : false;
        // set max_age 
        if (cache_control_string.find("max-age=") != std::string::npos) {
            auto max_age_start = cache_control_string.find("max-age=");
            auto max_age_string_start = cache_control_string.substr(max_age_start + 8);
            auto max_age_end_pos = max_age_string_start.find_first_of(",");
            auto max_age_string = max_age_string_start.substr(0, max_age_end_pos);
            max_age = std::stoi(max_age_string);
        }
        else {
            max_age = -1;
        }
        // set must_revalidate
        must_revalidate = true ? cache_control_string.find("must-revalidate") != std::string::npos : false;
        // set proxy_revalidate
        proxy_revalidate = true ? cache_control_string.find("proxy-revalidate") != std::string::npos : false;

        // set public
        public_ = true ? cache_control_string.find("public") != std::string::npos : false;
        // set private
        private_ = true ? cache_control_string.find("private") != std::string::npos : false;

        // set max_stale
        if (cache_control_string.find("max-stale=") != std::string::npos) {
            auto max_stale_start = cache_control_string.find("max-stale=");
            auto max_stale_string_start = cache_control_string.substr(max_stale_start + 10);
            auto max_stale_end_pos = max_stale_string_start.find_first_of(",");
            auto max_stale_string = max_stale_string_start.substr(0, max_stale_end_pos);
            max_stale = std::stoi(max_stale_string);
        }
        else {
            max_stale = -1;
        }
        // set min_fresh
        // if (cache_control_string.find("min-fresh=") != std::string::npos) {
        //     auto min_fresh_start = cache_control_string.find("min-fresh=");
        //     auto min_fresh_string_start = cache_control_string.substr(min_fresh_start + 10);
        //     auto min_fresh_end_pos = min_fresh_string_start.find_first_of(",");
        //     auto min_fresh_string = min_fresh_string_start.substr(0, min_fresh_end_pos);
        //     min_fresh = std::stoi(min_fresh_string);
        // }
        // else {
        //     min_fresh = -1;
        // }
    }
    else {
        std::cerr << "Cache-Control not found" << std::endl;
        no_cache = false;
    }
}

void Response::parse_expires() {
    std::string header_str(header.begin(), header.end());
    if (header_str.find("Expires: ") != std::string::npos) {
        auto expires_start = header_str.find("Expires: ");
        auto expires_string_start = header_str.substr(expires_start + 9);
        auto expires_end_pos = expires_string_start.find_first_of("\r\n");
        auto expires_string = expires_string_start.substr(0, expires_end_pos);
        expires = expires_string;
    }
    else {
        std::cerr << "Expires not found" << std::endl;
        expires = "";
    }
}

void Response::parse_last_modified() {
    std::string header_str(header.begin(), header.end());
    if (header_str.find("Last-Modified: ") != std::string::npos) {
        auto last_modified_start = header_str.find("Last-Modified: ");
        auto last_modified_string_start = header_str.substr(last_modified_start + 15);
        auto last_modified_end_pos = last_modified_string_start.find_first_of("\r\n");
        auto last_modified_string = last_modified_string_start.substr(0, last_modified_end_pos);
        last_modified = last_modified_string;
    }
    else {
        std::cerr << "Last-Modified not found" << std::endl;
        last_modified = "";
    }
}

void Response::parse_etag() {
    std::string header_str(header.begin(), header.end());
    if (header_str.find("ETag: ") != std::string::npos) {
        auto etag_start = header_str.find("ETag: ");
        auto etag_string_start = header_str.substr(etag_start + 6);
        auto etag_end_pos = etag_string_start.find_first_of("\r\n");
        auto etag_string = etag_string_start.substr(0, etag_end_pos);
        etag = etag_string;
    }
    else {
        std::cerr << "ETag not found" << std::endl;
        etag = "";
    }
}

void Response::parse_time() {
    std::string header_str(header.begin(), header.end());
    if (header_str.find("Date: ") != std::string::npos) {
        auto date_start = header_str.find("Date: ");
        auto date_string_start = header_str.substr(date_start + 6);
        auto date_end_pos = date_string_start.find_first_of("\r\n");
        auto date_string = date_string_start.substr(0, date_end_pos);
        // convert date_string to time_t
        struct tm tm;
        strptime(date_string.c_str(), "%a, %d %b %Y %H:%M:%S %Z", &tm);
        save_time = mktime(&tm);
    }
    else {
        std::cerr << "HTTP Date not found. used now" << std::endl;
        // create a time for now and use it
        save_time = time(0);
    }
}


void Response::parse_all_attributes(std::vector<char> input) {
    set_body(input);
    set_header(input);
    parse_cache_control();
    parse_expires();
    parse_last_modified();
    parse_etag();
    parse_time();
}

std::vector<char> Response::get_body() {
    return body;
}

std::vector<char> Response::get_header() {
    return header;
}






