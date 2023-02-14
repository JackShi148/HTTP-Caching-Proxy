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

#define PORT "8080"

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
    void connectClient(int connect_fd, int client_fd, int thread_id);
    const char * getPortNum();
};

#endif
