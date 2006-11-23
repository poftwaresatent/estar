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


#include "util.hpp"
#include "Facade.hpp"
#include "Grid.hpp"
#include <signal.h>
#include <stdlib.h>


#ifdef DEBUG
# define ESTAR_UTIL_DEBUG
#else
# undef ESTAR_UTIL_DEBUG
#endif

#ifdef ESTAR_UTIL_DEBUG
# define PDEBUG PDEBUG_OUT
#else
# define PDEBUG PDEBUG_OFF
#endif


using namespace std;


namespace estar {


  static void handle(int signum)
  {
    // The cleanup function is called implcitly through exit().
    exit(EXIT_SUCCESS);
  }


  void set_cleanup(void (*function)())
  {
    if(atexit(function)){
      perror("set_cleanup(): atexit() failed");
      exit(EXIT_FAILURE);
    }
    if(signal(SIGINT, handle) == SIG_ERR){
      perror("set_cleanup(): signal(SIGINT) failed");
      exit(EXIT_FAILURE);
    }
    if(signal(SIGHUP, handle) == SIG_ERR){
      perror("set_cleanup(): signal(SIGHUP) failed");
      exit(EXIT_FAILURE);
    }
    if(signal(SIGTERM, handle) == SIG_ERR){
      perror("set_cleanup(): signal(SIGTERM) failed");
      exit(EXIT_FAILURE);
    }
  }


  int compute_carrot(const Facade & facade,
		     double robot_x, double robot_y,
		     double distance, double stepsize,
		     size_t maxsteps,
		     double & carrot_x, double & carrot_y,
		     vector<pair<double, double> > * trace)
  {
    carrot_trace gtrace;
    const int res(trace_carrot(facade, robot_x, robot_y, distance, stepsize,
			       maxsteps, gtrace));
    if(0 > res)
      return res;
    if(gtrace.empty())
      return -3;
    carrot_x = gtrace.back().cx;
    carrot_y = gtrace.back().cy;
    if(0 == trace)
      return res;
    trace->clear();
    for(size_t ii(0); ii < gtrace.size(); ++ii)
      trace->push_back(make_pair(gtrace[ii].cx, gtrace[ii].cy));
    return res;
  }
  
  
  // maybe one day make it accessible from outside... but some
  // unstated preconditions!
  static int compute_stable_scaled_gradient(const Grid & grid,
					    size_t ix, size_t iy,
					    double stepsize,
					    /** only valid if return 0 */
					    double & gx,
					    /** only valid if return 0 */
					    double & gy,
					    /** always valid */
					    double & dx,
					    /** always valid */
					    double & dy)
  {
    dx = 0;
    dy = 0;
    gx = 0;			// redundant with Grid::ComputeGradient()
    gy = 0;			// but prudent about future changes
    const bool ok(grid.ComputeGradient(ix, iy, gx, gy));
    bool heur(false);
    if(ok){
      const double alpha(stepsize / (sqrt(square(gx) + square(gy))));
      if(alpha < epsilon)
	heur = true;
      else{
	dx = gx * alpha;
	dy = gy * alpha;
      }
    }
    if(heur || ( ! ok)){
      if(gx > 0)      dx =   stepsize / 2;
      else if(gx < 0) dx = - stepsize / 2;
      if(gy > 0)      dy =   stepsize / 2;
      else if(gy < 0) dy = - stepsize / 2;
    }
    if( ! ok)
      return 1;
    if(heur)
      return 2;
    return 0;
  }
  
  
  int trace_carrot(const Facade & facade,
		   double robot_x, double robot_y,
		   double distance, double stepsize,
		   size_t maxsteps,
		   carrot_trace & trace)
  {
    PDEBUG("(%g   %g)   d: %g   s: %g   N: %lu\n",
	   robot_x, robot_y, distance, stepsize, maxsteps);
    if((robot_x < 0) || (robot_y < 0)){
      PDEBUG_ERR("FAIL (robot_x < 0) || (robot_y < 0)\n");
      return -1;
    }
    robot_x /= facade.scale;
    robot_y /= facade.scale;
    distance /= facade.scale;
    stepsize /= facade.scale;
    PDEBUG("scaled: (%g   %g)   d: %g   s: %g\n",
	   robot_x, robot_y, distance, stepsize);
    size_t ix(static_cast<size_t>(rint(robot_x)));
    size_t iy(static_cast<size_t>(rint(robot_y)));
    if((ix >= facade.xsize) || (iy >= facade.ysize)){
      PDEBUG("FAIL (ix >= facade.xsize) || (iy >= facade.ysize)\n");
      return -1;
    }
    
    const Grid & grid(facade.GetGrid());
    if((grid.connect != FOUR_CONNECTED)
       && (grid.connect != EIGHT_CONNECTED)){
      PDEBUG_OUT("TODO: Implement carrot for hexgrids!\n");
      return -2;
    }
    const size_t max_ix(grid.GetXSize() - 1);
    const size_t max_iy(grid.GetYSize() - 1);
    
    trace.clear();
    double cx(robot_x);		// carrot
    double cy(robot_y);
    size_t ii;
    for(ii = 0; ii < maxsteps; ++ii){
      const double value(facade.GetValue(ix, iy));
      double dx, dy, gx, gy;
      const int res(compute_stable_scaled_gradient(grid, ix, iy, stepsize,
						   gx, gy, dx, dy));
      if(0 == res)
	trace.push_back(carrot_item(cx * facade.scale,
				    cy * facade.scale,
				    gx / facade.scale,
				    gy / facade.scale,
				    value,
				    false));
      else
	trace.push_back(carrot_item(cx * facade.scale,
				    cy * facade.scale,
				    dx / stepsize,
				    dy / stepsize,
				    value,
				    true));
      cx -= dx;
      cy -= dy;
      PDEBUG("(%g   %g) ==> (%g   %g)%s\n",
	     dx, dy, cx, cy, (heur || ( ! ok)) ? "[heuristic]" : "");
      
      if(sqrt(square(robot_x - cx) + square(robot_y - cy)) >= distance){
	PDEBUG("... >= distance");
	break;
      }
      if(value <= stepsize){
	PDEBUG("... value <= distance");
	break;
      }
      
      ix = boundval<size_t>(0, static_cast<size_t>(rint(cx)), max_ix);
      iy = boundval<size_t>(0, static_cast<size_t>(rint(cy)), max_iy);
    }
    // add final point to the trace
    {
      double dx, dy, gx, gy;
      const int res(compute_stable_scaled_gradient(grid, ix, iy, stepsize,
						   gx, gy, dx, dy));
      if(0 == res)
	trace.push_back(carrot_item(cx * facade.scale,
				    cy * facade.scale,
				    gx / facade.scale,
				    gy / facade.scale,
				    facade.GetValue(ix, iy),
				    false));
      else
	trace.push_back(carrot_item(cx * facade.scale,
				    cy * facade.scale,
				    dx / stepsize,
				    dy / stepsize,
				    facade.GetValue(ix, iy),
				    true));
    }
    
    if(ii >= maxsteps){
      PDEBUG("WARNING (ii >= maxsteps)\n");
      return 1;
    }
    PDEBUG("success: %g   %g\n", cx * facade.scale, cy * facade.scale);
    return 0;
  }
  
}
