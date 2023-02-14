#ifndef __HOOK_H__
#define __HOOK_H__
#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

class Hook {
  private:
    int thread_id;
    int connect_socket_fd;
    std::string client_ip_addr;
    unsigned short int client_port;
  
  public:
    Hook(int thread_id,int connect_socket_fd,std::string client_ip_addr,unsigned short int client_port)
    :thread_id(thread_id),connect_socket_fd(connect_socket_fd),client_ip_addr(client_ip_addr),client_port(client_port){}
    int getThreadID(){
      return this->thread_id;
    }
    int getConnectSocketFD(){
      return this->connect_socket_fd;
    }
    std::string getClientIPAddr(){
      return this->client_ip_addr;
    }
    unsigned short int getClientPort(){
      return this->client_port;
    }
};

#endif
