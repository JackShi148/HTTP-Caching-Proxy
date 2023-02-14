#include "client.h"

#define BUFFER_UNIT 65536

void Client::initClient()
{
    memset(&this->host_info, 0, sizeof(this->host_info));

    this->host_info.ai_family = AF_UNSPEC;
    this->host_info.ai_socktype = SOCK_STREAM;
    this->host_info.ai_protocol = 0;
    // get server IP address
    this->status = getaddrinfo(this->hostname, this->port, &host_info, &host_info_list);
    if (this->status != 0)
    {
        throw Exception("Cannot get IP address for this host, please double check your host or port\n");
    }
    // construct socket for client
    this->socket_fd = socket(this->host_info_list->ai_family,
                             this->host_info_list->ai_socktype,
                             this->host_info_list->ai_protocol);
    if (this->socket_fd == -1)
    {
        throw Exception("Something wrong during constructing socket\n");
    }
    // construct connection with server
    this->status = connect(this->socket_fd,
                           this->host_info_list->ai_addr,
                           this->host_info_list->ai_addrlen);
    if (this->status == -1)
    {
        throw Exception("Cannot connect with server\n");
    }
}

void Client::clientRecv(vector<char> &msg)
{
    recvHelper(this->socket_fd, msg);
}

void Client::clientSend(vector<char> &msg)
{
    sendHelper(this->socket_fd, msg);
}

void recvHelper(int socket_fd, vector<char> &msg)
{
    ssize_t pos = 0;
    msg.resize(BUFFER_UNIT);

    int cur_msg_len = recv(socket_fd, &msg.data()[pos], BUFFER_UNIT, 0);
    validateMsg(cur_msg_len);
    pos += cur_msg_len;
    // create Respond object
    Response resp = Response(msg);
    // if the content has not been modified
    // return directly
    if (resp.getStatusCode() == "304")
    {
        return;
    }
    if (resp.isChunked())
    {
        string new_chunk;
        new_chunk.insert(new_chunk.begin(), msg.begin(), msg.end());
        while (new_chunk.find("\r\n\r\n") == string::npos)
        {
            msg.resize(pos + BUFFER_UNIT);
            cur_msg_len = recv(socket_fd, &msg.data()[pos], BUFFER_UNIT, 0);
            // cur_msg_len may be 0: all content has been sent just now
            // or may be -1: something wrong occurs
            if (cur_msg_len <= 0)
            {
                break;
            }
            new_chunk = "";
            new_chunk.insert(new_chunk.begin(), msg.begin() + pos, msg.begin() + pos + cur_msg_len);
            pos += cur_msg_len;
        }
    }
    else
    {
        int msg_total_len = resp.getContentLength();
        while (pos < msg_total_len)
        {
            // buffer size need to expand
            msg.resize(pos + BUFFER_UNIT);
            cur_msg_len = recv(socket_fd, &msg.data()[pos], BUFFER_UNIT, 0);
            if (cur_msg_len <= 0)
            {
                break;
            }
            pos += cur_msg_len;
        }
    }
}

void sendHelper(int socket_fd, vector<char> &msg)
{
    int buf_size = send(socket_fd, msg.data(), msg.size(), 0);
    if (buf_size == -1)
    {
        throw Exception("Error occurs during sending\n");
    }
}

void validateMsg(int msg_len)
{
    if (msg_len == 0)
    {
        throw Exception("The content is empty\n");
    }
    if (msg_len = -1)
    {
        throw Exception("Error occurs during receiving\n");
    }
}