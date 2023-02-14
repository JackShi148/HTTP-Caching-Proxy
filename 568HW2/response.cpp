#include "response.hpp"

void Response::parseResponse() {
    
}

std::string Response::formatFinder(std::string field) {
    size_t start_pos = this->response_msg.find(field);
    if(start_pos == std::string::npos) {
        return "";
    }
    size_t end_pos = this->response_msg.find("\r\n");
    // skip the field name, colon and whitespace
    size_t field_len = field.size();
    start_pos += field_len + 2;
    std::string ans = this->response_msg.substr(start_pos, end_pos - start_pos);
    return ans;
}

bool Response::isChunked() {
    std::string encode_info = formatFinder("Transfer-Encoding");
    if(encode_info.find("chunked") != std::string::npos) {
        return true;
    }
    return false;
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
    size_t pos1 = this->status_line.find(' ');
    size_t pos2 = this->status_line.find(' ', pos1 + 1);
    std::string code = this->status_line.substr(pos1 + 1, pos2 - pos1 - 1);
    return code;
}

std::string Response::getHttpVer() {
    size_t pos1 = this->status_line.find(' ');
    std::string httpVer = this->status_line.substr(0, pos1);
    return httpVer;
}