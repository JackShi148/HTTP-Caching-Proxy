#include "request.hpp"

void Request::formatChecker()
{
    size_t start_pos = this->request.find("\r\n\r\n");
    if (start_pos == std::string::npos)
    {
        throw("REQUEST ERROR");
    }
}

std::string Request::formatFinder(std::string field)
{
    size_t start_pos = this->request.find(field);
    if (start_pos == std::string::npos)
    {
        return "";
    }
    size_t end_pos = this->request.find_first_of("\r\n", start_pos + 1);
    // skip the field name, colon and whitespace
    size_t field_len = field.size();
    start_pos += field_len + 2;
    std::string ans = this->request.substr(start_pos, end_pos - start_pos);
    return ans;
}

void Request::parseMethod()
{
    size_t end_pos = this->request_line.find(' ');
    if (end_pos == std::string::npos)
    {
        throw Exception("REQUEST ERROR");
    }
    this->method = this->request_line.substr(0, end_pos);
    if (this->method != "GET" && this->method != "POST" && this->method != "CONNECT")
    {
        throw Exception("REQUEST ERROR");
    }
}

void Request::parseUri()
{
    size_t start_pos = this->request_line.find(' ');
    size_t end_pos = this->request_line.find(' ', start_pos + 1);
    if (end_pos == std::string::npos)
    {
        throw Exception("REQUEST ERROR");
    }
    this->uri = this->request_line.substr(start_pos + 1, end_pos - start_pos - 1);
}

void Request::parseHostPort()
{
    size_t host_pos = this->request.find("Host: ");
    if (host_pos != std::string::npos)
    {
        if (this->httpVer == "HTTP/1.0")
        {
            throw Exception("REQUEST ERROR");
        }
        host_pos += 6;
        size_t end_pos = this->request.find("\r\n", host_pos);
        std::string host_info = this->request.substr(host_pos, end_pos - host_pos);
        size_t port_pos = host_info.find(':');
        if (port_pos == std::string::npos)
        {
            this->port = "80";
            this->hostname = host_info;
        }
        else
        {
            this->port = host_info.substr(port_pos + 1);
            this->hostname = host_info.substr(0, port_pos);
        }
    }
    else
    {
        if (this->httpVer == "HTTP/1.1")
        {
            throw Exception("REQUEST ERROR");
        }
        if (this->method == "CONNECT")
        {
            size_t slash_pos = this->request_line.find("//");
            size_t start_pos;
            size_t inter_pos;
            size_t end_pos;
            if (slash_pos == std::string::npos)
            {
                start_pos = this->request_line.find(' ');
                inter_pos = this->request_line.find(':', start_pos + 1);
                end_pos = this->request_line.find(' ', start_pos + 1);
            }
            else
            {
                start_pos = slash_pos + 1;
                inter_pos = this->request_line.find(':', start_pos + 1);
                end_pos = this->request_line.find(' ', start_pos + 1);
            }
            if (inter_pos == std::string::npos)
            {
                this->hostname = this->request_line.substr(start_pos + 1, end_pos - start_pos - 1);
                this->port = "443";
            }
            else
            {
                this->hostname = this->request_line.substr(start_pos + 1, inter_pos - start_pos - 1);
                this->port = this->request_line.substr(inter_pos + 1, end_pos - inter_pos - 1);
            }
        }
        else if (this->method == "POST" || this->method == "GET")
        {
            size_t start_pos = this->request_line.find("//");
            // what if no //
            if (start_pos == std::string::npos)
            {
                start_pos = this->request_line.find(' ');
                start_pos += 1;
            }
            else
            {
                start_pos += 2;
            }
            size_t end_pos = this->request_line.find('/', start_pos);
            if (end_pos == std::string::npos)
            {
                throw Exception("REQUEST ERROR");
            }
            this->hostname = this->request_line.substr(start_pos, end_pos - start_pos);
            // HTTP default port: 80
            size_t find_port = this->request_line.find(':', start_pos);
            if (find_port == std::string::npos)
            {
                this->port = "80";
            }
            else
            {
                this->port = this->request_line.substr(find_port + 1, end_pos - find_port - 1);
            }
        }
        else
        {
            throw Exception("REQUEST ERROR");
        }
    }
}

int Request::getReqCntLength()
{
    std::string req_content_length = this->formatFinder("Content-Length");
    if (req_content_length != "")
    {
        return std::stoi(req_content_length);
    }
    size_t start_pos = this->request.find_first_not_of("\r\n\r\n");
    std::string req_head = this->request.substr(0, start_pos);
    if (this->request.size() == req_head.size() + 4)
    {
        return -1;
    }
    else
    {
        int ans = this->request.size() - req_head.size() - 4;
        return ans;
    }
}

std::string Request::getRequestHead()
{
    size_t start_pos = this->request.find("\r\n\r\n");
    return this->request.substr(0, start_pos);
}

int Request::getMaxStale()
{
    int ans = 0;
    std::string cache_control = formatFinder("Cache-Control");
    size_t start_pos = cache_control.find("max-stale=");
    if (start_pos == std::string::npos)
    {
        return 0;
    }
    start_pos += 10;
    size_t end_pos = cache_control.find_first_of(',', start_pos);
    if (end_pos == std::string::npos)
    {
        ans = std::stoi(cache_control.substr(start_pos));
    }
    else
    {
        ans = std::stoi(cache_control.substr(start_pos, end_pos - start_pos));
    }
    return ans;
}
