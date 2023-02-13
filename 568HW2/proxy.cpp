#include "proxy.h"

void Proxy::startProxy(){
  try{
    Server server(hostname, port);
    int socket_fd = server.buildServer();
    int thread_id = 0;
    while(1){
      std::string ip_addr;
      int connect_fd = server.acceptConnections(&ip_addr);
      Hook * ahook = new Hook(thread_id,connect_fd,ip_addr);
      pthread_t thread;
      pthread_create(&thread, NULL, routeRequest, ahook);
      std::cout<<"New Thread Created: thread_id: "<<thread_id<<", connect_socket_fd: "<<connect_fd<<", ip_address: "<<ip_addr<<std::endl;
      ++thread_id;
    }
  }catch(std::exception e){
    //write to log file
  }
}

void * Proxy::routeRequest(void * ahook){
  Hook * hook_info = (Hook *) ahook;
  int connect_socket_fd = hook_info->getConnectSocketFD();
  char request[65536] = {0};
  int len = recv(connect_socket_fd, request, sizeof(request),0);
  if(len <= 0){
    //write to log file
    return NULL;
  }
  std::string req_msg = std::string(request,len);
  std::cout<<req_msg<<std::endl;
  return NULL;
}

const char * Proxy::getPortNum(){
  return this->port;
}
