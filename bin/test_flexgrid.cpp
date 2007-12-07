// compile with g++ -g -O0 -Wall -I.. -o test_flexgrid test_flexgrid.cpp

#include <estar/flexgrid.hpp>
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


void tfg2()
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


template<typename FGT, typename ITT>
void tfg_it(FGT & bar)
{
  cout << "  fwd ++ii style:\n   <";
  try {
    for (ITT ii(bar.begin()); ii != bar.end(); ++ii)
      cout << "  " << *ii;
  }
  catch (std::out_of_range) {
    cout << "[out_of_range]";
  }
  
  cout << " >\n  fwd ii++ style:\n   <";
  try {
    for (ITT ii(bar.begin()); ii != bar.end(); ii++)
      cout << "  " << *ii;
  }
  catch (std::out_of_range) {
    cout << "[out_of_range]";
  }
  
  cout << " >\n  bwd --ii style:\n   <";
  try {
    ITT ii(bar.end());
    if (ii != bar.begin())
      do {
	--ii;
	cout << "  " << *ii;
      } while (ii != bar.begin());
  }
  catch (std::out_of_range) {
    cout << "[out_of_range]";
  }
  
  cout << " >\n  bwd ii-- style:\n   <";
  try {
    ITT ii(bar.end());
    if (ii != bar.begin())
      do {
	--ii;
	cout << "  " << *ii;
      } while (ii != bar.begin());
  }
  catch (std::out_of_range) {
    cout << "[out_of_range]";
  }
  
  cout << " >\n";
}


void tfg_it_helper(fg_t & bar)
{
  dump(bar);
  cout << "non-const iterator:\n";
  tfg_it<fg_t, fg_t::iterator>(bar);
  cout << "const iterator from non-const grid:\n";
  tfg_it<fg_t, fg_t::const_iterator>(bar);
  cout << "const iterator from const grid:\n";
  tfg_it<const fg_t, fg_t::const_iterator>(bar);
  
  cout << "alternatively, using fg_t::line_begin() etc:\n";
  for (fg_t::const_line_iterator il(bar.line_begin());
       il != bar.line_end(); ++il) {
    cout << "  <";
    for (fg_t::const_cell_iterator ic(il->begin()); ic != il->end(); ++ic)
      cout << "  " << *ic;
    cout << " >\n";
  }
}


int main(int argc, char ** argv)
{
  {
    fg_t foo;
    fg_t const & bar(foo);
    {
      fg_t::iterator blah(foo.begin());
    }
    {
      fg_t::const_iterator blah(foo.begin());
    }
    {
      fg_t::const_iterator blah(bar.begin());
    }
  }
  
  cout << "\ntrying to decrement fg.begin(): ";
  {
    fg_t fg;
    fg_t::iterator ii(fg.begin());
    --ii;
    if (ii != fg.begin())
      cout << "ERROR\n";
    else
      cout << "OK\n";
  }
  
  cout << "\ntrying to increment fg.end(): ";
  {
    fg_t fg;
    fg_t::iterator ii(fg.end());
    ++ii;
    if (ii != fg.end())
      cout << "ERROR\n";
    else
      cout << "OK\n";
  }
  
  {
    cout << "\n==================================================\n"
	 << "normal:\n";
    fg_t fg;
    fg.resize(-2, 2, -1, 2);
    {
      int ii(0);
      for (ssize_t ix(-2); ix < 2; ++ix)
	for (ssize_t iy(-1); iy < 2; ++iy)
	  fg.at(ix, iy) = ++ii;
    }
    tfg_it_helper(fg);
  }
  
  {
    cout << "\n==================================================\n"
	 << "empty:\n";
    fg_t fg;
    tfg_it_helper(fg);
  }
  
  {
    cout << "\n==================================================\n"
	 << "empty X-range:\n";
    fg_t fg;
    fg.resize(0, 0, -2, 2);
    tfg_it_helper(fg);
  }
  
  {
    cout << "\n==================================================\n"
	 << "empty Y-range:\n";
    fg_t fg;
    fg.resize(-2, 2, 0, 0);
    tfg_it_helper(fg);
  }  
}
