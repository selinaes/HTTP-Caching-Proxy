#include <string>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <time.h> 
#include "struct_helper.h"


class Logging {
    private:
        pthread_mutex_t mutex;
        std::string filePath;

    public:
        Logging(std::string filepath, pthread_mutex_t lock) : filePath(filepath), mutex(lock) {}
        void clientRequest(ConnParams *conn); // from client
        void requestServer(ConnParams *conn); // ask server
        void respondToClient(ConnParams *conn, std::string responseLine); // from proxy to client
        void serverRespond(ConnParams *conn); // from server

};


