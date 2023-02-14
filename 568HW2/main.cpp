#include "client.h"

int main() {
    Client c = Client("127.0.0.1", "123");
    std::cout << "Client has connected with hostname: " << c.hostname << " and port: " << c.port << std::endl; 
    return 0;
}