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

using namespace std;

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

int actAsServer(const char *hostname, const char * port){
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  //const char *hostname = NULL;

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family   = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags    = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  } 

  socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }

  status = listen(socket_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl; 
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(host_info_list);
  return socket_fd;
}

int acceptConnection(int socket_fd, string * ip){
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd;
  client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connection_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    exit(EXIT_FAILURE);
  }
  struct sockaddr_in * addr = (struct sockaddr_in *)&socket_addr;
  *ip = inet_ntoa(addr->sin_addr);
  return client_connection_fd;
}

int main(int argc, char *argv[]){
  int server_fd = actAsServer(argv[1],argv[2]);
  std::string ip;
  int proxy_connection_fd = acceptConnection(server_fd,&ip);
  
  while(1){
    std::cout << "Send Message: ";
    char input[1000];
    std::cin.getline(input,1000);
    if(input[0] == 'q'){
      break;
    }
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
    send(proxy_connection_fd, msg.data(), msg.size(), 0);
    char request[65536] = {0};
    int len;
    while((len = recv(proxy_connection_fd, request, sizeof(request),0)) < 0){}
    std::cout << request << std::endl;
  }
  close(server_fd);
  close(proxy_connection_fd);
  std::cout << "Connection Closed" <<std::endl;
  return 1;
}

