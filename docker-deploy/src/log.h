#ifndef __LOG_H__
#define __LOG_H__
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <thread>

#include "exception.h"
#include "hook.h"

class Log
{
private:
  std::fstream logfile;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  // std::mutex thread_mutex;
public:
  ~Log()
  {
    closeLogFile();
  }
  void openLogFile(std::string filepath);
  void writeLogFile(Hook *h, std::string msg, int option);
  void writeCacheLog(Hook *h, std::string msg, int option);
  std::string getCurrTime();
  void closeLogFile();
};

#endif
