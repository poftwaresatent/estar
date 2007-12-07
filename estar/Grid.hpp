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


#ifndef ESTAR_GRID_HPP
#define ESTAR_GRID_HPP


#include <estar/GridNode.hpp>
#include <estar/util.hpp>


namespace estar {
  
  
  /** Select type of connections for a Grid instance. */
  typedef enum {
    FOUR_CONNECTED,
    EIGHT_CONNECTED,
    HEX_GRID
  } connectedness_t;
  
  
  /** C-space graph that is actually a grid. The topology can be
      chosen from connectedness_t. */
  class Grid {
  public:
    const ssize_t xsize, ysize;
    const connectedness_t connect;
    
    Grid(ssize_t xsize, ssize_t ysize, connectedness_t connect);
    
    ssize_t GetXSize() const { return xsize; }
    ssize_t GetYSize() const { return ysize; }
    
    const GridNode & Vertex2Node(vertex_t vertex) const
    { return *(m_cspace->Lookup(vertex)); }
    
    const GridNode & Index2Node(ssize_t ix, ssize_t iy) const
    { return *(m_node_array[ix][iy]); }
    
    vertex_t Index2Vertex(ssize_t ix, ssize_t iy) const
    { return m_node_array[ix][iy]->vertex; }
    
    /** The gradient is computed with an upwind formula. It does not
	take into account neither the grid resolution nor the grid
	frame. This requires information not available within the Grid
	instance.
	
	\return true if we got upwind-components for both x and y.

	\todo This would be a good place for "ridge detection", to be
	used in trace_carrot(). Check how many clients depend on this
	before adding it though.
    */
    bool ComputeGradient(ssize_t ix, ssize_t iy,
			 double & gradx, double & grady) const;
    
    /**
       This is a more "useful" version of the gradient
       computation. Apart from doing ComputeGradient() on the
       parameters <code>(gx, gy)</code>, it applies "stabilizing"
       heuristics and scales the gradient to length
       <code>stepsize</code> and returns that in <code>(dx,
       dy)</code>.
       
       \return Information on the "quality" of (dx, dy)<br>
       0: all is fine, you can trust (gx, gy) and (dx, dy)<br>
       1: ComputeGradient() failed, you can use (dx, dy) but it will
          simply point along an coordinate axis or along a
          bisector.<br>
       2: Although ComputeGradient() succeeded, the (dx, dy) data was
          stabilized using the same heuristic as if in case 1 because
          the gradient length was too small.
	  
       \todo BUG: The heuristic produces (dx, dy) of lengths 0.5 or
       sqrt(0.25) or 0... not really proper, but "usable" so far.
    */
    int ComputeStableScaledGradient(ssize_t ix, ssize_t iy,
				    /** wanted length of (dx, dy) */
				    double stepsize,
				    /** only valid if return 0 */
				    double & gx,
				    /** only valid if return 0 */
				    double & gy,
				    /** always valid */
				    double & dx,
				    /** always valid */
				    double & dy) const;

    grid_postransform const & GetPosTransform() const
    { return *m_postransform; }
    
    boost::shared_ptr<GridCSpace const> GetCSpace() const { return m_cspace; }
    boost::shared_ptr<GridCSpace> GetCSpace() { return m_cspace; }
    
  protected:
    typedef array<boost::shared_ptr<GridNode>, ssize_t> node_array_t;
    node_array_t m_node_array;
    
    boost::shared_ptr<GridCSpace> m_cspace;
    
    // not really needed here, lives better in GridCSpace or so
    boost::shared_ptr<grid_postransform const> m_postransform;
  };
  
  
} // namespace estar

#endif // ESTAR_GRID_HPP
