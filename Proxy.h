#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>

class Proxy {
    private:
        const char* port; // to pass to getaddrinfo
        const char* hostname; // to pass to getaddrinfo

    public:
        Proxy(const char* port_in, const char* hostname_in) : port(port_in), hostname(hostname_in) {}
        void runProxy();
        static void * threadProcess(void* params);
        int acceptRequest(int proxyfd);
        void handleGET();

};

struct ConnParams {
    int conn_id;
    int client_fd;
    int server_fd;
    // std::string request;
    // std::string response;
};
