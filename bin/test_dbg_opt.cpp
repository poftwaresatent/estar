// compile with g++ -Wall -I.. -o test_dbg_opt test_dbg_opt.cpp

#include "../estar/util.hpp"
#include "../estar/pdebug.hpp"
#include <iostream>

static int side_effect(int foo)
{
#ifndef ESTAR_DEBUG
  PDEBUG_OUT("Dang, side_effect() even though ESTAR_DEBUG not defined!\n");
#endif // ESTAR_DEBUG
  return foo;
}

int main(int argc, char ** argv)
{
  PDEBUG("Debug message %d\n", side_effect(42));
  std::cout << "Hello world!\n";
}
