#include "response.hpp"

void Response::parseResponse() {
    this->max_age = parseMaxAge();
    this->s_maxage = parseSMaxAge();
    size_t header_end = this->response_msg.find_first_of("\r\n");
    this->status_line = this->response_msg.substr(0, header_end);
    this->Etag = formatFinder("Etag");
    this->last_modified = formatFinder("Last-Modified");
    this->cache_info = formatFinder("Cache-Control");
    
    std::string respTime = formatFinder("Date");
    if(respTime != "") {
         this->response_time.parse(respTime);
    }
    std::string expTime = formatFinder("Expires");
    if(expTime != "" && expTime != "-1" && expTime != "0") {
        this->expire_time.parse(expTime);
    }
}

std::string Response::formatFinder(std::string field) {
    size_t start_pos = this->response_msg.find(field);
    if(start_pos == std::string::npos) {
        return "";
    }
    size_t end_pos = this->response_msg.find_first_of("\r\n", start_pos + 1);
    // skip the field name, colon and whitespace
    size_t field_len = field.size();
    start_pos += field_len + 2;
    std::string ans = this->response_msg.substr(start_pos, end_pos - start_pos);
    return ans;
}

std::string Response::getResponseHead() {
    size_t start_pos = this->response_msg.find("\r\n\r\n");
    if(start_pos == std::string::npos){
        throw Exception("RESPONSE ERROR");
    }
    return this->response_msg.substr(0, start_pos);
}

bool Response::isChunked() {
    std::string encode_info = formatFinder("Transfer-Encoding");
    if(encode_info.find("chunked") != std::string::npos) {
        return true;
    }
    return false;
}

std::string Response::getResponseLine() {
    return this->status_line;
}

int Response::getContentLength() {
    std::string content_len = formatFinder("Content-Length");
    if(content_len == "") {
        return -1;
    }
    int ans = stoi(content_len);
    return ans;
}

std::string Response::getStatusCode() {
    size_t pos1 = this->status_line.find_first_of(' ');
    size_t pos2 = this->status_line.find_first_of(' ', pos1 + 1);
    std::string code = this->status_line.substr(pos1 + 1, pos2 - pos1 - 1);
    return code;
}

std::string Response::getHttpVer() {
    size_t pos1 = this->status_line.find_first_of(' ');
    std::string httpVer = this->status_line.substr(0, pos1);
    return httpVer;
}

int Response::parseMaxAge() {
    int ans = -1;
    size_t start_pos = this->cache_info.find("max-age=");
    if(start_pos != std::string::npos) {
        size_t end_pos = this->cache_info.find_first_of(',', start_pos + 1);
        if(end_pos == std::string::npos) {
            ans = std::stoi(this->cache_info.substr(start_pos + 8));
        }
        else {
            ans = std::stoi(this->cache_info.substr(start_pos + 8, end_pos - start_pos - 8));
        }
    }
    return ans;
}

int Response::getMaxAge() {
    return this->max_age;
}

int Response::parseSMaxAge() {
    int ans = -1;
    size_t start_pos = this->cache_info.find("s-maxage=");
    if(start_pos != std::string::npos) {
        size_t end_pos = this->cache_info.find_first_of(',', start_pos + 1);
        if(end_pos == std::string::npos) {
            ans = std::stoi(this->cache_info.substr(start_pos + 9));
        }
        else {
            ans = std::stoi(this->cache_info.substr(start_pos + 9, end_pos - start_pos - 9));
        }
    }
    return ans;
}

int Response::getSMaxAge() {
    return this->s_maxage;
}

bool Response::isPrivate() {
    if(size_t pos = this->cache_info.find("private") != std::string::npos) {
        return true;
    }
    return false;
}

bool Response::isNoCache() {
    if(size_t pos = this->cache_info.find("no-cache") != std::string::npos) {
        return true;
    }
    return false;
}

bool Response::isNoStore() {
    if(size_t pos = this->cache_info.find("no-store") != std::string::npos) {
        return true;
    }
    return false;
}

std::string Response::getWhenExpire() {
    time_t respTime = mktime(response_time.getTimeInfo());
    if(getSMaxAge() != -1) {
        time_t expire_moment = respTime + (time_t)getSMaxAge();
        const char * expTime_c = asctime(gmtime(&expire_moment));
        std::string expTime = std::string(expTime_c);
        return expTime;
    }
    else if(getMaxAge() != -1) {
        time_t expire_moment = respTime + (time_t)getMaxAge();
        const char * expTime_c = asctime(gmtime(&expire_moment));
        std::string expTime = std::string(expTime_c);
        return expTime;
    }
    else if(formatFinder("Expires") != "") {
        time_t expire_moment = mktime(expire_time.getTimeInfo());
        const char * expTime_c = asctime(gmtime(&expire_moment));
        std::string expTime = std::string(expTime_c);
        return expTime;
    }
    else {
        time_t expire_moment = respTime + 300;
        const char * expTime_c = asctime(gmtime(&expire_moment));
        std::string expTime = std::string(expTime_c);
        return expTime;
    }
}

bool Response::pastDue() {
    time_t now = time(0);
    tm * tm_gmt = gmtime(&now);
    time_t nowTime = mktime(tm_gmt);
    time_t respTime = mktime(response_time.getTimeInfo());
    int lifeSpan = nowTime - respTime;
    if(getSMaxAge() != -1) {
        return getSMaxAge() <= lifeSpan;
    }
    else if(getMaxAge() != -1) {
        return getMaxAge() <= lifeSpan;
    }
    else if(formatFinder("Expires") != "") {
        time_t expTime = mktime(expire_time.getTimeInfo());
        return difftime(expTime, nowTime) <= 0;
    }
    else {
        return 300 <= lifeSpan;
    }
}

bool Response::isCachable() {
    if(!isPrivate() && !isNoStore() && !isChunked()) {
        return true;
    }
    return false;
}

bool Response::needRevalidate() {
    if(isNoCache()) {
        return true;
    }
    return pastDue();
}

std::string Response::getResponse() {
    return this->response_msg;
}

std::string Response::getResponseTime_str() {
    std::string respTime = formatFinder("Date");
    return respTime;
}

std::string Response::getExpireTime_str() {
    std::string expTime = formatFinder("Expires");
    return expTime;
}

std::string Response::getEtag() {
    return this->Etag;
}

std::string Response::getLastModified() {
    return this->last_modified;
}

TimeInfo Response::getResponseTime() {
    return this->response_time;
}

TimeInfo Response::getExpireTime() {
    return this->expire_time;
}

std::string Response::getCacheControl() {
    return this->cache_info;
}