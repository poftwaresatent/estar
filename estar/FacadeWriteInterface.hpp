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


#include <estar/GridNode.hpp>
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
       
       \return true if the index was valid.
    */
    virtual bool SetMeta(ssize_t ix, ssize_t iy, double meta) = 0;
    
    /**
       Declare a cell to be a goal, and define the value of the
       navigation function at that point. This method will queue the
       cell's neighbors for expansion. Using smoothly varying a-priori
       values in a goal region can significantly improve the quality
       of interpolation.
       
       \note Call RemoveAllGoals() first if you want to move the goal.
       
       \return true if the index was valid.
    */
    virtual bool AddGoal(ssize_t ix, ssize_t iy, double value) = 0;
    
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
    
    /**
       Makes sure that the required range of indices is available,
       where (xend, yend) is the ONE-PAST-end marker for the X- and Y-
       directions. That is to say, after this call there is guaranteed
       to be a GridNode at [xend-1, yend-1] but not beyond that. The
       method does not modify any prior existing nodes in the given
       range of indices, except for adding neighborhood edges where
       appropriate.
       
       \return The number of added vertices.
    */
    virtual size_t AddRange(ssize_t xbegin, ssize_t xend,
			    ssize_t ybegin, ssize_t yend,
			    double meta) = 0;
    
    /**
       Ignore if (ix, iy) is already in the grid. Otherwise, add a new
       GridNode instance and "hook it" into the E* structures.
       
       \return true if a new GridNode was allocated and inserted into
       the grid.
    */
    virtual bool AddNode(ssize_t ix, ssize_t iy, double meta) = 0;
  };
  
} // namespace estar

#endif // ESTAR_FACADE_WRITE_INTERFACE_HPP
