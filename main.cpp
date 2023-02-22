#include "Proxy.h"
#include <string>



using namespace std;
int main() {
    // const char * hostname = "0.0.0.0";
    // const char * port = "12345";
    std::string hostname = "0.0.0.0";
    std::string port = "12345";
    Proxy * proxy = new Proxy(hostname, port);
    // Proxy proxy = Proxy(hostname, port);
    proxy->runProxy();
    return 1;
}