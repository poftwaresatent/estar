// g++ -Wall -I.. -o test_fake_os test_fake_os.cpp

#include "../estar/util.hpp"
#include "../estar/pdebug.hpp"

int main(int argc, char ** argv)
{
  estar::debugos dbg;
  
  dbg << "a debug message with number " << 42 << " and some blah";
  PDEBUG("here comes the debug string \"%s\"\n", dbg.str().c_str());
}
