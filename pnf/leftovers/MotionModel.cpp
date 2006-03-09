/* 
 * Copyright (C) 2004
 * Swiss Federal Institute of Technology, Lausanne. All rights reserved.
 * 
 * Author: Roland Philippsen <roland dot philippsen at gmx net>
 *         Autonomous Systems Lab <http://asl.epfl.ch/>
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


#include "MotionModel.hpp"
#include <estar/numeric.hpp>


namespace pnf {
  
  
  /**
     \note Formulae based on Matlab-code developed by Bjoern Jensen at
     the Autonomous Systems Lab in 2003
  */
  double MotionModel::
  Compute(double object_distance,
	  double robot_distance,
	  double object_speed,
	  double robot_speed,
	  double resolution)
    const
  {
    if(absval(object_speed) < epsilon){
      if(absval(object_distance) < resolution)
	return 1;
      return 0;
    }
    
    robot_distance = absval(robot_distance);
    
    const double robot_time(robot_distance / robot_speed);
    const double cell_traversal_time(resolution / robot_speed);
    const double n_steps(ceil(robot_time / cell_traversal_time));
    
    // osb = "object speed bound"
    // os = "object speed"
    // sq = "squared"
    const double osb1((object_distance - resolution / 2) / robot_time);
    const double osb2((object_distance + resolution / 2) / robot_time);
    const double corr((n_steps - 1) / n_steps);
    const double osb_sq1(sqr(osb1));
    const double osb_sq2(sqr(osb2));
    const double os_sq(sqr(object_speed));
    
    double probability(0);
    
    // left
    if((osb1 < - object_speed)
       && ( - object_speed < osb2)
       && (osb2 < 0)){
      const double pleft((osb1 + object_speed) / object_speed
			 + corr * (osb_sq1 - os_sq) / (2 * os_sq));
      probability += pleft;
    }
    
    // bothleft
    if(( - object_speed <= osb1)
       && (osb1 < 0)
       && ( - object_speed <= osb2)
       && (osb2 < 0)){
      const double pbothleft((osb2 - osb1) / object_speed
			     + corr * (osb_sq2 - osb_sq1) / (2 * os_sq));
      probability += pbothleft;
    }
    
    // middle
    if(( - object_speed <= osb1)
       && (osb1 < 0)
       && (0 <= osb2)
       && (osb2 < object_speed)){
      const double pmiddle((osb2 - osb1) / object_speed
			   - corr * (osb_sq2 + osb_sq1) / (2 * os_sq));
      probability += pmiddle;
    }
    
    // bothright
    if((0 <= osb1)
       && (osb1 < object_speed)
       && (0 <= osb2)
       && (osb2 < object_speed)){
      const double pbothright((osb2 - osb1) / object_speed
			      - corr * (osb_sq2 - osb_sq1) / (2 * os_sq));
      probability += pbothright;
    }
    
    // right
    if((0 <= osb1)
       && (osb1 < object_speed)
       && (object_speed <= osb2)){
      const double pright((object_speed - osb2) / object_speed
			  - corr * (os_sq - osb_sq2) / (2 * os_sq));
      probability += pright;
    }
    
    return probability;
  }
  
}
