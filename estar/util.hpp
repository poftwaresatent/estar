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


namespace estar {
  
  
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
  template<typename value_t, typename index_t = size_t>
  class array
  {
  public:
    typedef boost::scoped_array<value_t> inner_t;
    typedef boost::scoped_array<inner_t> outer_t;
  
    array(index_t xsize, index_t ysize)
      : data(new inner_t[xsize])
    {
      for (index_t ix(0); ix < xsize; ++ix)
	data[ix].reset(new value_t[ysize]);
    }
    
    array(index_t xsize, index_t ysize, const value_t & init)
      : data(new inner_t[xsize])
    {
      for (index_t ix(0); ix < xsize; ++ix) {
	data[ix].reset(new value_t[ysize]);
        for(index_t iy(0); iy < ysize; ++iy)
	  data[ix][iy] = init;
      }
    }
    
    inner_t & operator [] (index_t ix) { return data[ix]; }
    const inner_t & operator [] (index_t ix) const { return data[ix]; }
    
    outer_t data;
  };
  
}

#endif // ESTAR_UTIL_HPP
