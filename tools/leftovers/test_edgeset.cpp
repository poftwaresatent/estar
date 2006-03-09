// g++ -Wall -I.. -o test_edgeset test_edgeset.cpp

#include <estar/base.hpp>

int main(int argc, char ** argv)
{
  estar::upwind_t m_upwind;
  if(m_upwind.m_edges.empty())
    return 0;
  return 42;
}
