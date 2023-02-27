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
#include "struct_helper.hpp"


class Logging {
    private:
        pthread_mutex_t mutex;
        std::string filePath;

    public:
        Logging(std::string filepath, pthread_mutex_t lock) : filePath(filepath), mutex(lock) {}
        void clientRequest(ConnParams *conn); // from client
        void requestServer(ConnParams *conn); // ask server
        void respondToClient(ConnParams *conn, std::string responseLine); // from proxy to client
        void serverRespond(ConnParams *conn, std::string responseLine); // from server
        void noteLog(ConnParams *conn, std::string note);
        void errorLog(ConnParams *conn, std::string error);
        void warningLog(ConnParams *conn, std::string warning);
        void insertCacheLog(ConnParams *conn, int situation, std::string reason, std::string expireTime);
        void retrieveCacheLog(ConnParams *conn, int situation, std::string expiredAt);
        void tunnelCloseLog(ConnParams *conn);

};


