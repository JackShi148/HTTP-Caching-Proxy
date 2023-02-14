#include "request.hpp"

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

void Request::validate() {
    try{
        if(this->method != "GET" && this->method != "POST" && this->method != "CONNECT") {
            throw Exception("The method is incorrect\n");
        }
    }
    catch(Exception e) {
        std::cout << e.what() << std::endl;
        return;
    }
}