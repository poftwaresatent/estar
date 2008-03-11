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


#include <estar/GridNode.hpp>
#include <vector>


namespace estar {
  
  class Algorithm;
  class Grid;
  class Kernel;
  
  
  /**
     An element of the 'trace' of the steepest gradient from a given
     start position. Use FacadeReadInterface::TraceCarrot() to compute
     a (partial) plan to the goal from any (valid) starting position.
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
    virtual double GetValue(ssize_t ix, ssize_t iy) const = 0;
    
    /**
       \return The kernel-dependent "meta" of the cell. Use
       GetFreespaceMeta() and GetObstacleMeta() to determine if the
       cell is an obstacle, in freespace, or something in between.
    */
    virtual double GetMeta(ssize_t ix, ssize_t iy) const = 0;
    
    /**
       \return true if the cell at index (ix, iy) is a goal cell.
    */
    virtual bool IsGoal(ssize_t ix, ssize_t iy) const = 0;
    
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
       
       \note If you don't know the indices of a node, use GetStatus(vertex_t).
    */
    virtual node_status_t GetStatus(ssize_t ix, ssize_t iy) const = 0;
    
    /**
       You don't need a node's coordinates in order to compute it's
       status. Just call this variant of GetStatus(ssize_t, ssize_t)
       instead.
    */
    virtual node_status_t GetStatus(vertex_t vertex) const = 0;
    
    /**
       \return true if there is a cell at index (ix, iy).
    */
    virtual bool IsValidIndex(ssize_t ix, ssize_t iy) const = 0;
    
    /**
       The "lowest inconsistent value" is the "height" of the cell
       which would be propagated next. This value is needed if you
       want to figure out, for example, if a given cell is in the
       region of known stabilized navigation function or not. However,
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
    
  protected:
    friend void dump_raw(const FacadeReadInterface &, FILE *, FILE *);
    friend void dump_queue(const FacadeReadInterface &, size_t, FILE *);
    friend void dump_facade_range_highlight(const FacadeReadInterface &,
					    ssize_t, ssize_t, ssize_t, ssize_t,
					    ssize_t, ssize_t, FILE *);
    
    /**
       \note this is a hack for legacy code, don't rely on it
    */
    virtual boost::shared_ptr<Grid const> GetGrid() const = 0;
  public:
    
    /**
       \return The underlying Kernel instance.
    */
    virtual const Kernel & GetKernel() const = 0;
    
    
    /**
       Compute a global path via iterative gradient descent. Start at
       the position <code>(robot_x, robot_y)</code> and repeatedly move
       <code>stepsize</code> along the gradient of the E* navigation
       function. The trace stops if one of the following conditions is
       met: The curvilinear distance reached the parameter
       <code>distance</code>, the number of iterations has reached
       <code>maxsteps</code>, or the trace has reached a goal location.
       
       \note The trace is computed in the grid frame of reference. The
       <code>trace</code> is cleared prior to filling it.
       
       \return
        0 on success<br>
        1 if distance wasn't reached after maxstep iterations<br>
       -1 if the robot is outside the grid<br>
       -2 if the grid is a hexgrid (not implemented yet)<br>
          other negative numbers would represent errors that were not
          yet defined at the time when this documentation was
          written... look at the implementation in
          Facade::TraceCarrot().
       
       \todo URGENT: Need ridge detection (and avoidance) to avoid
       going straight toward obstacle in case the robot is on the ridge
       where two wavefronts meet after they swept around an
       obstacle. In such a case, one direction should be chosen at
       "random"!
    */
    virtual int TraceCarrot(double robot_x, double robot_y,
			    double distance, double stepsize,
			    size_t maxsteps,
			    carrot_trace & trace,
			    /** optionally write error info here */
			    std::ostream * err_os) const = 0;

    virtual boost::shared_ptr<GridCSpace const> GetCSpace() const = 0;
  };
  
} // namespace estar

#endif // ESTAR_FACADE_READ_INTERFACE_HPP
