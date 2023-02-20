#include "log.h"

void Log::openLogFile(std::string filepath){
  logfile.open(filepath, std::ostream::out);
  if(!logfile.is_open()){
    throw Exception("Error: cannot open the log file: ", filepath);
  }
}

void Log::writeLogFile(Hook * h, std::string msg, int option){
  pthread_mutex_lock(&mutex);
  if(option == -1){
    logfile << h->getThreadID() << ": WARNING " << msg << " from " << h->getClientIPAddr()
    << " @ " << getCurrTime() << std::endl;
  }else if(option == 0){
    logfile << h->getThreadID() << ": " << msg << std::endl;
  }else if(option == 1){
    logfile << h->getThreadID() << ": \"" << msg <<"\" from " << h->getClientIPAddr()
    << " @ " << getCurrTime() << std::endl;
  }else if(option == 2){
    logfile << h->getThreadID() << ": " << "Requesting \"" << msg << "\" from " 
    << h->getReqHostName() << std::endl;
  }else if(option == 3){
    logfile << h->getThreadID() << ": Responding \"" << msg << "\"" << std::endl;
  }else if(option == 4){
    logfile << h->getThreadID() << ": " << "Received \"" << msg << "\" from " 
    << h->getReqHostName() << std::endl;
  }
  pthread_mutex_unlock(&mutex);
}

void Log::writeCacheResult(Hook * h, std::string msg, int option){
  pthread_mutex_lock(&mutex);
  if(option == 10){
    logfile << h->getThreadID() << ": " << msg << std::endl;
  }else if(option == 11){
    logfile << h->getThreadID() << ": in cache, but expired at " << msg << std::endl;
  }
  pthread_mutex_unlock(&mutex);
}

std::string Log::getCurrTime(){
  time_t curr_time = time(0);
  struct tm * now_time = gmtime(&curr_time);
  const char * time = asctime(now_time);
  return std::string(time);
}

void Log::closeLogFile(){
  logfile.close();
}

