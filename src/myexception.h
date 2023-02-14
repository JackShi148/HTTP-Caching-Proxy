#ifndef __MYEXCEPTION_H__
#define __MYEXCEPTION_H__

#include <exception>
#include <stdexcept>
#include <string>

using namespace std;

class MyException : public exception {
    string msg;
public:
    MyException(const string &errMsg) : msg(errMsg) {}
    virtual const char * what() const throw() {
        return msg.c_str();
    }
};

#endif