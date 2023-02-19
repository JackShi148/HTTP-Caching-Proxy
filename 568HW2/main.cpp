#include "client.h"

int main() {
    Client c = Client("67.159.88.176", "8000");
    std::cout << "Client has connected with hostname: " << c.hostname << " and port: " << c.port << std::endl; 
    return 0;
}