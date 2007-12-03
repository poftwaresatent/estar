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


#include <estar/base.hpp>
#include <boost/scoped_array.hpp>
//#include <cstdio>


namespace estar {
  
  
  class Algorithm;
  
  
  /** Select type of connections for a Grid instance. */
  typedef enum {
    FOUR_CONNECTED,
    EIGHT_CONNECTED,
    HEX_GRID
  } connectedness_t;
  
  
  /** Holds a C-space vertex and associated data for a node in a Grid. */
  class GridNode {
  public:
    GridNode() {}
    GridNode(size_t _ix, size_t _iy,
	     vertex_t _vertex, size_t _igrid)
      : ix(_ix), iy(_iy), vertex(_vertex), igrid(_igrid) {}
    friend std::ostream & operator << (std::ostream & os, const GridNode & gn);
    size_t ix, iy;
    vertex_t vertex;

  private:
    friend class Grid;
    size_t igrid;
  };
  
  
  /** C-space graph that is actually a grid. The topology can be
      chosen from connectedness_t. */
  class Grid {
  public:
    const size_t xsize, ysize;
    const connectedness_t connect;
    
    const value_map_t & value_map;
    const meta_map_t &  meta_map;
    const rhs_map_t &   rhs_map;
    const flag_map_t &  flag_map;

    Grid(Algorithm & algo, size_t xsize, size_t ysize,
	 connectedness_t connect);
    
    size_t GetXSize() const { return xsize; }
    size_t GetYSize() const { return ysize; }
    
    const GridNode & Vertex2Node(vertex_t vertex) const
    { return boost::get(m_node_map, vertex); }
    
    const GridNode & Index2Node(size_t ix, size_t iy) const
    { return m_node_array[Index2Index(ix, iy)]; }
    
    const GridNode & Index2Node(size_t igrid) const
    { return m_node_array[igrid]; }

    vertex_t Index2Vertex(size_t ix, size_t iy) const
    { return m_node_array[Index2Index(ix, iy)].vertex; }
    
    vertex_t Index2Vertex(size_t igrid) const
    { return m_node_array[igrid].vertex; }
    
    size_t Index2Index(size_t ix, size_t iy) const
    { return ix + xsize * iy; }
    
    /** The gradient is computed with an upwind formula. It does not
	take into account neither the grid resolution nor the grid
	frame. This requires information not available within the Grid
	instance.
	
	\return true if we got upwind-components for both x and y.

	\todo This would be a good place for "ridge detection", to be
	used in trace_carrot(). Check how many clients depend on this
	before adding it though.
    */
    bool ComputeGradient(size_t ix, size_t iy,
			 double & gradx, double & grady) const;
    
    
  protected:
    typedef boost::iterator_property_map<GridNode *, vertexid_map_t>
    node_map_t;
    
    boost::scoped_array<GridNode> m_node_array;
    node_map_t m_node_map;
  };
  
  
} // namespace estar

#endif // ESTAR_GRID_HPP
