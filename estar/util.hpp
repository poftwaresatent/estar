/* 
 * Copyright (C) 2005 Roland Philippsen <roland dot philippsen at gmx net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */


#ifndef ESTAR_UTIL_HPP
#define ESTAR_UTIL_HPP


#include <boost/scoped_array.hpp>
#include <string>
#include <vector>
#include <unistd.h>
#include <stdio.h>


#define PDEBUG_ERR(fmt, arg...) fprintf(stderr, "%s(): "fmt, __func__, ## arg)
#define PDEBUG_OUT(fmt, arg...) fprintf(stdout, "%s(): "fmt, __func__, ## arg)
#define PDEBUG_OFF(fmt, arg...)
#undef  PDEBUG


namespace estar {
  
  
  class Facade;
  class Algorithm;
  class Grid;
  

  /** Utility for compile-time switchable debug messages. Can be used
      like a std::ostream, but doesn't do anything except return an
      empty string if requested to do so. Mimics std::ostringstream
      actually. */
  class fake_os {
  public:
    template<class Foo>
    fake_os & operator << (const Foo &) { return * this; }
    std::string str() const { return std::string(); }
  };


  /**
     Set the function for cleaning up after your program. Also sets up
     signal handlers for SIGINT, SIGHUP, and SIGTERM to call that ceanup
     function.
   
     \note If you use this function, you should put your cleanup all
     into the function passed as argument and NOT call that function
     yourself. It is automatically called upon calls to exit() or return
     from main.
  */
  void set_cleanup(void (*function)());


  /**
     Simple 2D-array with "self destroying" underlying data.
   
     \note If you want to put this into an STL container, wrap it into a
     boost::shared_ptr to avoid problems with the non-copyable
     boost::scoped_array fields.
  */
  template<typename T>
  class array
  {
  public:
    typedef boost::scoped_array<T> inner_t;
    typedef boost::scoped_array<inner_t> outer_t;
  
    array(size_t xsize, size_t ysize): data(new inner_t[xsize])
    { for(size_t ix(0); ix < xsize; ++ix) data[ix].reset(new T[ysize]); }
  
    array(size_t xsize, size_t ysize, const T & init): data(new inner_t[xsize])
    { for(size_t ix(0); ix < xsize; ++ix)
      { data[ix].reset(new T[ysize]);
        for(size_t iy(0); iy < ysize; ++iy) data[ix][iy] = init; } }
    
    inner_t & operator [] (size_t ix) { return data[ix]; }
    const inner_t & operator [] (size_t ix) const { return data[ix]; }
  
    outer_t data;
  };
  
  
  /**
     \return
       0 on success<br>
       1 if distance wasn't reached after maxstep iterations
      -1 if the robot is outside the grid<br>
      -2 if the grid is a hexgrid (not implemented yet)
  */
  int compute_carrot(const Facade & facade,
		     double robot_x, double robot_y,
		     double distance, double stepsize,
		     size_t maxsteps,
		     double & carrot_x, double & carrot_y,
		     std::vector<std::pair<double, double> > * trace);
  
}

#endif // ESTAR_UTIL_HPP
