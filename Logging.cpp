#include "Logging.h"
#include <string>
#include <arpa/inet.h>

void Logging::clientRequest(ConnParams *conn){
    // get ip address
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(conn->client_fd, (struct sockaddr *)&addr, &addr_size);
    if (res == -1) {
        perror("getpeername failed");
        return;
    }
    
    std::string ip = inet_ntoa(addr.sin_addr);

    // get current time
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    // outstream to log
    pthread_mutex_lock(&mutex);
    std::cout << conn->conn_id << ": \"" << conn->requestp->line << "\" from " << ip << "@ " << asctime(timeinfo) << std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn->conn_id << ": \"" << conn->requestp->line << "\" from " << ip << "@ " << asctime(timeinfo) << std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::respondToClient(ConnParams *conn, std::string responseLine) {
    pthread_mutex_lock(&mutex);
    std::cout << conn->conn_id << ": Responding \"" << responseLine << "\""<< std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn->conn_id << ": Responding \"" << responseLine << "\"" <<  std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::serverRespond(ConnParams *conn, std::string responseLine) {
    pthread_mutex_lock(&mutex);
    std::cout << conn->conn_id << ": Received \"" << responseLine << "\" from " << conn->requestp->host << std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn->conn_id << ": Received \"" << responseLine << "\" from " << conn->requestp->host << std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::requestServer(ConnParams *conn) {
    pthread_mutex_lock(&mutex);
    std::cout << conn->conn_id << ": Requesting \"" << conn->requestp->line << "\" from " << conn->requestp->host << std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn->conn_id << ": Requesting \"" << conn->requestp->line << "\" from " << conn->requestp->host << std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
    
}