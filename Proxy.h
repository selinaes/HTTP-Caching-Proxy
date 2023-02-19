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
        void createThread();

};
