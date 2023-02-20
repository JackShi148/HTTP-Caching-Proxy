#ifndef __CLIENT_H__
#define __CLIENT_H__
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>

#include "exception.h"

class Client{
  private:
    const char * server_hostname;
    const char * server_port;
    struct addrinfo server_info;
    struct addrinfo * server_info_list;
    struct addrinfo * p;
    int server_socket_fd;
    int status;
  
  public:
    Client(const char * server_hostname, const char * server_port): server_hostname(server_hostname),server_port(server_port){}
    int connectServer();
    std::string getServerAddr();
};

#endif