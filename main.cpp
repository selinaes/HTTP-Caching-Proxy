#include "Proxy.h"

int main() {
    const char * hostname = "0.0.0.0";
    const char * port = "12345";
    Proxy * proxy = new Proxy(hostname, "12345");
    // Proxy proxy = Proxy(hostname, port);
    proxy->runProxy();
    return 1;
}