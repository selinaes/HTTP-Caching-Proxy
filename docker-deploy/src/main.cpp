#include "Proxy.h"
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>




using namespace std;
// int main() {
//     const char * hostname = "0.0.0.0";
//     const char * port = "12345";
//     Proxy * proxy = new Proxy(hostname, port);
//     proxy->runProxy();
//     delete proxy;
//     return 1;
// }

int main() {
    const char * hostname = "0.0.0.0";
    const char * port = "12345";

    // Fork first time
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }
    else if (pid > 0) { 
        // Parent process, exit
        printf("Parent process, exit");
        exit(0);
    }

    // Child process, continue
    // Create new session, now a process group leader and session leader -> not have controlling tty
    if (setsid() < 0) {
        perror("setsid failed");
        exit(1);
    }

    // close stdin, stdout, stderr, open them to /dev/null
    // Redirect standard file descriptors to /dev/null
    int fd = open("/dev/null", O_RDWR);
    if (fd == -1) {
        exit(1); // Failed to open /dev/null
    }
    if (dup2(fd, STDIN_FILENO) == -1) {
        exit(1); // Failed to redirect stdin
    }
    if (dup2(fd, STDOUT_FILENO) == -1) {
        exit(1); // Failed to redirect stdout
    }
    if (dup2(fd, STDERR_FILENO) == -1) {
        exit(1); // Failed to redirect stderr
    }
    if (fd > STDERR_FILENO) {
        close(fd);
    }

    // dessociate from controlling terminal
    if (chdir("/") < 0) {
        perror("chdir failed");
        exit(1);
    }

    umask(0); // clear umask

    // Fork second time, not be a session leader
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork failed");
        exit(1);
    }
    else if (pid2 > 0) { 
        // Parent process, exit
        exit(0);
    }
    
    // Do the work
    Proxy * proxy = new Proxy(hostname, port);
    proxy->runProxy();
    delete proxy;
    
    return 0;
}