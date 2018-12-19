#include <iostream>

#include "opengl3_base.h"
#include "test_app.h"

int main(int argc, char * argv[]) {
  try
  {
    vktuto::TestApp test_app;
    test_app.Run();
  }
  catch (const std::exception& except)
  {
    std::cerr << "[Error] catched: "
              << except.what() << std::endl; 
  }

  return EXIT_SUCCESS;
}