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
        void clientRequest(int conn_id, int client_fd, std::string line); // from client
        void requestServer(int conn_id, std::string line, std::string host); // ask server
        void respondToClient(int conn_id, std::string responseLine); // from proxy to client
        void serverRespond(int conn_id, std::string responseLine, std::string host); // from server
        void noteLog(int conn_id, std::string note);
        void errorLog(int conn_id, std::string error);
        void warningLog(int conn_id, std::string warning);
        void insertCacheLog(int conn_id, int situation, std::string reason, std::string expireTime);
        void retrieveCacheLog(int conn_id, int situation, std::string expiredAt);
        void tunnelCloseLog(int conn_id);

};


