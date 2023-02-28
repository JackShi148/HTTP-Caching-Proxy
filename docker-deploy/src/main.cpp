#include <unistd.h>

#include "proxy.h"

int main()
{
  try
  {
    if(daemon(1, 0) != 0) {
      throw Exception("Error: fail to init as daemon");
    }
    Proxy p("12345");
    p.startProxy();
  }
  catch (Exception e)
  {
    std::cerr << e.what() << std::endl;
  }
  return EXIT_FAILURE;
}
