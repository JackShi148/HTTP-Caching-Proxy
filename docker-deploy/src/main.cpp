// #include <unistd.h>

#include "proxy.h"

int main()
{
  try
  {
    Proxy p("12345");
    p.startProxy();
  }
  catch (Exception e)
  {
    std::cerr << e.what() << std::endl;
  }
  return 1;
}
