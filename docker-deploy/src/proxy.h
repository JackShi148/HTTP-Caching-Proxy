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

#include "server.h"
#include "client.h"
#include "exception.h"
#include "hook.h"
#include "request.hpp"
#include "log.h"
#include "response.hpp"
#include "cache.h"

#define PORT "12345"
#define MAX_MSGLEN 65536

#define WARNING -1
#define NOTE -2
#define EVICT -3
#define LOGMSG 0
#define NEW_REQUEST 1
#define REQUEST 2
#define RESPOND 3
#define RECEIVE 4

#define CACHE 10
#define CACHE_EXPIREDTIME 11
#define NOT_CACHEABLE 12
#define CACHE_EXPIRES 13

#define CACHE_CAPACITY 100

class Proxy
{
private:
  const char *hostname;
  const char *port;

public:
  Proxy() : hostname(NULL), port(PORT) {}
  Proxy(const char *myPort) : hostname(NULL), port(myPort) {}
  Proxy(const char *myHost, const char *myPort) : hostname(myHost), port(myPort) {}
  void startProxy();
  static void *routeRequest(void *ahook);
  void connectRequest(int client_connect_socket_fd, int server_fd, void *hook);
  void getRequest(int client_connect_socket_fd, int request_server_fd, Request req, void *hook);
  void postRequest(int client_connect_socket_fd, int request_server_fd, Request req, void *hook);
  void proxyResponse(int client_connect_socket_fd, std::string exp, void *hook);
  std::string getEntireResponse(int request_server_fd);
  void trySaveResponse(std::string uri, Response response, int max_stale, void *hook);
  std::string revalidate(int request_server_fd, Request request, Response response, void *hook);
  std::string validateCache(int request_server_fd, Request request, Response response,
                            std::string check_type, std::string check_content, void *hook);
  void printNoteLog(Response res, int max_stale, void *hook);
  const char *getHostName();
  const char *getPortNum();
};

#endif
