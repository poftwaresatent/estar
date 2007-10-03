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


#ifndef ESTAR_FACADE_READ_INTERFACE_HPP
#define ESTAR_FACADE_READ_INTERFACE_HPP


#include <stddef.h>


namespace estar {
  
  
  class Algorithm;
  class Grid;
  class Kernel;
  
  
  /**
     Abstraction of operations that "read" from a Facade. We split
     this from "write" operations in order to implement comparing
     repaired navigation functions with entirely replanned ones. See
     also class FacadeWriteInterface.
  */
  class FacadeReadInterface
  {
  public:
    virtual ~FacadeReadInterface() {}
    
    /**
       Useful for deciding whether a client has to keep
       propagating. If a cell has status DOWNWIND or WAVEFRONT, its
       value has not yet been determined and you should keep
       propagating.    
    */
    typedef enum {
      /** the provided node (index) is outside of the grid */
      OUT_OF_GRID,
      /** the node is upwind (globally consistent, usable, your best friend) */
      UPWIND,
      /** the node is downwind, it's value might change during propagation */
      DOWNWIND,
      /** the node is on the wavefront, all bets are off */
      WAVEFRONT,
      /** the node is in a goal region */
      GOAL,
      /** the node is in an obstacle */
      OBSTACLE
    } node_status_t;
    
    /**
       \return The grid dimension along X.
    */
    virtual size_t GetXSize() const = 0;
    
    /**
       \return The grid dimension along Y.
    */
    virtual size_t GetYSize() const = 0;
    
    /**
       \return The spacing between grid cells.
    */
    virtual double GetScale() const = 0;
    
    /**
       \return The kernel-dependent "meta" which is used to represent
       a cell that lies entirely in freespace, i.e. it is completely
       traversable (collision risk is zero).
    */
    virtual double GetFreespaceMeta() const = 0;
    
    /**
       \return The kernel-dependent "meta" which is used to represent
       an obstacle cell, i.e. it is completely non-traversable
       (collision risk is one).
    */
    virtual double GetObstacleMeta() const = 0;
    
    /**
       \return The navigation function value of a cell, i.e. its
       "height" when you consider the navigation function to be a
       potential.
    */
    virtual double GetValue(size_t ix, size_t iy) const = 0;
    
    /**
       \return The kernel-dependent "meta" of the cell. Use
       GetFreespaceMeta() and GetObstacleMeta() to determine if the
       cell is an obstacle, in freespace, or something in between.
    */
    virtual double GetMeta(size_t ix, size_t iy) const = 0;
    
    /**
       \return true if the cell at index (ix, iy) is a goal cell.
    */
    virtual bool IsGoal(size_t ix, size_t iy) const = 0;
    
    /**
       \return true if the wavefront is non-empty.
       
       \note Depending on the application, it is usually better to use
       GetStatus() to find out if there is a need to keep propagating.
    */
    virtual bool HaveWork() const = 0;
    
    /**
       Determines the node_status_t of the cell at (ix, iy). This can
       be used to determine whether to call ComputeOne() or
       not. Typically, the cell (ix, iy) is where the robot is
       currently at, and you only need to propagate the wavefront
       further if GetStatus returns WAVEFRONT or DOWNWIND.
    */
    virtual node_status_t GetStatus(size_t ix, size_t iy) const = 0;
    
    /**
       The "lowest inconsistent value" is the "height" of the cell
       which would be propagated next. This value is needed if you
       want to figure out, for example, if a given cell is in the
       region of knownstabilized navigation function or not. However,
       due to replanning, a cell with a value higher than
       GetLowestInconsistentValue() can already be at its "stable"
       height. It will only change if the cell lies in the shadow of
       one of the changes that triggered the current replanning.
       
       \return false only if the queue is empty (all nodes are
       already locally consistent).
    */
    virtual bool GetLowestInconsistentValue(double & value) const = 0;
    
    /**
       \return The underlying Algorithm instance.
    */
    virtual const Algorithm & GetAlgorithm() const = 0;
    
    /**
       \return The underlying Grid instance.
    */
    virtual const Grid & GetGrid() const = 0;
    
    /**
       \return The underlying Kernel instance.
    */
    virtual const Kernel & GetKernel() const = 0;
  };
  
} // namespace estar

#endif // ESTAR_FACADE_READ_INTERFACE_HPP
