#include "proxy.h"

void Proxy::startProxy(){
  Server server(hostname, port);
  int socket_fd = server.createServer();
  int thread_id = 0;
  Log log;
  log.openLogFile("./proxy.log");
  while(1){
    std::string client_ip_addr;
    unsigned short int client_port;
    //proxy accepts connections from clients
    int client_connect_socket_fd = server.acceptConnections(&client_ip_addr,&client_port);
    Hook * ahook = new Hook(thread_id, client_connect_socket_fd, client_ip_addr, client_port, this, &log);
    pthread_t thread;
    pthread_create(&thread, NULL, routeRequest, ahook);
    std::cout<<"New Thread Created: thread_id: "<<thread_id<<", client_connect_socket_fd: "
    <<client_connect_socket_fd<<", client_IP_address: "<<client_ip_addr<<", client_port: "<<client_port<<std::endl;
    ++thread_id;
  }
}

void * Proxy::routeRequest(void * ahook){
  Hook * hook_info = (Hook *) ahook;
  int client_connect_socket_fd = hook_info->getClientConnectSocketFD();
  Proxy * p = (Proxy *) hook_info->getThisObject();
  Log * log = (Log *) hook_info->getLog();
  char request[MAX_MSGLEN] = {0};
  int len = recv(client_connect_socket_fd, request, sizeof(request),0);
  if(len <= 0){
    log->writeLogFile(hook_info, "Invalid Request", WARNING);
    return NULL;
  }
  std::string req_msg = std::string(request,len);

  
  Request r(req_msg);
  std::string method = r.getMethod();
  std::string request_hostname = r.getHostName();
  std::cout << request_hostname << std::endl;
  std::string request_port = r.getPort();
  std::cout << request_port << std::endl;
  
  hook_info->setReqHostName(r.getHostName());
  
  if(r.validate()) {
    log->writeLogFile(hook_info, r.getRequestLine(), NEW_REQUEST);
  }
  else {
    send(client_connect_socket_fd, "HTTP/1.1 400 Bad Request\r\n\r\n", 28, 0);
    std::string errMsg = "HTTP/1.1 400 Bad Request";
    log->writeLogFile(hook_info, errMsg, RESPOND);
  }
  std::cout << r.getRequestLine() << std::endl;
  Client client(request_hostname, request_port);
  int request_server_fd = client.connectServer();
  
  std::cout << "Client("<< hook_info->getClientIPAddr() << ") requests a " << method 
  <<" to Server("<< request_hostname << ", " << request_server_fd << ")" << std::endl;
  
  if(method == "CONNECT") {
    log->writeLogFile(hook_info, r.getRequestLine(), REQUEST);
    p->connectRequest(client_connect_socket_fd, request_server_fd, ahook);
    log->writeLogFile(hook_info, "Tunnel closed", LOGMSG);
  }
  else if(method == "GET") {
    //write cache
  }
  else {
    log->writeLogFile(hook_info, r.getRequestLine(), REQUEST);
    p->postRequest(client_connect_socket_fd, request_server_fd, r, ahook);
  }
  
  close(request_server_fd);
  close(client_connect_socket_fd);
  
  return NULL;
}

void Proxy::connectRequest(int client_connect_socket_fd, int request_server_fd, void * hook){
  Hook * h = (Hook *) hook;
  Log * log = (Log *) h->getLog();
  send(client_connect_socket_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
  log->writeLogFile(h, "HTTP/1.1 200 OK", RESPOND);
  fd_set readset;
  int maxfd = client_connect_socket_fd > request_server_fd? client_connect_socket_fd : request_server_fd;
  while(1)
  {
    FD_ZERO(&readset);
    FD_SET(request_server_fd,&readset);
    FD_SET(client_connect_socket_fd,&readset);
    select(maxfd + 1, &readset, NULL, NULL, NULL);
    int fds[2] = {request_server_fd, client_connect_socket_fd};
    int len;
    for(int i = 0 ;i < 2; i++) {
      char message[MAX_MSGLEN] = {0};
      if(FD_ISSET(fds[i],&readset)) {
        len = recv(fds[i],message,sizeof(message),0);
        if(len <= 0) {
       	  return;
        }
        else {
          if (send(fds[1 - i], message, len, 0) <= 0) {
            return;
          }
        }
      }
    }
  }
}

void Proxy::postRequest(int client_connect_socket_fd, int request_server_fd, Request req, void * hook){
  Hook * h = (Hook *) hook;
  Log * log = (Log *) h->getLog();
  if(req.getReqCntLength() == -1){
    return;
  }
  std::string request = req.getRequest();
  send(request_server_fd, request.data(), request.size()+1, 0);
  char response[MAX_MSGLEN] = {0};
  int response_length = recv(request_server_fd,response,sizeof(response),MSG_WAITALL);
  /*
  if(response_length > 0){
    Response res(response);
    log->writeLogFile(h, req.getRequestLine(), RECEIVE);
    send(client_connect_socket_fd,response,response_length,0);
    log->writeLogFile(h, req.getRequestLine(), RESPOND);
    return;
  }
  */
  return;
}

void Proxy::check502(int client_connect_socket_fd, std::string response, void * hook){
  if(response.find("\r\n\r\n") == std::string::npos){
    const char * bad_gateway_msg = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
    send(client_connect_socket_fd, bad_gateway_msg, sizeof(bad_gateway_msg), 0);
    Hook * h = (Hook *) hook;
    Log * log = (Log *) h->getLog();
    log->writeLogFile(h, "HTTP/1.1 502 Bad Gateway", RESPOND);
  }
  return;
}

const char * Proxy::getPortNum(){
  return this->port;
}

void Proxy::putResponseToCache(std::string uri, Response response, Cache * cache) {
  std::string status_code = response.getStatusCode();
  std::string cache_control = response.getCacheControl();
  if(status_code == "200" && response.isCachable()) {
    cache->put(uri, response.getResponse());
    // need log expire or revalidate
  }
  else {
    std::string reason;
    if(response.isPrivate()) {
      reason = "Cache-Control: private";
      // need log
    }
    else if(response.isNoStore()) {
      reason = "Cache-Control: no-store";
      // need log
    }
  }
}

void Proxy::sendString(int socket_fd, std::string msg) {
  send(socket_fd, msg.data(), msg.size() + 1, 0);
}

std::string Proxy::sendNewRequest(int socket_fd, Request request) {
  std::string old_req = request.getRequest();
  sendString(socket_fd, old_req);
  std::vector<char> v;
  recvHelper(socket_fd, v);
  Response new_response(v);
  return new_response.getResponse();
}

std::string Proxy::revalidate(int socket_fd, Request request, Response response, Cache * cache) {
  std::string Etag = response.getEtag();
  std::string last_modified = response.getLastModified();
  if(Etag != "") {
    return validateCache(socket_fd, request, response, "Etag", Etag, cache);
  }
  else if(last_modified != "") {
    return validateCache(socket_fd, request, response, "Last-Modified", last_modified, cache);
  }
  else {
    std::string new_response = sendNewRequest(socket_fd, request);
    putResponseToCache(request.getUri(), new_response, cache);
    return new_response;
  }
}

std::string Proxy::validateCache(int connect_server_fd, Request request, Response response, 
                          std::string check_type, std::string check_content, Cache * cache) {
  std::string old_req_head = request.getRequestHead();
  old_req_head += "\r\n" + check_type + ": " + check_content + "\r\n\r\n";
  sendString(connect_server_fd, old_req_head);
  std::vector<char> new_response_v;
  recvHelper(connect_server_fd, new_response_v);
  Response new_response(new_response_v);
  if(new_response.getStatusCode() == "304") {
    return response.getResponse();
  }
  else {
    putResponseToCache(request.getUri(), new_response, cache);
    return new_response.getResponse();
  }
}