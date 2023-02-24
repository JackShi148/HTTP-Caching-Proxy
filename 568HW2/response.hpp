#ifndef __RESPONSE_HPP__
#define __RESPONSE_HPP__
#include <iostream>
#include <string>
#include <vector>
#include <time.h>

#include "exception.h"
#include "timeprocessing.hpp"

class Response {
    int max_age;
    int s_maxage;
    std::string response_msg;
    std::string status_line; 
    std::string Etag;    
    std::string last_modified;
    TimeInfo response_time;  
    TimeInfo expire_time;  
    std::string cache_info;  // max-age   no-cache  no-store  private
    std::string formatFinder(std::string field);
    void parseResponse();
    int parseMaxAge();
    int parseSMaxAge();
public:
    Response(std::vector<char> msg) {
        std::string temp(msg.begin(), msg.end());
        response_msg = temp;
        parseResponse();
    }
    Response(std::string response_msg) : response_msg(response_msg) {
        parseResponse();
    }
    std::string getResponseHead();
    std::string getResponseLine();
    std::string getResponse();
    std::string getEtag();
    std::string getLastModified();
    TimeInfo getResponseTime();
    TimeInfo getExpireTime();
    std::string getResponseTime_str();
    std::string getExpireTime_str();
    std::string getCacheControl();
    std::string getWhenExpire();
    std::string getHttpVer();
    std::string getStatusCode();
    int getContentLength();
    int getMaxAge();
    int getSMaxAge();    
    bool isChunked();
    bool isPrivate();
    bool isNoCache();
    bool isNoStore();
    bool pastDue();
    bool isCachable();
    bool needRevalidate();
};

#endif
