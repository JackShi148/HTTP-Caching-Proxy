#include "client.h"

int Client::connectServer()
{
  memset(&server_info, 0, sizeof server_info);
  server_info.ai_family = AF_UNSPEC;
  server_info.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(server_hostname.c_str(), server_port.c_str(), &server_info, &server_info_list)) != 0)
  {
    throw Exception("URL ERROR");
  }
  for (p = server_info_list; p != NULL; p = p->ai_next)
  {
    if ((server_socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      continue;
    }
    if (connect(server_socket_fd, p->ai_addr, p->ai_addrlen) == -1)
    {
      close(server_socket_fd);
      continue;
    }
    break;
  }

  if (p == NULL)
  {
    throw Exception("URL ERROR");
  }
  std::cout << "Client: Connecting to " << this->getServerAddr() << ", port " << server_port << ", waitting for server acception..." << std::endl;
  freeaddrinfo(server_info_list);
  return server_socket_fd;
}

std::string Client::getServerAddr()
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
