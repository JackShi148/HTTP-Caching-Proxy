#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__
#include <exception>
#include <stdexcept>
#include <string>

class Exception : public std::exception
{
public:
  std::string error_message;
  Exception()
  {
    error_message.append("Error occurs.");
  }
  Exception(const std::string &errMsg)
  {
    error_message.append(errMsg);
  }
  Exception(const std::string &errMsg1, const std::string &errMsg2)
  {
    error_message.append(errMsg1);
    error_message.append(" ");
    error_message.append(errMsg2);
    error_message.append(".");
  }
  virtual const char *what() const throw()
  {
    return error_message.c_str();
  }
};

#endif
