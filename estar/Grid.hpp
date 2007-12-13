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


#ifndef ESTAR_GRID_HPP
#define ESTAR_GRID_HPP


#include <estar/GridNode.hpp>
#include <boost/shared_ptr.hpp>


namespace estar {
  
  
  class Algorithm;
  class Kernel;
  template<typename value_t> class flexgrid;
  
  
  class Grid
  {
  public:
    typedef enum {
      FOUR,
      EIGHT,
      SIX
    } neighborhood_t;
    
    /** You have to call Init() before actually using the Grid
	instance, otherwise you'll get segfaults. */
    explicit Grid(neighborhood_t neighborhood);
    
    /** Also calls Init(), see the comments there. */
    Grid(ssize_t xbegin, ssize_t xend,
	 ssize_t ybegin, ssize_t yend,
	 neighborhood_t neighborhood,
	 double meta);
    
    /**
       Allocate memory for the grid and initialize it (set its meta,
       connect to neighbors). This is not intended for ranges that do
       not include 0, so all bets are off if you use it that way.
    */
    void Init(ssize_t xbegin, ssize_t xend,
	      ssize_t ybegin, ssize_t yend,
	      double meta);
    
    /**
       Makes sure that the required range of indices is available,
       where (xend, yend) is the ONE-PAST-end marker for the X- and Y-
       directions. That is to say, after this call there is guaranteed
       to be a GridNode at [xend-1, yend-1] but not beyond that.
       
       All newly created GridNode instances are initialized by calling
       AddNode(). The method does not modify any prior existing nodes
       in the given range of indices, except for adding neighborhood
       edges where appropriate.
       
       \note This method calls Algorithm::AddVertex() in order to
       properly register any new nodes with the wavefront queue.
       
       \return The number of added vertices.
    */
    size_t AddRange(ssize_t xbegin, ssize_t xend,
		    ssize_t ybegin, ssize_t yend,
		    double meta,
		    Algorithm & algo, Kernel const & kernel);
    
    /**
       If (ix, iy) is already in the grid, simply call
       Algorithm::SetMeta(). Otherwise, add a new GridNode instance,
       hook it up via edges to its neighbors, initialize its meta, and
       call Algorithm::AddVertex().
       
       \note All freshly added nodes get assigned a value of infinity.
       
       \return true if a new GridNode was allocated and inserted into
       the grid.
    */
    bool AddNode(ssize_t ix, ssize_t iy, double meta,
		 Algorithm & algo, Kernel const & kernel);
    
    
    ssize_t GetXBegin() const;
    
    ssize_t GetXEnd() const;
    
    ssize_t GetYBegin() const;
    
    ssize_t GetYEnd() const;

    /**
       A direct but slightly unsafe way of accessing a GridNode. If
       you want to iterate over all nodes, you should call GetCSpace()
       and use BaseCSpace::begin() which is much more decoupled from
       the grid implementation.
       
       \note There is no guarantee that the entire range (GetXBegin(),
       GetXEnd(), GetYBegin(), GetYEnd()) is covered, so you might get
       null pointers.
       
       \return Can return a null pointer if there is no node residing
       at that index.
    */
    boost::shared_ptr<GridNode const> GetNode(ssize_t ix, ssize_t iy) const;
    
    boost::shared_ptr<GridCSpace const> GetCSpace() const { return m_cspace; }
    
    boost::shared_ptr<GridCSpace> GetCSpace() { return m_cspace; }
    
    /**
       \return -1: no such node. 0: success. 1: (partially) incomplete
       information.
    */
    int ComputeGradient(ssize_t ix, ssize_t iy,
			double & gradx, double & grady) const;
    
    /**
       \return true if there was information for both the X- and Y-
       directions.
    */
    bool ComputeGradient(boost::shared_ptr<GridNode const> node,
			 double & gradx, double & grady) const;
    
    /**
       \return -1: no such node. 0: success. 1: (partially) incomplete
       information. 2: gradient was too small, replaced with heuristic.
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

    /**
       \return 0: success. 1: (partially) incomplete information. 2:
       gradient was too small, replaced with heuristic.
    */
    int ComputeStableScaledGradient(boost::shared_ptr<GridNode const> node,
				    double stepsize,
				    double & gx,
				    double & gy,
				    double & dx,
				    double & dy) const;
    
    neighborhood_t GetNeighborhood() const { return m_neighborhood; }
    
  private:
    typedef GridCSpace::vertex_data_t vertex_data_t;
    typedef flexgrid<vertex_data_t> flexgrid_t;

    struct offset {
      offset(ssize_t _dx, ssize_t _dy): dx(_dx), dy(_dy) {}
      ssize_t dx, dy;
    };
    
    typedef std::vector<offset> nbor_offset_t;
    
    neighborhood_t const m_neighborhood;
    nbor_offset_t m_nbor_offset;
    
    boost::shared_ptr<flexgrid_t> m_flexgrid;
    boost::shared_ptr<GridCSpace> m_cspace;
    
    
    /**
       Blindly allocate a new node and add it to the flexgrid, hooking
       it up with its neighbors and initializing its meta.
    */
    vertex_data_t DoAddNode(ssize_t ix, ssize_t iy, double meta);
    
    void InitNborStuff();
  };
  
  
} // namespace estar

#endif // ESTAR_GRID_HPP
