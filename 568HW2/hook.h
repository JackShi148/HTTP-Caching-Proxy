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
    std::string ip_addr;
  
  public:
    Hook(int thread_id,int connect_socket_fd,std::string ip_addr)
    :thread_id(thread_id),connect_socket_fd(connect_socket_fd),ip_addr(ip_addr){}
    int getThreadID(){
      return this->thread_id;
    }
    int getConnectSocketFD(){
      return this->connect_socket_fd;
    }
    std::string getIPAddr(){
      return this->ip_addr;
    }
};

#endif
