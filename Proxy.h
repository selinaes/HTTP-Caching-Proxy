#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "struct_helper.h"



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
        static void handleResponse(ConnParams* params);
        static void handleCONNECT(ConnParams* params);
        static void handlePOST(ConnParams* params);
        static void handleGET(ConnParams* params);

};


