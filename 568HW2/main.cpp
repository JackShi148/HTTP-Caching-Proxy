#include "proxy.h"

int main(){
  try{
    Proxy p("172.16.222.129","8000");
    p.startProxy();
  }catch(Exception e){
    std::cerr<<e.what()<<std::endl;
  }
  return 1;
}
