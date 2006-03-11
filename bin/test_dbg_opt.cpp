// compile with g++ -Wall -I.. -o test_dbg_opt test_dbg_opt.cpp

#include <estar/util.hpp>
#include <iostream>

#undef DEBUG
#ifdef DEBUG
# define PDEBUG PDEBUG_OUT
#else
# define PDEBUG PDEBUG_OFF
#endif

static int side_effect(int foo)
{
#ifndef DEBUG
  PDEBUG_OUT("Dang, side_effect() called even though DEBUG not defined!\n");
#endif // DEBUG
  return foo;
}

int main(int argc, char ** argv)
{
  PDEBUG("Debug message %d\n", side_effect(42));
  std::cout << "Hello world!\n";
}
