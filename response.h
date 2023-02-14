#ifndef __RESPONSE_H__
#define __RESPONSE_H__
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Response {
    string response_msg;
    string formatFinder(string field);
public:
    Response() {}
    Response(vector<char> msg);
    Response(string response_msg) : response_msg(response_msg) {}
    bool isChunked();
    int getContentLength();
    string getStatusCode();
};

#endif