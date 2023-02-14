#include "client.h"

int main() {
    Client c = Client("www.google.com", NULL);
    std::cout << "Client has connected with hostname: " << c.hostname << " and port: " << c.port << std::endl; 
    return 0;
}