// compile with g++ -g -O0 -Wall -I.. -o test_flexgrid test_flexgrid.cpp

#include "../estar/flexgrid.hpp"
#include <cstdio>
#include <iostream>
#include <sstream>


using namespace estar;
using namespace std;


typedef sdeque<int> sd_t;
typedef flexgrid<int> fg_t;


void dump(sd_t const & sd)
{
  printf("index|");
  for (ssize_t ii(sd.ibegin()); ii < sd.iend(); ++ii)
    printf(" %+4ld |", ii);
  printf("\n"
	 "value|");
  for (sd_t::const_iterator ii(sd.begin()); ii != sd.end(); ++ii)
    printf(" %+4d |", *ii);
  printf("\n"
	 "check|");
  for (ssize_t ii(sd.ibegin()); ii < sd.iend(); ++ii)
    printf(" %+4d |", sd.at(ii));
  printf("\n");
}


void dump(fg_t const & fg)
{
  ostringstream sep;
  sep << "\n";
  for (ssize_t ix(fg.xbegin()); ix <= fg.xend(); ++ix)
    sep << "----+";
  sep << "\n";
  printf("    |");
  for (ssize_t ix(fg.xbegin()); ix < fg.xend(); ++ix)
    printf("%+3ld |", ix);
  cout << sep.str();
  for (ssize_t iy(fg.yend() - 1); iy >= fg.ybegin(); --iy) {
    printf("%+3ld |", iy);
    for (ssize_t ix(fg.xbegin()); ix < fg.xend(); ++ix)
      printf("%+3d |", fg.at(ix, iy));
    cout << sep.str();
  }
}


void tsd()
{
  sd_t foo;
  cout << "after creation:\n";
  dump(foo);
  foo.resize_end(3, 5);
  cout << "\nafter resize_end(3, 5)\n";
  dump(foo);
  foo.resize_begin(-3, 17);
  cout << "\nafter resize_begin(-3, 17)\n";
  dump(foo);
  foo.resize_end(-1, 99);
  cout << "\nafter resize_end(-1, 99)\n";
  dump(foo);
  foo.resize(-5, 5);
  cout << "\nafter resize(-5, 5)\n";
  dump(foo);
  for (ssize_t ii(foo.ibegin()); ii < foo.iend(); ++ii)
    foo.at(ii) = 42 + ii;
  cout << "\nafter setting values\n";
  dump(foo);
  try {
    foo.at(-1000) = 30;
    cout << "BUG: at(-1000) should have thrown\n";
  }
  catch (std::out_of_range) {
    ////    cout << "at(-1000) threw like it should\n";
  }
  try {
    foo.at(1000) = 30;
    cout << "BUG: at(1000) should have thrown\n";
  }
  catch (std::out_of_range) {
    ////    cout << "at(1000) threw like it should\n";
  }
  try {
    foo.resize_end(-6);
    cout << "BUG: resize_end(-6) should have thrown\n";
  }
  catch (std::out_of_range) {
    ////    cout << "resize_end(-6) threw like it should\n";
    sd_t bar(foo);
    try {
      bar.resize_end(-5);
      cout << "OK: resize_end(-5) didn't throw\n";
      dump(bar);
    }
    catch (std::out_of_range) {
      cout << "BUG: resize_end(-5) shouldn't have thrown\n";
    }
  }
  try {
    foo.resize_begin(6);
    cout << "BUG: resize_begin(6) should have thrown\n";
  }
  catch (std::out_of_range) {
    ////    cout << "resize_begin(6) threw like it should\n";
    sd_t bar(foo);
    try {
      bar.resize_begin(5);
      cout << "OK: resize_begin(5) didn't throw\n";
      dump(bar);
    }
    catch (std::out_of_range) {
      cout << "BUG: resize_begin(5) shouldn't have thrown\n";
    }
  }
}


void tfg1()
{
  fg_t fg;
  cout << "after creation:\n";
  dump(fg);
  fg.resize_x(-1, 1);
  cout << "after resize_x(-1, 1):\n";
  dump(fg);
  fg.resize_y(-1, 1);
  cout << "after resize_y(-1, 1):\n";
  dump(fg);
  fg.resize_xend(3, 42);
  cout << "after resize_xend(3, 42):\n";
  dump(fg);
  fg.resize_ybegin(-3, 17);
  cout << "after resize_ybegin(-3, 17):\n";
  dump(fg);
  for (ssize_t iy(-3); iy < 1; ++iy)
    fg.at(0, iy) = -7 + 2 * iy;
  cout << "after assigning stuff to column 0:\n";
  dump(fg);
  fg.resize(-3, +2, -2, 2, 99);
  cout << "after resize(-3, +2, -2, 2, 99):\n";
  dump(fg);
  fg.resize(-5, 5, -5, 5);
  cout << "after resize(-5, +5, -5, 5):\n";
  dump(fg);
}


int main(int argc, char ** argv)
{
  fg_t fg;
  try {
    cout << fg.at(12, 34);
  }
  catch (std::out_of_range) {
    cout << "yep, caught it\n";
  }
  fg.smart_at(-2,  1) =  42;
  fg.smart_at(-1,  2) =  98;
  fg.smart_at( 5, -2) = -17;
  fg.smart_at( 0, -3) =  42;
  dump(fg);
}
