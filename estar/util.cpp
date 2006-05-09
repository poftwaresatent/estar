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


using std::make_pair;


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
		     std::vector<std::pair<double, double> > * trace)
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
    
    carrot_x = robot_x;
    carrot_y = robot_y;
    size_t ii;
    for(ii = 0; ii < maxsteps; ++ii){
      double gx, gy;
      const bool ok(grid.ComputeGradient(ix, iy, gx, gy));
      bool heur(false);
      if(ok){
	const double scale(stepsize / (sqrt(square(gx) + square(gy))));
	if(scale < epsilon)
	  heur = true;
	else{
	  carrot_x -= gx * scale;
	  carrot_y -= gy * scale;
	}
      }
      if(heur || ( ! ok)){
	if(gx > 0)      carrot_x -= stepsize / 2;
	else if(gx < 0) carrot_x += stepsize / 2;
	if(gy > 0)      carrot_y -= stepsize / 2;
	else if(gy < 0) carrot_y += stepsize / 2;
      }
      ix = boundval<size_t>(0, static_cast<size_t>(rint(carrot_x)), max_ix);
      iy = boundval<size_t>(0, static_cast<size_t>(rint(carrot_y)), max_iy);
      PDEBUG("(%g   %g) ==> (%g   %g)%s\n",
	     gx, gy, carrot_x, carrot_y,
	     (heur || ( ! ok)) ? "[heuristic]" : "");
      
      if(0 != trace)
	trace->push_back(make_pair(carrot_x * facade.scale,
				   carrot_y * facade.scale));
      
      if(sqrt(square(robot_x-carrot_x)+square(robot_y-carrot_y)) >= distance){
	PDEBUG("... >= distance");
	break;
      }
    }
    carrot_x *= facade.scale;
    carrot_y *= facade.scale;
    
    if(ii >= maxsteps){
      PDEBUG("WARNING (ii >= maxsteps)\n");
      return 1;
    }
    PDEBUG("success: %g   %g\n", carrot_x, carrot_y);
    return 0;
  }
  
}
