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
#include <vector>
//#include <unistd.h>


namespace estar {
  
  
  class FacadeReadInterface;
  class Algorithm;
  class Grid;


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
     This is now implemented as a wrapper around trace_carrot().
     
     \note The carrot is computed in the grid frame of reference.
     
     \return
       0 on success<br>
       1 if distance wasn't reached after maxstep iterations
      -1 if the robot is outside the grid<br>
      -2 if the grid is a hexgrid (not implemented yet)
      -3 trace_carrot() succeeded but the trace is emtpy (probably a bug)
  */
  int compute_carrot(const FacadeReadInterface & facade,
		     double robot_x, double robot_y,
		     double distance, double stepsize,
		     size_t maxsteps,
		     double & carrot_x, double & carrot_y,
		     std::vector<std::pair<double, double> > * trace);
  
  
  /**
     An element of the 'trace' of the steepest gradient from a given
     start position, used (pcked into a vector) as out-parameter for
     trace_carrot().
  */
  struct carrot_item {
    carrot_item(double x, double y, double gx, double gy, double val, bool dgn)
      : cx(x), cy(y), gradx(gx), grady(gy), value(val), degenerate(dgn) {}
    double cx;			/**< carrot x-coordinate */
    double cy;			/**< carrot y-coordinate */
    double gradx;		/**< gradient at carrot, x-component */
    double grady;		/**< gradient at carrot, y-component */
    double value;		/**< navigation function value */
    bool degenerate;		/**< degenerate gradient (used heuristic) */
  };
  
  typedef std::vector<carrot_item> carrot_trace;
  
  
  /**
     Like (the original) compute_carrot(), but leaves a more richly
     informed trace.
     
     \note The trace is computed in the grid frame of reference.
     
     \return
       0 on success<br>
       1 if distance wasn't reached after maxstep iterations
      -1 if the robot is outside the grid<br>
      -2 if the grid is a hexgrid (not implemented yet)
      
      \todo URGENT: Need ridge detection (and avoidance) to avoid
      going straight toward obstacle in case the robot is on the ridge
      where two wavefronts meet after they swept around an
      obstacle. In such a case, one direction should be chosen at
      "random"!
  */
  int trace_carrot(const FacadeReadInterface & facade,
		   double robot_x, double robot_y,
		   double distance, double stepsize,
		   size_t maxsteps,
		   carrot_trace & trace);
  
}

#endif // ESTAR_UTIL_HPP
