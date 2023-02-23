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
#include <numeric>

#include "cache.h"
#include "server.h"
#include "client.h"
#include "exception.h"
#include "hook.h"
#include "request.hpp"
#include "response.hpp"
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

#define CACHE_CAPACITY 100

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
    void getRequest(int client_connect_socket_fd, int request_server_fd, Request req, void * hook);
    void check502(int client_connect_socket_fd, std::string response, void * hook);
    const char * getPortNum();
    void proxyResponse(int client_connect_socket_fd, std::string exp, void * hook);
    void trySaveResponse(std::string uri, Response response, void * hook);
    std::string getEntireResponse(int request_server_fd);
    // send corresponding message to server and check if 304, and return response
    std::string validateCache(int socket_fd, Request request, Response response, std::string check_type, std::string check_content, void * hook);
    // do the revalidation and get response
    std::string revalidate(int socket_fd, Request request, Response response, void * hook);
    std::string sendNewRequest(int socket_fd, Request request, void * hook);
};

#endif
