#include "Proxy.h"

int main() {
    const char * hostname = "127.0.0.1";
    const char * port = "12345";
    Proxy * proxy = new Proxy(hostname, port);
    // Proxy proxy = Proxy(hostname, port);
    proxy->runProxy();
    return 1;
}