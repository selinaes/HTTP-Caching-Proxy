#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>

class Proxy {
    private:
        std::string port; // to pass to getaddrinfo
        std::string hostname; // to pass to getaddrinfo

    public:
        Proxy(std::string port_in, std::string hostname_in) : port(port_in), hostname(hostname_in) {}
        void runProxy();
        static void * threadProcess(void* params);
        int acceptRequest(int proxyfd); // returns client fd
        static int connectToHost(const char* hostname, const char* port); // returns host server fd
        // static void handleResponse(ConnParams* params, Request* request);
        static void handleGET();

};

struct ConnParams {
    int conn_id;
    int client_fd;
    int server_fd;
    // std::string request;
    // std::string response;
};
