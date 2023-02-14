#include "response.h"

Response::Response(vector<char> msg) {
    this->response_msg.insert(response_msg.begin(), msg.begin(), msg.end());
}

string Response::formatFinder(string field) {
    size_t start_pos = this->response_msg.find(field);
    if(start_pos == string::npos) {
        return "";
    }
    size_t end_pos = this->response_msg.find("\r\n");
    // skip the field name, colon and whitespace
    size_t field_len = field.size();
    start_pos += field_len + 2;
    string ans = this->response_msg.substr(start_pos, end_pos - start_pos);
    return ans;
}

bool Response::isChunked() {
    string encode_info = formatFinder("Transfer-Encoding");
    if(encode_info.find("chunked") != string::npos) {
        return true;
    }
    return false;
}

int Response::getContentLength() {
    string content_len = formatFinder("Content-Length");
    if(content_len == "") {
        return -1;
    }
    int ans = stoi(content_len);
    return ans;
}

string Response::getStatusCode() {
    int pos1 = this->response_msg.find(' ');
    int pos2 = this->response_msg.find(' ', pos1 + 1);
    string code = this->response_msg.substr(pos1 + 1, pos2 - pos1 - 1);
    return code;
}