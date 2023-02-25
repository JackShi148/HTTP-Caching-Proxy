#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>

std::string getHostAddr(struct addrinfo * p){
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

int main(int argc, char *argv[]){
  struct addrinfo host_info;
  struct addrinfo * host_info_list;
  struct addrinfo * p;
  int socket_fd;
  int status;
  if (argc != 3){
    std::cerr<<"ERROR: server hostname and port usage."<<std::endl;
    exit(1);
  }
  memset(&host_info, 0, sizeof host_info);
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  
  if ((status = getaddrinfo(argv[1], argv[2], &host_info, &host_info_list)) != 0) {
    std::cerr<<"ERROR: getaddrinfo failed."<<std::endl;
    exit(1);
  }
  for(p = host_info_list; p != NULL; p = p->ai_next) {
    if ((socket_fd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
      continue;
    }
    if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
      close(socket_fd);
      continue;
    }
    break;
  }

  if (p == NULL) {
    std::cerr<<"ERROR: client failed to connect any server."<<std::endl;
    exit(1);
  }
  std::cout<<"Client: Connecting to "<<getHostAddr(p)<<", port "<<argv[2]<<", waitting for server acception..."<<std::endl;
  freeaddrinfo(host_info_list);
  //send messages
  //const char *messages = "CONNECT www.google.com:443 HTTP/1.1\r\n\r\n";
  //std::string msg(messages);
  //send(socket_fd, msg.c_str(), msg.size()+1, 0);
  
  std::cout << "Enter Request: ";
  char input[1000];
  std::cin.getline(input,1000);
  for(int i = 0; i<1000; i++){
    if(input[i] == '\0'){
      input[i] = '\r';
      input[i+1] = '\n';
      input[i+2] = '\r';
      input[i+3] = '\n';
      break;
    }
  }
  std::string msg(input);
  send(socket_fd, msg.data(), msg.size(), 0);
  
  char req[65536] = {0};
  int len;
  while((len = recv(socket_fd, req, sizeof(req),0)) < 0){}
  std::cout << req << std::endl;
  
  while(1){
    char request[65536] = {0};
    int len;
    while((len = recv(socket_fd, request, sizeof(request),0)) < 0){}
    std::cout << request << std::endl;
    std::cout << "Send Message: ";
    char inp[1000] = {0};
    std::cin.getline(inp,1000);
    if(inp[0] == 'q'){
      break;
    }
    for(int i = 0; i<1000; i++){
      if(inp[i] == '\0'){
        inp[i] = '\r';
        inp[i+1] = '\n';
        inp[i+2] = '\r';
        inp[i+3] = '\n';
        break;
      }
    }
    std::string msgg(inp);
    send(socket_fd, msgg.data(), msgg.size(), 0);
  }
  
  close(socket_fd);
  std::cout << "Connection Closed" <<std::endl;
  
  return socket_fd;
}

