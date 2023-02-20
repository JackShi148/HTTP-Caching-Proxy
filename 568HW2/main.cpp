#include "proxy.h"

int main(){
  try{
    Proxy p("67.159.88.176","8000");
    p.startProxy();
  }catch(Exception e){
    std::cerr<<e.what()<<std::endl;
  }
  return 1;
}