// g++ -Wall -I.. -o test_fake_os test_fake_os.cpp

#include <estar/util.hpp>

#undef DEBUG
#ifdef DEBUG
# include <sstream>
using std::ostringstream;
# define PDEBUG PDEBUG_OUT
#else
# define PDEBUG PDEBUG_OFF
#endif

int main(int argc, char ** argv)
{
#ifdef DEBUG
  ostringstream dbg;
#else // DEBUG
  estar::fake_os dbg;
#endif // DEBUG
  
  dbg << "a debug message with number " << 42 << " and some blah";
  PDEBUG("here comes the debug string \"%s\"\n", dbg.str().c_str());
}
