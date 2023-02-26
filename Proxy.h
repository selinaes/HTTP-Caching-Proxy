#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include "Logging.h"

class Proxy {
    private:
        const char * hostname; // to pass to getaddrinfo
        const char * port; // to pass to getaddrinfo
        
    public:
       
        Proxy(const char * hostname_in, const char * port_in) : hostname(hostname_in), port(port_in) {}
        void runProxy();
        static void * threadProcess(void* params);
        int acceptRequest(int proxyfd); // returns client fd
        static int connectToHost(const char* hostname, const char* port); // returns host server fd
        static void handleResponse(ConnParams* param, int cur_pos);
        static void handleCONNECT(ConnParams* params);
        static void handlePOST(ConnParams* params, int cur_pos);
        static void handleGET(ConnParams* params);
        static bool checkChunk(std::vector<char> message);
        static void handleChunked(ConnParams* params, std::vector<char>& message, int recv_fd, int send_fd);
        static void handleNonChunked(ConnParams* params, std::vector<char>& message, int cur_pos, int recv_fd, int send_fd);

        static bool revalidate(Response* cached_response, ConnParams* conn);
        static void retrieve_from_cache(std::string url, ConnParams* conn);
        static void handle_cache(std::string url, ConnParams* conn);

};


