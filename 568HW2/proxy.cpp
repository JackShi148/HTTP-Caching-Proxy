#include "proxy.h"

void Proxy::startProxy(){
  Server server(hostname, port);
  int socket_fd = server.createServer();
  int thread_id = 0;
  Log log;
  Cache cache(CACHE_CAPACITY);
  log.openLogFile("./proxy.log");
  while(1){
    std::string client_ip_addr;
    unsigned short int client_port;
    //proxy accepts connections from clients
    int client_connect_socket_fd = server.acceptConnections(&client_ip_addr,&client_port);
    Hook * ahook = new Hook(thread_id, client_connect_socket_fd, client_ip_addr, client_port, this, &log, &cache);
    pthread_t thread;
    pthread_create(&thread, NULL, routeRequest, ahook);
    std::cout<<"New Thread Created: thread_id: " << thread_id << ", client_connect_socket_fd: "
    << client_connect_socket_fd << ", client_IP_address: " << client_ip_addr << ", client_port: " << client_port << std::endl;
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

void Proxy::getRequest(int client_connect_socket_fd, int request_server_fd, Request req, void * hook){
  Hook * h = (Hook *) hook;
  Proxy * p = (Proxy *) h->getThisObject();
  Log * log = (Log *) h->getLog();
  Cache * cache = (Cache *) h->getCache();
  std::string uri = req.getUri();
  std::string cached_response = cache->getResponse(uri);
  if(cached_response == ""){
    //not in cache
    log->writeCacheLog(h, "not in cache", CACHE);
    std::string request = req.getRequest();
    send(request_server_fd, request.data(), request.size() + 1, 0);
    log->writeLogFile(h, req.getRequestLine(), REQUEST);

    //receive responses
    std::string response = p->getEntireResponse(request_server_fd);
    
    //receive request server response
    Response res(response);
    log->writeLogFile(h, res.getResponseLine(), RECEIVE);
    send(client_connect_socket_fd, response.data(), response.size() + 1, 0); // ?
    
    p->trySaveResponse(req.getUri(), res, hook);
    log->writeLogFile(h, res.getResponseLine(), RESPOND);
  }
  else{
    //in cache
    Response res(cached_response);
    if(res.needRevalidate()){
      if(res.pastDue()) {
          std::string expTime = res.getWhenExpire();
          log->writeCacheLog(h, expTime, CACHE_EXPIREDTIME);
      }
      else {
        log->writeCacheLog(h, "in cache, requires validation", CACHE);
      }
      std::string new_response = p->revalidate(request_server_fd, req, res, hook);
    }
    else{
      log->writeCacheLog(h, "in cache, valid", CACHE);
    }
  }
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
  if(response_length > 0){
    Response res(response);
    log->writeLogFile(h, res.getResponseLine(), RECEIVE);
    send(client_connect_socket_fd,response,response_length,0);
    log->writeLogFile(h, res.getResponseLine(), RESPOND);
    std::cout << h->getThreadID() << ": succeed in posting client request -> " << req.getRequestLine() << std::endl;
    return;
  }
  std::cout << h->getThreadID() << ": failed to post client request -> " << req.getRequestLine() << std::endl;
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

void Proxy::proxyResponse(int client_connect_socket_fd, std::string exp, void * hook){
  Hook * h = (Hook *) hook;
  Log * log = (Log *) h->getLog();
  std::string errMsg = h->getHttpVer() + " ";
  if(exp == "SUCCESS"){
    errMsg += "200 OK";
  }else if(exp == "URL ERROR"){
    errMsg += "404 Not Found";
  }else if(exp == "REQUEST ERROR"){
    errMsg += "400 Bad Request";
  }else{
    errMsg += "503 Server Unavailable";
  }
  char sendMsg[errMsg.length()+4] = {0};
  for(int i = 0; i < errMsg.length(); i++){
    sendMsg[i] = errMsg[i];
  }
  sendMsg[errMsg.length()] = '\r';
  sendMsg[errMsg.length()+1] = '\n';
  sendMsg[errMsg.length()+2] = '\r';
  sendMsg[errMsg.length()+3] = '\n';
  send(client_connect_socket_fd, sendMsg, sizeof(sendMsg), 0);
  log->writeLogFile(h, errMsg, RESPOND);
}

void Proxy::trySaveResponse(std::string uri, Response response, void * hook) {
  Hook * h = (Hook*)hook;
  Log * log = (Log *) h->getLog();
  Cache * cache = (Cache*)h->getCache();
  std::string status_code = response.getStatusCode();
  std::string cache_control = response.getCacheControl();
  if(status_code == "200" && response.isCachable()) {
    cache->put(uri, response.getResponse());
    // need log expire or revalidate
    if(response.isNoCache()) {
      log->writeCacheLog(h, "cached, but requires re-validation", CACHE);
    }
    else if(response.getExpireTime_str() != "") {
      std::string expTime = response.getWhenExpire();
      log->writeCacheLog(h, expTime, CACHE_EXPIREDTIME); 
    }
  }
  else {
    if(response.isPrivate()) {
      log->writeCacheLog(h, "not cacheable because the Cache-Control contains private", CACHE);
    }
    else if(response.isNoStore()) {
      log->writeCacheLog(h, "not cacheable because the Cache-Control contains no-store", CACHE);
    }
    else if(response.isChunked()) {
      log->writeCacheLog(h, "not cacheable because the response is chunked", CACHE);
    }
  }
}

std::string Proxy::sendNewRequest(int socket_fd, Request request, void * hook) {
  Hook * h = (Hook*)hook;
  Proxy * p = (Proxy*)h->getThisObject();
  std::string old_req = request.getRequest();
  send(socket_fd, old_req.c_str(), old_req.size() + 1, 0);
  std::string response = p->getEntireResponse(socket_fd);
  return response;
}

std::string Proxy::revalidate(int socket_fd, Request request, Response response, void * hook) {
  Hook * h = (Hook*)hook;
  Cache * cache = (Cache*)h->getCache();
  Proxy * p = (Proxy*)h->getThisObject();
  std::string Etag = response.getEtag();
  std::string last_modified = response.getLastModified();
  if(Etag != "") {
    return p->validateCache(socket_fd, request, response, "If-None-Match", Etag, cache);
  }
  else if(last_modified != "") {
    return p->validateCache(socket_fd, request, response, "If-Modified-Since", last_modified, cache);
  }
  else {
    std::string new_response = sendNewRequest(socket_fd, request, h);
    p->trySaveResponse(request.getUri(), new_response, cache);
    return new_response;
  }
}

std::string Proxy::validateCache(int connect_server_fd, Request request, Response response, 
                          std::string check_type, std::string check_content, void * hook) {
  Hook * h = (Hook*)hook;
  Cache * cache = (Cache*)h->getCache();
  Proxy * p = (Proxy*)h->getThisObject();
  std::string old_req_head = request.getRequestHead();
  old_req_head += "\r\n" + check_type + ": " + check_content + "\r\n\r\n";
  send(connect_server_fd, old_req_head.c_str(), old_req_head.size() + 1, 0);
  std::string res = p->getEntireResponse(connect_server_fd);
  Response new_response(res);
  if(new_response.getStatusCode() == "304") {
    return response.getResponse();
  }
  else {
    p->trySaveResponse(request.getUri(), new_response, cache);
    return new_response.getResponse();
  }
}

std::string Proxy::getEntireResponse(int request_server_fd){
  std::vector<std::string> msg;
  //process response head
  //ssize_t response_length = 0;
  std::string response_head_string = "";
  while(response_head_string.find("\r\n\r\n")==std::string::npos){
    char response_head[MAX_MSGLEN] = {0};
    int len;
    if((len = recv(request_server_fd,response_head,sizeof(response_head),0)) <= 0){
      break;
    }
    //response_length += len;
    response_head_string = std::string(response_head);
    msg.push_back(response_head_string);
    response_head_string = "";
  }

  response_head_string = std::accumulate(msg.begin(),msg.end(),response_head_string);
  Response res(response_head_string);
  if(res.getStatusCode() == "304"){
    return response_head_string;
  }
  
  //process main body
  std::string response_body_string = "";
  if(res.isChunked()){
    //memset(&response,0,sizeof(response));
    while(response_body_string.substr(0,response_body_string.find("\r\n")) != "0"
    || response_body_string.find("\r\n\r\n")==std::string::npos){
      char response_body[MAX_MSGLEN] = {0};
      if(recv(request_server_fd,response_body,sizeof(response_body),0) <= 0){
        break;
      }
      response_body_string = std::string(response_body);
      msg.push_back(response_body_string);
    }
  }else{
    int msg_len = res.getContentLength();
    if(msg_len == -1){
      return response_head_string;
    }
    int body_len = 0;
    while(body_len < msg_len){
      char response_body[MAX_MSGLEN] = {0};
      int len;
      if((len = recv(request_server_fd,response_body,sizeof(response_body),0)) <= 0){
        break;
      }
      response_body_string = std::string(response_body);
      msg.push_back(response_body_string);
      body_len += len;
    }
  }
  std::string response_string = "";
  return std::accumulate(msg.begin(),msg.end(),response_string);
}