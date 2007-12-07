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
#include "FacadeReadInterface.hpp"
#include "Grid.hpp"
#include <signal.h>
#include <stdlib.h>


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


  int compute_carrot(const FacadeReadInterface & facade,
		     double robot_x, double robot_y,
		     double distance, double stepsize,
		     size_t maxsteps,
		     double & carrot_x, double & carrot_y,
		     vector<pair<double, double> > * trace)
  {
    carrot_trace gtrace;
    const int res(facade.TraceCarrot(robot_x, robot_y, distance, stepsize,
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
  
  
  int trace_carrot(const FacadeReadInterface & facade,
		   double robot_x, double robot_y,
		   double distance, double stepsize,
		   size_t maxsteps,
		   carrot_trace & trace)
  {
    return facade.TraceCarrot(robot_x, robot_y, distance,
			      stepsize, maxsteps, trace);
  }
  
}
