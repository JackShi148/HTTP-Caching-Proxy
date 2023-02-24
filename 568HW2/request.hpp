#ifndef __REQUEST_HPP__
#define __REQUEST_HPP__

#include <iostream>
#include <string>
#include <vector>

#include "client.h"
#include "exception.h"

class Request {
    std::string request;
    std::string request_line;
    std::string method;
    std::string uri;
    std::string hostname;
    std::string port;
    std::string httpVer;
    void parseMethod();
    void parseUri();
    void parseHostPort();
public:
    Request(std::vector<char> &request_msg) {
        std::string req(request_msg.begin(), request_msg.end());
        request = req;
        size_t http_pos = request.find("HTTP/");
        if(http_pos == std::string::npos) {
            throw Exception("REQUEST ERROR");
        }
        httpVer = request.substr(http_pos, 8);
        size_t line_end = request.find("\r\n");
        request_line = request.substr(0, line_end);
        parseMethod();
        parseHostPort();
        parseUri();
    }
    Request(std::string request_msg) : request(request_msg) {
        size_t http_pos = request.find("HTTP/");
        if(http_pos == std::string::npos) {
            throw Exception("REQUEST ERROR");
        }
        httpVer = request.substr(http_pos, 8);
        size_t line_end = request.find("\r\n");
        request_line = request.substr(0, line_end);
        parseMethod();
        parseHostPort();
        parseUri();
    }
    std::string formatFinder(std::string field);
    int getReqCntLength();
    std::string getRequest() {
        return request;
    }
    std::string getRequestHead(){
        size_t start_pos = this->request.find("\r\n\r\n");
        return this->request.substr(0, start_pos);
    }
    std::string getRequestLine() {
        return request_line;
    }
    std::string getMethod() {
        return method;
    }
    std::string getUri() {
        return uri;
    }
    std::string getHostName() {
        return hostname;
    }
    std::string getPort() {
        return port;
    }
    std::string getHttpVer() {
        return httpVer;
    }
};

#endif
