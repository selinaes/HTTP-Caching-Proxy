#include "logging.h"
#include <string>
#include <arpa/inet.h>

void logging::receiveRequest(ConnParams *conn){
    // get ip address
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(conn->client_fd, (struct sockaddr *)&addr, &addr_size);
    if (res == -1) {
        perror("getpeername failed");
        exit(1);
    }
    std::string ip = inet_ntoa(addr.sin_addr);

    // get current time
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    // outstream to log
    pthread_mutex_lock(&mutex);
    logFile << conn->conn_id << ': "' << conn->requestp->line << '" from ' << ip << "@ " << asctime(timeinfo) << std::endl;
    pthread_mutex_unlock(&mutex);
}

void logging::sendResponse(ConnParams *conn) {
    
}

void logging::receiveResponse(ConnParams *conn) {
    
}