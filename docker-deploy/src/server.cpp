#include "server.h"

int Server::createServer()
{
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;
  if ((status = getaddrinfo(hostname, port, &host_info, &host_info_list)) != 0)
  {
    throw Exception("ERROR: getaddrinfo failed, unable to get address information of server.");
  }
  for (p = host_info_list; p != NULL; p = p->ai_next)
  {
    // create socket descriptor
    if ((listen_socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      continue;
    }
    // test whether address already be in use
    if (setsockopt(listen_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
      throw Exception("ERROR: setsockopt failed, socket already be in used.");
    }
    // bind IP address and port to socket
    if (bind(listen_socket_fd, p->ai_addr, p->ai_addrlen) == -1)
    {
      close(listen_socket_fd);
      continue;
    }
    break;
  }
  if (p == NULL)
  {
    throw Exception("ERROR: socket or bind failed, server unable to create socket or bind any IP address.");
  }
  // listen to the port
  if (listen(listen_socket_fd, backlog) == -1)
  {
    throw Exception("ERROR: listen failed, cannot listen to port: ", port);
  }
  if (hostname == NULL)
  {
    std::cout << "Server (ADDR_ANY): Listening on port " << port << ", waitting for client connections..." << std::endl;
  }
  else
  {
    std::cout << "Server (" << getHostAddr() << "): Listening on port " << port << ", waitting for client connections..." << std::endl;
  }
  freeaddrinfo(host_info_list);
  return listen_socket_fd;
}

int Server::acceptConnections(std::string *ip_addr, unsigned short int *port)
{
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connect_socket_fd = accept(listen_socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connect_socket_fd == -1)
  {
    throw Exception("ERROR: accept failed, cannot connect to the client.");
  }
  // return IP address of client
  // only for IPv4, may need change
  //struct sockaddr_in *addr = (struct sockaddr_in *)&socket_addr;
  //*ip_addr = inet_ntoa(addr->sin_addr);
  //*port = addr->sin_port;
  void *addr;
  if(socket_addr.ss_family == AF_INET)
  {
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)&socket_addr;
    addr = &(ipv4->sin_addr);
    *port = ipv4->sin_port;
  }
  else
  {
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&socket_addr;
    addr = &(ipv6->sin6_addr);
    *port = ipv6->sin6_port;
  }
  char ipstr[INET6_ADDRSTRLEN];
  inet_ntop(socket_addr.ss_family, addr, ipstr, sizeof(ipstr));
  *ip_addr = std::string(ipstr);
  std::cout << "Server: get connection from " << *ip_addr << ", port " << *port << std::endl;
  return client_connect_socket_fd;
}

std::string Server::getHostAddr()
{
  char ipstr[INET6_ADDRSTRLEN];
  void *addr;
  std::string ipver;
  if (p->ai_family == AF_INET)
  {
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
    addr = &(ipv4->sin_addr);
    ipver = "IPV4";
  }
  else
  {
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
    addr = &(ipv6->sin6_addr);
    ipver = "IPV6";
  }
  inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
  std::string res;
  res.append(ipver);
  res.append(": ");
  res.append(ipstr);
  return res;
}

const char *Server::getHostName()
{
  return this->hostname;
}

const char *Server::getPortNum()
{
  return this->port;
}
