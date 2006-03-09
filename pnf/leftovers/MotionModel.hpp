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


#ifndef MOTION_MODEL_HPP
#define MOTION_MODEL_HPP


namespace pnf {
  
  
  /**
     \todo ...total overkill virtuality...
  */
  class MotionModel
  {
  public:
    virtual ~MotionModel() { }
    virtual double Compute(double object_distance,
			   double robot_distance,
			   double object_speed,
			   double robot_speed,
			   double resolution) const;
  };
  
}

#endif // MOTION_MODEL_HPP
