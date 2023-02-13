#include "server.h"

int Server::buildServer(){
  memset(&host_info,0,sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;
  if((status = getaddrinfo(hostname,port,&host_info,&host_info_list)) != 0){
    throw Exception("ERROR: unable to get address information of server.");
  }
  for(p = host_info_list; p != NULL; p = p->ai_next){
    //create socket descriptor
    if((listen_socket_fd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1){
      continue;
    }
    //test whether address already be in use
    if(setsockopt(listen_socket_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1){
      throw Exception("ERROR: address already be in used.");
    }
    //bind IP address and port to socket
    if(bind(listen_socket_fd,p->ai_addr,p->ai_addrlen) == -1){
      close(listen_socket_fd);
      continue;
    }
    break;
  }
  if(p == NULL){
    throw Exception("ERROR: server unable to bind address.");
  }
  //listen to the port
  if(listen(listen_socket_fd,backlog) == -1){
    throw Exception("ERROR: cannot listen to port: ",port);
  }
  if(hostname == NULL){
    std::cout<<"Server (ADDR_ANY): Listening on port "<<port<<std::endl;
  }else{
    std::cout<<"Server ("<<getHostAddr()<<"): Listening on port "<<port<<std::endl;
  }
  freeaddrinfo(host_info_list);
  return listen_socket_fd;
}

int Server::acceptConnections(std::string *ip_addr){
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int connect_socket_fd = accept(listen_socket_fd,(struct sockaddr *)&socket_addr,&socket_addr_len);
  if(connect_socket_fd == -1){
    throw Exception("ERROR: cannot connect to the client.");
  }
  //return IP address of client
  struct sockaddr_in * addr = (struct sockaddr_in *)&socket_addr;
  //may need change
  *ip_addr = inet_ntoa(addr->sin_addr);
  return connect_socket_fd;
}

std::string Server::getHostAddr(){
  char ipstr[INET6_ADDRSTRLEN];
  void *addr;
  std::string ipver;
  if(p->ai_family == AF_INET){
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
    addr = &(ipv4->sin_addr);
    ipver = "IPV4";
  }else{
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
    addr = &(ipv6->sin6_addr);
    ipver = "IPV6";
  }
  inet_ntop(p->ai_family,addr,ipstr,sizeof(ipstr));
  std::string res;
  res.append(ipver);
  res.append(": ");
  res.append(ipstr);
  return res;
}

const char * Server::getHostName(){
  return this->hostname;
}

const char * Server::getPortNum(){
  return this->port;
}
