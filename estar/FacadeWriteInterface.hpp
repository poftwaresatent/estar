/* 
 * Copyright (C) 2007 Roland Philippsen <roland dot philippsen at gmx net>
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


#ifndef ESTAR_FACADE_WRITE_INTERFACE_HPP
#define ESTAR_FACADE_WRITE_INTERFACE_HPP


#include <stddef.h>


namespace estar {
  
  
  /**
     Abstraction of operations that "write" to a Facade. We split this
     from "read" operations in order to implement comparing repaired
     navigation functions with entirely replanned ones. See also class
     FacadeReadInterface.
  */
  class FacadeWriteInterface
  {
  public:
    virtual ~FacadeWriteInterface() {}
    
    /**
       Set the kernel-dependent "meta" of a cell, which represents its
       traversability or collision risk. See
       FacadeReadInterface::GetMeta() about the interpretation of meta
       values.
    */
    virtual void SetMeta(size_t ix, size_t iy, double meta) = 0;
    
    /**
       Initialize the kernel-dependent "meta" of a cell. Unlike
       SetMeta(), this method does not trigger replanning and should
       thus only be used during setup. See
       FacadeReadInterface::GetMeta() about the interpretation of meta
       values.
       
       \note DO NOT use for normal operation! Only appropriate during
       initialization. Use SetMeta() when changing the environment
       information.
    */
    virtual void InitMeta(size_t ix, size_t iy, double meta) = 0;
    
    /**
       Declare a cell to be a goal, and define the value of the
       navigation function at that point. This method will queue the
       cell's neighbors for expansion. Using smoothly varying a-priori
       values in a goal region can significantly improve the quality
       of interpolation.
       
       \note Call RemoveAllGoals() first if you want to move the goal.
    */
    virtual void AddGoal(size_t ix, size_t iy, double value) = 0;
    
    /**
       Revert all goal cells to normal status. Useful if you just want
       to completely change the goal, which is the typical case. Call
       AddGoal() to set the new goal.
    */
    virtual void RemoveAllGoals() = 0;
    
    /**
       Compute one step of wavefront propagation. If there are no
       cells on the queue, then this method does nothing.
    */
    virtual void ComputeOne() = 0;
    
    /**
       Reset the algorithm, leaving only goal- and meta- information
       intact and re-initializing the wavefront queue to the set of
       goal nodes.
    */
    virtual void Reset() = 0;
  };
  
} // namespace estar

#endif // ESTAR_FACADE_WRITE_INTERFACE_HPP
