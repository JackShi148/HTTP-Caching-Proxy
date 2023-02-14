#ifndef __CLIENT_H__
#define __CLIENT_H__
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <cstdlib>

#include "response.hpp"
#include "assert.h"
#include "exception.h"

using namespace std;

class Client {
public:
    int status;
    int socket_fd;
    const char * hostname;
    const char * port;
    struct addrinfo host_info;
    struct addrinfo * host_info_list;

    Client(const char * hostname, const char * port) : status(0), socket_fd(0), hostname(hostname), port(port), host_info_list(NULL) {
        initClient();
    }

    ~Client() {
        if(socket_fd != 0) {
            close(socket_fd);
        }
        if(host_info_list != NULL) {
            free(host_info_list);
        }
    }

    void initClient();
    void clientSend(vector<char> &msg);
    void clientRecv(vector<char> &msg);
};

void validateMsg(int msg_len);
void recvHelper(int socket_fd, vector<char> &msg);
void sendHelper(int socket_fd, vector<char> &msg);


#endif