#include "request.h"

std::string Request::formatFinder(std::string field) {
    size_t start_pos = this->request.find(field);
    if(start_pos == std::string::npos) {
        return "";
    }
    size_t end_pos = this->request.find("\r\n", start_pos);
    // skip the field name, colon and whitespace
    size_t field_len = field.size();
    start_pos += field_len + 2;
    std::string ans = this->request.substr(start_pos, end_pos - start_pos);
    return ans;
}

void Request::parseMethod() {
    size_t end_pos = this->request_line.find(' ');
    this->method = this->request_line.substr(0, end_pos);
}

void Request::parseUri() {
    size_t start_pos = this->request_line.find(' ');
    size_t end_pos = this->request_line.find(' ', start_pos + 1);
    this->uri = this->request_line.substr(start_pos + 1, end_pos - start_pos - 1);
}

void Request::parseHostPort() {
    size_t host_pos = this->request.find("Host: ");
    if(host_pos != std::string::npos) {
        host_pos += 6;
        size_t end_pos = this->request.find("\r\n", host_pos);
        std::string host_info = this->request.substr(host_pos, end_pos - host_pos);
        size_t port_pos = host_info.find(':');
        if(port_pos == std::string::npos) {
            this->port = "80";
            this->hostname = host_info;
        }
        else {
            this->port = host_info.substr(port_pos + 1);
            this->hostname = host_info.substr(0, port_pos);
        }
    }
    else {
        if(this->method == "CONNECT") {
            size_t start_pos = this->request_line.find(' ');
            size_t inter_pos = this->request_line.find(':', start_pos + 1);
            size_t end_pos = this->request_line.find(' ', start_pos + 1);
            this->hostname = this->request_line.substr(start_pos + 1, inter_pos - start_pos - 1);
            this->port = this->request_line.substr(inter_pos + 1, end_pos - inter_pos - 1);
        }
        else {
            size_t start_pos = this->request_line.find("//");
            start_pos += 2;
            size_t end_pos = this->request_line.find('/', start_pos);
            this->hostname = this->request_line.substr(start_pos, end_pos - start_pos);
            // HTTP default port: 80
            this->port = "80";
        }
    }
}

int Request::getReqCntLength() {
    size_t start_pos = this->request.find("Content-Length: ");
    if(start_pos == string::npos) {
        return -1;
    }
    size_t head_end_pos = this->request.find("\r\n\r\n");
    int content_len = this->request.size() - head_end_pos - 8;
    size_t end_pos = this->request.find("\r\n", start_pos);
    string req_len_str = this->request.substr(start_pos + 16, end_pos - start_pos - 16);
    int req_len = stoi(req_len_str);
    return req_len - content_len - 8;
}
