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


class logging {
    private:
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        std::ofstream logFile;

    public:
        logging(std::string filepath) : logFile(filepath, std::ios::app) {}
        void receiveRequest(ConnParams *conn);
        void sendResponse(ConnParams *conn);
        void receiveResponse(ConnParams *conn);

};


