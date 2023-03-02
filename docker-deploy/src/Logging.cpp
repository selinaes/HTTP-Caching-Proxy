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

void Logging::noteLog(ConnParams *conn, std::string note) {
    pthread_mutex_lock(&mutex);
    std::cout << conn->conn_id << ": NOTE " << note << std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn->conn_id << ": NOTE " <<note << std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::errorLog(ConnParams *conn,std::string error) {
    pthread_mutex_lock(&mutex);
    std::cout << conn->conn_id << ": ERROR " << error << std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn->conn_id << ": ERROR " << error << std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::warningLog(ConnParams *conn,std::string warning) {
    pthread_mutex_lock(&mutex);
    std::cout << conn->conn_id << ": WARNING " << warning << std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn->conn_id << ": WARNING " << warning << std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::insertCacheLog(ConnParams *conn, int situation, std::string reason, std::string expireTime) {
    pthread_mutex_lock(&mutex);
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);

    if (situation == 0) {
        std::cout << conn->conn_id << ": not cacheable because" <<  reason << std::endl;
        logFile << conn->conn_id << ": not cacheable because" <<  reason << std::endl;
    } else if (situation == 1) {
        std::cout << conn->conn_id << ": cached, expires at" <<  expireTime << std::endl;
        logFile << conn->conn_id << ": cached, expires at" <<  expireTime << std::endl;
    } else if (situation == 2) {
        std::cout << conn->conn_id << ": cached, but requires re-validation" << std::endl;
        logFile << conn->conn_id << ": cached, but requires re-validation" << std::endl;
    } 
    
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::retrieveCacheLog(ConnParams *conn, int situation, std::string expiredAt) {
    pthread_mutex_lock(&mutex);
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);

    if (situation == 0) {
        std::cout << conn->conn_id << ": not in cache"  << std::endl;
        logFile << conn->conn_id << ": not in cache"  << std::endl;
    } else if (situation == 1) {
        std::cout << conn->conn_id << ": in cache, but expired at" <<  expiredAt << std::endl;
        logFile << conn->conn_id << ": in cache, but expired at" <<  expiredAt << std::endl;
    } else if (situation == 2) {
        std::cout << conn->conn_id << ": in cache, requires validation" << std::endl;
        logFile << conn->conn_id << ": in cache, requires validation" << std::endl;
    } else if (situation == 3) {
        std::cout << conn->conn_id << ": in cache, valid" << std::endl;
        logFile << conn->conn_id  << ": in cache, valid" << std::endl;
    } 
    
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::tunnelCloseLog(ConnParams *conn) {
    pthread_mutex_lock(&mutex);
    std::cout << conn->conn_id << ": Tunnel closed" << std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn->conn_id << ": Tunnel closed" << std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
}
