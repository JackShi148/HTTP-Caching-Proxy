#include "proxy.h"

void Proxy::startProxy()
{
  Server server(hostname, port);
  int socket_fd = server.createServer();
  int thread_id = 0;
  Log log;
  log.openLogFile("/var/log/erss/proxy.log");
  Cache cache(CACHE_CAPACITY);
  while (1)
  {
    std::string client_ip_addr;
    unsigned short int client_port;
    // proxy accepts connections from clients
    int client_connect_socket_fd = server.acceptConnections(&client_ip_addr, &client_port);
    Hook *ahook = new Hook(thread_id, client_connect_socket_fd, client_ip_addr, client_port, this, &log, &cache);
    pthread_t thread;
    pthread_create(&thread, NULL, routeRequest, ahook);
    std::cout << "New Thread Created: thread_id: " << thread_id << ", client_connect_socket_fd: "
              << client_connect_socket_fd << ", client_IP_address: " << client_ip_addr << ", client_port: " << client_port << std::endl;
    ++thread_id;
  }
  log.closeLogFile();
}

void *Proxy::routeRequest(void *ahook)
{
  Hook *hook_info = (Hook *)ahook;
  int client_connect_socket_fd = hook_info->getClientConnectSocketFD();
  Proxy *p = (Proxy *)hook_info->getThisObject();
  Log *log = (Log *)hook_info->getLog();
  char request[MAX_MSGLEN] = {0};
  int len = recv(client_connect_socket_fd, request, sizeof(request), 0);
  if (len <= 0)
  {
    log->writeLogFile(hook_info, "Invalid Request", WARNING);
    return NULL;
  }
  std::string req_msg = std::string(request, len);

  int request_server_fd;
  try
  {
    Request r(req_msg);
    std::string method = r.getMethod();
    std::string request_hostname = r.getHostName();
    std::string request_port = r.getPort();
    log->writeLogFile(hook_info, r.getRequestLine(), NEW_REQUEST);

    hook_info->setReqHostName(r.getHostName());
    hook_info->setHttpVer(r.getHttpVer());

    std::cout << hook_info->getThreadID() << ": received request from client -> " << r.getRequestLine() << std::endl;

    Client client(request_hostname, request_port);
    request_server_fd = client.connectServer();

    std::cout << "Client(" << hook_info->getThreadID() << ", " << hook_info->getClientIPAddr() << ") requests a " << method
              << " to Server(" << request_server_fd << ", " << request_hostname << ")" << std::endl;

    if (method == "CONNECT")
    {
      log->writeLogFile(hook_info, r.getRequestLine(), REQUEST);
      p->proxyResponse(client_connect_socket_fd, "SUCCESS", ahook);
      p->connectRequest(client_connect_socket_fd, request_server_fd, ahook);
      log->writeLogFile(hook_info, "Tunnel closed", LOGMSG);
    }
    else if (method == "GET")
    {
      log->writeLogFile(hook_info, r.getRequestLine(), REQUEST);
      p->getRequest(client_connect_socket_fd, request_server_fd, r, ahook);
    }
    else if (method == "POST")
    {
      log->writeLogFile(hook_info, r.getRequestLine(), REQUEST);
      p->postRequest(client_connect_socket_fd, request_server_fd, r, ahook);
    }
  }
  catch (Exception e)
  {
    std::string errMsg;
    std::string exp = std::string(e.what());
    p->proxyResponse(client_connect_socket_fd, exp, ahook);
    close(request_server_fd);
    close(client_connect_socket_fd);
    delete hook_info;
    return NULL;
  }

  close(request_server_fd);
  close(client_connect_socket_fd);
  delete hook_info;
  return NULL;
}

void Proxy::connectRequest(int client_connect_socket_fd, int request_server_fd, void *hook)
{
  fd_set readset;
  int maxfdp1 = client_connect_socket_fd > request_server_fd ? client_connect_socket_fd + 1 : request_server_fd + 1;
  while (1)
  {
    FD_ZERO(&readset);
    FD_SET(request_server_fd, &readset);
    FD_SET(client_connect_socket_fd, &readset);
    select(maxfdp1, &readset, NULL, NULL, NULL);
    int fds[2] = {request_server_fd, client_connect_socket_fd};
    int len;
    for (int i = 0; i < 2; i++)
    {
      char message[MAX_MSGLEN] = {0};
      if (FD_ISSET(fds[i], &readset))
      {
        len = recv(fds[i], message, sizeof(message), 0);
        if (len <= 0)
        {
          return;
        }
        else
        {
          if (send(fds[1 - i], message, len, 0) <= 0)
          {
            return;
          }
        }
      }
    }
  }
}

void Proxy::getRequest(int client_connect_socket_fd, int request_server_fd, Request req, void *hook)
{
  Hook *h = (Hook *)hook;
  Proxy *p = (Proxy *)h->getThisObject();
  Log *log = (Log *)h->getLog();
  Cache *cache = (Cache *)h->getCache();
  std::string uri = req.getUri();
  std::string cached_response = cache->getResponse(uri);
  int max_stale = req.getMaxStale();
  if (cached_response == "")
  {
    // not in cache
    log->writeCacheLog(h, "not in cache", CACHE);
    std::string request = req.getRequest();

    send(request_server_fd, request.data(), request.size(), 0);
    log->writeLogFile(h, req.getRequestLine(), REQUEST);

    // receive responses
    std::string server_response = p->getEntireResponse(request_server_fd);

    // receive request server response
    Response res(server_response);
    log->writeLogFile(h, res.getResponseLine(), RECEIVE);
    p->printNoteLog(res, max_stale, hook);
    send(client_connect_socket_fd, server_response.data(), server_response.size(), 0);
    p->trySaveResponse(uri, server_response, max_stale, hook);
    log->writeLogFile(h, res.getResponseLine(), RESPOND);
  }
  else
  {
    // in cache
    Response res(cached_response);
    if (res.needRevalidate(max_stale))
    {
      if (res.pastDue(max_stale))
      {
        std::string expTime = res.getWhenExpire(max_stale);
        log->writeCacheLog(h, expTime, CACHE_EXPIREDTIME);
      }
      else
      {
        log->writeCacheLog(h, "in cache, requires validation", CACHE);
      }
      std::string revalidated_response = p->revalidate(request_server_fd, req, res, hook);
      send(client_connect_socket_fd, revalidated_response.data(), revalidated_response.size(), 0);
    }
    else
    {
      log->writeCacheLog(h, "in cache, valid", CACHE);
      send(client_connect_socket_fd, cached_response.data(), cached_response.size(), 0);
    }
  }
}

void Proxy::postRequest(int client_connect_socket_fd, int request_server_fd, Request req, void *hook)
{
  Hook *h = (Hook *)hook;
  Log *log = (Log *)h->getLog();
  Proxy *p = (Proxy *)h->getThisObject();
  if (req.getReqCntLength() == -1)
  {
    return;
  }
  std::string request = req.getRequest();
  send(request_server_fd, request.data(), request.size(), 0);
  std::string response = p->getEntireResponse(request_server_fd);
  int response_length = response.size();
  if (response_length > 0)
  {
    Response res(response);
    log->writeLogFile(h, res.getResponseLine(), RECEIVE);
    send(client_connect_socket_fd, response.data(), response_length, 0);
    log->writeLogFile(h, res.getResponseLine(), RESPOND);
    std::cout << h->getThreadID() << ": succeed in posting client request -> " << req.getRequestLine() << std::endl;
    return;
  }
  std::cout << h->getThreadID() << ": failed to post client request -> " << req.getRequestLine() << std::endl;
  return;
}

void Proxy::proxyResponse(int client_connect_socket_fd, std::string exp, void *hook)
{
  Hook *h = (Hook *)hook;
  Log *log = (Log *)h->getLog();
  std::string errMsg = h->getHttpVer() + " ";
  if (exp == "SUCCESS")
  {
    errMsg += "200 OK";
  }
  else if (exp == "URL ERROR")
  {
    errMsg += "404 Not Found";
  }
  else if (exp == "REQUEST ERROR")
  {
    errMsg += "400 Bad Request";
  }
  else if (exp == "RESPONSE ERROR")
  {
    errMsg += "502 Bad Gateway";
  }
  else
  {
    errMsg += "503 Server Unavailable";
  }
  char sendMsg[errMsg.length() + 4] = {0};
  for (int i = 0; i < errMsg.length(); i++)
  {
    sendMsg[i] = errMsg[i];
  }
  sendMsg[errMsg.length()] = '\r';
  sendMsg[errMsg.length() + 1] = '\n';
  sendMsg[errMsg.length() + 2] = '\r';
  sendMsg[errMsg.length() + 3] = '\n';
  send(client_connect_socket_fd, sendMsg, sizeof(sendMsg), 0);
  log->writeLogFile(h, errMsg, RESPOND);
}

std::string Proxy::getEntireResponse(int request_server_fd)
{
  std::vector<std::string> msg;
  // process response head
  ssize_t tol_msg_length = 0;
  int len = 0;
  std::string response_head_string = "";
  while (response_head_string.find("\r\n\r\n") == std::string::npos)
  {
    char response_head[MAX_MSGLEN] = {0};
    response_head_string = "";
    if ((len = recv(request_server_fd, response_head, sizeof(response_head), 0)) <= 0)
    {
      break;
    }
    tol_msg_length += len;
    response_head_string = std::string(response_head, len);
    msg.push_back(response_head_string);
  }

  response_head_string = "";
  response_head_string = std::accumulate(msg.begin(), msg.end(), response_head_string);
  Response res(response_head_string);

  ssize_t head_len = response_head_string.find("\r\n\r\n") + 4;
  ssize_t body_len = tol_msg_length - head_len;

  if (res.getStatusCode() == "304")
  {
    return response_head_string;
  }

  // process main body
  std::string response_body_string = "";
  if (res.isChunked())
  {
    while (response_body_string.find("0\r\n\r\n") == std::string::npos)
    {
      char response_body[MAX_MSGLEN] = {0};
      if ((len = recv(request_server_fd, response_body, sizeof(response_body), 0)) <= 0)
      {
        break;
      }
      response_body_string = std::string(response_body, len);
      msg.push_back(response_body_string);
    }
  }
  else
  {
    int msg_len = res.getContentLength();
    if (msg_len == -1 || body_len == msg_len)
    {
      return response_head_string;
    }
    while (body_len < msg_len)
    {
      char response_body[MAX_MSGLEN] = {0};
      if ((len = recv(request_server_fd, response_body, sizeof(response_body), 0)) <= 0)
      {
        break;
      }
      response_body_string = std::string(response_body, len);
      msg.push_back(response_body_string);
      body_len += len;
    }
  }
  std::string response_string = "";
  return std::accumulate(msg.begin(), msg.end(), response_string);
}

void Proxy::trySaveResponse(std::string uri, Response response, int max_stale, void *hook)
{
  Hook *h = (Hook *)hook;
  Log *log = (Log *)h->getLog();
  Cache *cache = (Cache *)h->getCache();
  std::string status_code = response.getStatusCode();
  std::string cache_control = response.getCacheControl();
  if (status_code == "200" && response.isCachable())
  {
    std::string remove_uri = cache->put(uri, response.getResponse());
    if (remove_uri != "")
    {
      log->writeLogFile(h, remove_uri, EVICT);
    }
    // need log expire or revalidate
    if (response.isNoCache())
    {
      log->writeCacheLog(h, "cached, but requires re-validation", CACHE);
    }
    else
    {
      std::string expTime = response.getWhenExpire(max_stale);
      log->writeCacheLog(h, expTime, CACHE_EXPIRES);
    }
  }
  else
  {
    if (response.isPrivate())
    {
      log->writeCacheLog(h, "the Cache-Control contains private", NOT_CACHEABLE);
    }
    else if (response.isNoStore())
    {
      log->writeCacheLog(h, "the Cache-Control contains no-store", NOT_CACHEABLE);
    }
  }
}

std::string Proxy::revalidate(int request_server_fd, Request request, Response response, void *hook)
{
  Hook *h = (Hook *)hook;
  Cache *cache = (Cache *)h->getCache();
  Proxy *p = (Proxy *)h->getThisObject();
  std::string Etag = response.getEtag();
  std::string last_modified = response.getLastModified();
  if (Etag != "")
  {
    return p->validateCache(request_server_fd, request, response, "If-None-Match", Etag, cache);
  }
  else if (last_modified != "")
  {
    return p->validateCache(request_server_fd, request, response, "If-Modified-Since", last_modified, cache);
  }
  else
  {
    // pass due
    std::string old_req = request.getRequest();
    send(request_server_fd, old_req.data(), old_req.size(), 0);
    std::string new_response = p->getEntireResponse(request_server_fd);
    p->trySaveResponse(request.getUri(), new_response, request.getMaxStale(), hook);
    return new_response;
  }
}

std::string Proxy::validateCache(int request_server_fd, Request request, Response response,
                                 std::string check_type, std::string check_content, void *hook)
{
  Hook *h = (Hook *)hook;
  Cache *cache = (Cache *)h->getCache();
  Proxy *p = (Proxy *)h->getThisObject();
  std::string old_req_head = request.getRequestHead();
  if (old_req_head.find(check_type) == std::string::npos)
  {
    old_req_head += "\r\n" + check_type + ": " + check_content + "\r\n\r\n";
  }
  else
  {
    old_req_head += "\r\n\r\n";
  }
  send(request_server_fd, old_req_head.data(), old_req_head.size(), 0);
  // receive new response
  std::string res = p->getEntireResponse(request_server_fd);
  Response new_response(res);
  if (new_response.getStatusCode() == "304")
  {
    return response.getResponse();
  }
  else
  {
    p->trySaveResponse(request.getUri(), new_response, request.getMaxStale(), hook);
    return new_response.getResponse();
  }
}

void Proxy::printNoteLog(Response res, int max_stale, void *hook)
{
  std::string status_code = res.getStatusCode();
  if (status_code == "200")
  {
    Hook *h = (Hook *)hook;
    Log *log = (Log *)h->getLog();
    if (res.getCacheControl() != "")
    {
      std::string msg = "Cache-Control: " + res.getCacheControl();
      log->writeLogFile(h, msg, NOTE);
    }
    if (res.getEtag() != "")
    {
      std::string msg = "ETag: " + res.getEtag();
      log->writeLogFile(h, msg, NOTE);
    }
    if (res.getLastModified() != "")
    {
      std::string msg = "Last-Modified: " + res.getLastModified();
      log->writeLogFile(h, msg, NOTE);
    }
    std::string expTime = res.formatFinder("Expires");
    if (expTime != "" && expTime != "-1" && expTime != "0")
    {
      time_t expire_moment = mktime(res.getExpireTime().convertGMT());
      const char *expTime_c = asctime(gmtime(&expire_moment));
      std::string expireTime = std::string(expTime_c);
      std::string msg = "Expires: " + expireTime.substr(0, expireTime.size() - 1) + " GMT";
      log->writeLogFile(h, msg, NOTE);
    }
    if (res.formatFinder("Date") != "")
    {
      time_t res_moment = mktime(res.getResponseTime().convertGMT());
      const char *resTime_c = asctime(gmtime(&res_moment));
      std::string resTime = std::string(resTime_c);
      std::string msg = "Date: " + resTime.substr(0, resTime.size() - 1) + " GMT";
      log->writeLogFile(h, msg, NOTE);
    }
  }
}

const char *Proxy::getHostName()
{
  return this->hostname;
}

const char *Proxy::getPortNum()
{
  return this->port;
}
