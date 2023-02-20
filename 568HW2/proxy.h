#ifndef __PROXY_H__
#define __PROXY_H__
#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <ctime>
#include <fstream>
#include <map>
#include <unordered_map>
#include <vector>

#include "server.h"
#include "client.h"
#include "Exception.h"
#include "hook.h"
#include "request.h"
#include "log.h"

#define PORT "8080"
#define MAX_MSGLEN 65536

#define WARNING -1
#define LOGMSG 0
#define NEW_REQUEST 1
#define REQUEST 2
#define RESPOND 3
#define RECEIVE 4

#define CACHE 10
#define CACHE_EXPIREDTIME 11

class Proxy{
  private:
    const char * hostname;
    const char * port;
  public:
    Proxy():hostname(NULL),port(PORT){}
    Proxy(const char * myPort):hostname(NULL),port(myPort){}
    Proxy(const char * myHost,const char * myPort):hostname(myHost),port(myPort){}
    void startProxy();
    static void * routeRequest(void * ahook);
    void connectRequest(int client_connect_socket_fd, int server_fd, void * hook);
    void postRequest(int client_connect_socket_fd, int request_server_fd, Request req, void * hook);
    void check502(int client_connect_socket_fd, std::string response, void * hook);
    const char * getPortNum();
};

#endif
