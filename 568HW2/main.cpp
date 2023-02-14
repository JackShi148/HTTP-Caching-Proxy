#include "proxy.h"

int main(){
  try{
    Proxy p("0.0.0.0","8000");
    p.startProxy();
  }catch(std::exception e){
    std::cerr<<e.what()<<std::endl;
  }
  return 1;
}
