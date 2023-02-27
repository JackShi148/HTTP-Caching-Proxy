#ifndef __SERVER_H__
#define __SERVER_H__
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

#include "exception.h"

#define PORT "12345"
#define BACKLOG 100

class Server
{
private:
  const char *hostname;
  const char *port;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  struct addrinfo *p;
  int listen_socket_fd;
  int status;
  int yes;
  int backlog;

public:
  Server() : hostname(NULL), port(PORT), yes(1), backlog(BACKLOG) {}
  Server(const char *hostname, const char *port) : hostname(hostname), port(port), yes(1), backlog(BACKLOG) {}
  Server(const char *hostname, const char *port, int backlog) : hostname(hostname), port(port), yes(1), backlog(backlog) {}
  int createServer();
  int acceptConnections(std::string *ip_addr, unsigned short int *port);
  std::string getHostAddr();
  const char *getHostName();
  const char *getPortNum();
};

#endif
