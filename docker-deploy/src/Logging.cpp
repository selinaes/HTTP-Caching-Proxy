#include "Logging.h"
#include <string>
#include <arpa/inet.h>

void Logging::clientRequest(int conn_id, int client_fd, std::string line){
    // get ip address
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(client_fd, (struct sockaddr *)&addr, &addr_size);
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
    std::cout << conn_id << ": \"" << line << "\" from " << ip << "@ " << asctime(timeinfo) << std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn_id << ": \"" << line << "\" from " << ip << "@ " << asctime(timeinfo) << std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::respondToClient(int conn_id, std::string responseLine) {
    pthread_mutex_lock(&mutex);
    std::cout << conn_id << ": Responding \"" << responseLine << "\""<< std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn_id << ": Responding \"" << responseLine << "\"" <<  std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::serverRespond(int conn_id, std::string responseLine, std::string host) {
    pthread_mutex_lock(&mutex);
    std::cout << conn_id << ": Received \"" << responseLine << "\" from " << host << std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn_id << ": Received \"" << responseLine << "\" from " << host << std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::requestServer(int conn_id, std::string line, std::string host) {
    pthread_mutex_lock(&mutex);
    std::cout << conn_id << ": Requesting \"" << line << "\" from " << host << std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn_id << ": Requesting \"" << line << "\" from " << host << std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
    
}

void Logging::noteLog(int conn_id, std::string note) {
    pthread_mutex_lock(&mutex);
    std::cout << conn_id << ": NOTE " << note << std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn_id << ": NOTE " << note << std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::errorLog(int conn_id,std::string error) {
    pthread_mutex_lock(&mutex);
    std::cout << conn_id << ": ERROR " << error << std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn_id << ": ERROR " << error << std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::warningLog(int conn_id,std::string warning) {
    pthread_mutex_lock(&mutex);
    std::cout << conn_id << ": WARNING " << warning << std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn_id << ": WARNING " << warning << std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::insertCacheLog(int conn_id, int situation, std::string reason, std::string expireTime) {
    pthread_mutex_lock(&mutex);
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);

    if (situation == 0) {
        std::cout << conn_id << ": not cacheable because " <<  reason << std::endl;
        logFile << conn_id << ": not cacheable because " <<  reason << std::endl;
    } else if (situation == 1) {
        std::cout << conn_id << ": cached, expires at " <<  expireTime << std::endl;
        logFile << conn_id << ": cached, expires at " <<  expireTime << std::endl;
    } else if (situation == 2) {
        std::cout << conn_id << ": cached, but requires re-validation" << std::endl;
        logFile << conn_id << ": cached, but requires re-validation" << std::endl;
    } 
    
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::retrieveCacheLog(int conn_id, int situation, std::string expiredAt) {
    pthread_mutex_lock(&mutex);
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);

    if (situation == 0) {
        std::cout << conn_id << ": not in cache"  << std::endl;
        logFile << conn_id << ": not in cache"  << std::endl;
    } else if (situation == 1) {
        std::cout << conn_id << ": in cache, but expired at" <<  expiredAt << std::endl;
        logFile << conn_id << ": in cache, but expired at" <<  expiredAt << std::endl;
    } else if (situation == 2) {
        std::cout << conn_id << ": in cache, requires validation" << std::endl;
        logFile << conn_id << ": in cache, requires validation" << std::endl;
    } else if (situation == 3) {
        std::cout << conn_id << ": in cache, valid" << std::endl;
        logFile << conn_id  << ": in cache, valid" << std::endl;
    } 
    
    logFile.close();
    pthread_mutex_unlock(&mutex);
}

void Logging::tunnelCloseLog(int conn_id) {
    pthread_mutex_lock(&mutex);
    std::cout << conn_id << ": Tunnel closed" << std::endl;
    std::ofstream logFile;
    logFile.open(filePath, std::ios_base::app);
    logFile << conn_id << ": Tunnel closed" << std::endl;
    logFile.close();
    pthread_mutex_unlock(&mutex);
}
