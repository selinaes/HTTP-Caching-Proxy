#include "Proxy.h"
#include <string>



using namespace std;
int main() {
    const char * hostname = "0.0.0.0";
    const char * port = "12345";
    Proxy * proxy = new Proxy(hostname, port);
    proxy->runProxy();
    delete proxy;
    return 1;
}