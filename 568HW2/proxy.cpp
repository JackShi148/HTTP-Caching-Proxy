#include "proxy.h"

void Proxy::startProxy(){
  Server server(hostname, port);
  int socket_fd = server.createServer();
  int thread_id = 0;
  while(1){
    std::string client_ip_addr;
    unsigned short int client_port;
    int connect_fd = server.acceptConnections(&client_ip_addr,&client_port);
    Hook * ahook = new Hook(thread_id,connect_fd,client_ip_addr,client_port);
    pthread_t thread;
    pthread_create(&thread, NULL, routeRequest, ahook);
    std::cout<<"New Thread Created: thread_id: "<<thread_id<<", connect_socket_fd: "
    <<connect_fd<<", client_IP_address: "<<client_ip_addr<<", client_port: "<<client_port<<std::endl;
    ++thread_id;
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
  //std::cout<<req_msg<<std::endl;
  /*
  Request r(req_msg);
  std::string method = r.getMethod();
  if(method == "CONNECT"){
    
  }else if(method == "GET"){
    
  }else{
    
  }
  */
  return NULL;
}

void Proxy::connectClient(int connect_fd, int client_fd, int thread_id){
  
}

const char * Proxy::getPortNum(){
  return this->port;
}
