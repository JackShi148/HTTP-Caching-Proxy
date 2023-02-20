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
    int client_connect_socket_fd;
    std::string client_ip_addr;
    unsigned short int client_port;
    std::string request_hostname;
    void * proxy;
    void * log;
  
  public:
    Hook(int thread_id,int client_connect_socket_fd,std::string client_ip_addr,unsigned short int client_port,void * proxy,void * log)
    :thread_id(thread_id),client_connect_socket_fd(client_connect_socket_fd)
    ,client_ip_addr(client_ip_addr),client_port(client_port),request_hostname("")
    ,proxy(proxy),log(log){}
    int getThreadID(){
      return this->thread_id;
    }
    int getClientConnectSocketFD(){
      return this->client_connect_socket_fd;
    }
    std::string getClientIPAddr(){
      return this->client_ip_addr;
    }
    unsigned short int getClientPort(){
      return this->client_port;
    }
    void setReqHostName(std::string hostname){
      this->request_hostname = hostname;
    }
    std::string getReqHostName(){
      return this->request_hostname;
    }
    void * getThisObject(){
      return proxy;
    }
    void * getLog(){
      return log;
    }
};

#endif
