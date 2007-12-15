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


#include "Grid.hpp"
#include "Algorithm.hpp"
#include "Kernel.hpp"
#include "flexgrid.hpp"
#include "numeric.hpp"


using namespace boost;
using namespace std;


namespace local {
  
  // copy paste from graphics.cpp
  static const double hex_yscale(0.866025403784); // sqrt(3)/2
  
  struct postransform_cartesian: public estar::grid_postransform {
    virtual std::pair<double, double> operator()(double ix, double iy) const
    { return make_pair(ix, iy); }
  };
  
  struct postransform_hexgrid: public estar::grid_postransform {
    virtual std::pair<double, double> operator()(double ix, double iy) const {
      if(estar::absval(fmod(iy, 2) - 1) > 0.5)
	return make_pair(ix + 0.5, hex_yscale * iy + 0.5);
      return make_pair(ix + 1.0, hex_yscale * iy + 0.5);
    }
  };
  
  struct bbox_cartesian: public estar::grid_bbox_compute {
    explicit bbox_cartesian(estar::Grid const * _grid)
      : grid(_grid) {}
    
    virtual void operator()(double & x0, double & y0,
			    double & x1, double & y1) const {
      x0 = grid->GetXBegin() - 0.5;
      y0 = grid->GetYBegin() - 0.5;
      x1 = grid->GetXEnd() - 0.5;
      y1 = grid->GetYEnd() - 0.5;
    }
    
    estar::Grid const * grid;
  };
  
  struct bbox_hexgrid: public estar::grid_bbox_compute {
    explicit bbox_hexgrid(estar::Grid const * _grid)
      : grid(_grid) {}
    
    virtual void operator()(double & x0, double & y0,
			    double & x1, double & y1) const {
      x0 = grid->GetXBegin();
      y0 = grid->GetYBegin();
      x1 = grid->GetXEnd() + 0.5;
      y1 = hex_yscale * (grid->GetYEnd() - 1) + 1;
    }
    
    estar::Grid const * grid;
  };  
  
}

using namespace local;


namespace estar {


  Grid::
  Grid(neighborhood_t neighborhood)
    : m_neighborhood(neighborhood)
  {
    InitNborStuff();
  }
  
  
  Grid::
  Grid(ssize_t xbegin, ssize_t xend,
       ssize_t ybegin, ssize_t yend,
       neighborhood_t neighborhood,
       double meta)
    : m_neighborhood(neighborhood)
  {
    InitNborStuff();
    Init(xbegin, xend, ybegin, yend, meta);
  }
  
  
  void Grid::
  Init(ssize_t xbegin, ssize_t xend,
       ssize_t ybegin, ssize_t yend,
       double meta)
  {
    m_flexgrid.reset(new flexgrid_t());
    m_flexgrid->resize(xbegin, xend, ybegin, yend);
    for (ssize_t ix(xbegin); ix < xend; ++ix)
      for (ssize_t iy(ybegin); iy < yend; ++iy)
	DoAddNode(ix, iy, meta);
  }
  
  
  void Grid::
  InitNborStuff()
  {
    m_nbor_offset.push_back(offset( 0, -1));
    m_nbor_offset.push_back(offset( 0,  1));
    m_nbor_offset.push_back(offset(-1,  0));
    m_nbor_offset.push_back(offset( 1,  0));
    if (SIX == m_neighborhood) {
      m_nbor_offset.push_back(offset(-1,  1));
      m_nbor_offset.push_back(offset( 1,  1));
    }
    else if (EIGHT == m_neighborhood) {
      m_nbor_offset.push_back(offset(-1, -1));
      m_nbor_offset.push_back(offset(-1,  1));
      m_nbor_offset.push_back(offset( 1, -1));
      m_nbor_offset.push_back(offset( 1,  1));
    }
    
    shared_ptr<grid_postransform const> postransform;
    shared_ptr<grid_bbox_compute const> bbox_compute;
    if (SIX == m_neighborhood) {
      postransform.reset(new postransform_hexgrid());
      bbox_compute.reset(new bbox_hexgrid(this));
    }
    else {
      postransform.reset(new postransform_cartesian());
      bbox_compute.reset(new bbox_cartesian(this));
    }
    m_cspace.reset(new GridCSpace(postransform, bbox_compute));
  }
  
  
  size_t Grid::
  AddRange(ssize_t xbegin, ssize_t xend,
	   ssize_t ybegin, ssize_t yend,
	   double meta,
	   Algorithm & algo, Kernel const & kernel)
  {
    size_t count(0);
    
    // Grow the flexgrid if necessary. Do NOT simply call
    // flexgrid::resize() because that could end up removing cells,
    // which we do not support (yet?).
    if (xbegin < m_flexgrid->xbegin())
      m_flexgrid->resize_xbegin(xbegin);
    if (xend > m_flexgrid->xend())
      m_flexgrid->resize_xend(xend);
    if (ybegin < m_flexgrid->ybegin())
      m_flexgrid->resize_ybegin(ybegin);
    if (yend > m_flexgrid->yend())
      m_flexgrid->resize_yend(yend);
    
    for (ssize_t ix(xbegin); ix < xend; ++ix)
      for (ssize_t iy(ybegin); iy < yend; ++iy) {
	vertex_data_t node(m_flexgrid->at(ix, iy));
	if ( ! node) {
	  node = DoAddNode(ix, iy, meta);
	  algo.AddVertex(node->vertex, kernel);
	  ++count;
	}
      }
    
    return count;
  }
  
  
  bool Grid::
  AddNode(ssize_t ix, ssize_t iy, double meta,
	  Algorithm & algo, Kernel const & kernel)
  {
    bool added(false);
    vertex_data_t node(m_flexgrid->smart_at(ix, iy));
    if (node)
      algo.SetMeta(node->vertex, meta, kernel);
    else {
      node = DoAddNode(ix, iy, meta);
      algo.AddVertex(node->vertex, kernel);
      added = true;
    }
    return added;
  }
  
  
  ssize_t Grid::
  GetXBegin() const
  {
    return m_flexgrid->xbegin();
  }
  
  
  ssize_t Grid::
  GetXEnd() const
  {
    return m_flexgrid->xend();
  }
  
  
  ssize_t Grid::
  GetYBegin() const
  {
    return m_flexgrid->ybegin();
  }
  
  
  ssize_t Grid::
  GetYEnd() const
  {
    return m_flexgrid->yend();
  }
  
  
  boost::shared_ptr<GridNode const> Grid::
  GetNode(ssize_t ix, ssize_t iy) const
  {
    vertex_data_t node;
    if (m_flexgrid->valid(ix, iy))
      node = m_flexgrid->at(ix, iy);
    return node;
  }
  
  
  Grid::vertex_data_t Grid::
  DoAddNode(ssize_t ix, ssize_t iy, double meta)
  {
    vertex_data_t node(new GridNode());
    node->ix = ix;
    node->iy = iy;
    node->vertex = m_cspace->AddVertex(node);
    m_flexgrid->at(ix, iy) = node;
    m_cspace->SetMeta(node->vertex, meta);
    
    for (nbor_offset_t::const_iterator in(m_nbor_offset.begin());
	 in != m_nbor_offset.end(); ++in) {
      ssize_t const nx(ix + in->dx);
      ssize_t const ny(iy + in->dy);
      if (m_flexgrid->valid(nx, ny)) {
	vertex_data_t const nbor(m_flexgrid->at(nx, ny));
	if (nbor)
	  m_cspace->AddNeighbor(node->vertex, nbor->vertex);
      }
    }
    
    return node;
  }
  
  
  int Grid::
  ComputeGradient(ssize_t ix, ssize_t iy,
		  double & gradx, double & grady) const
  {
    shared_ptr<GridNode const> node(GetNode(ix, iy));
    if ( ! node)
      return -1;
    if (ComputeGradient(node, gradx, grady))
      return 0;
    return 1;
  }
  
  
  bool Grid::
  ComputeGradient(shared_ptr<GridNode const> node,
		  double & gradx, double & grady) const
  {
    const double baseval(m_cspace->GetValue(node->vertex));
    size_t count_x(0);
    size_t count_y(0);
    gradx = 0;
    grady = 0;

    // Only look at four-neighborhood structure. If we're a hexgrid,
    // this will compute garbage.
    for (edge_read_iteration iedge(m_cspace->begin(node->vertex));
	 iedge.not_at_end(); ++iedge) {
      shared_ptr<GridNode const> const nbor(m_cspace->Lookup(*iedge));
      if (node->ix == nbor->ix) {
	if (node->iy > nbor->iy) {
	  const double
	    dyminus(baseval - m_cspace->GetValue(nbor->vertex));
	  if (dyminus > 0) {
	    grady += dyminus;
	    ++count_y;
	  }
	}
	else {			// trust that iy really differs
	  const double
	    dyplus(m_cspace->GetValue(nbor->vertex) - baseval);
	  if (dyplus < 0) {
	    grady += dyplus;
	    ++count_y;
	  }
	}
      }
      else if (node->iy == nbor->iy) {
	if (node->ix > nbor->ix) {
	  const double
	    dxminus(baseval - m_cspace->GetValue(nbor->vertex));
	  if (dxminus > 0) {
	    gradx += dxminus;
	    ++count_x;
	  }
	}
	else {
	  const double
	    dxplus(m_cspace->GetValue(nbor->vertex) - baseval);
	  if (dxplus < 0) {
	    gradx += dxplus;
	    ++count_x;
	  }
	}
      }
    }
    
    if (1 < count_x)
      gradx /= static_cast<double>(count_x);
    if (1 < count_y)
      grady /= static_cast<double>(count_y);
    
    return (count_x != 0) && (count_y != 0);
  }
  
  
  int Grid::
  ComputeStableScaledGradient(ssize_t ix, ssize_t iy,
			      double stepsize,
			      double & gx,
			      double & gy,
			      double & dx,
			      double & dy) const
  {
    shared_ptr<GridNode const> node(GetNode(ix, iy));
    if ( ! node)
      return -1;
    return ComputeStableScaledGradient(node, stepsize, gx, gy, dx, dy);
  }
  
  
  int Grid::
  ComputeStableScaledGradient(shared_ptr<GridNode const> node,
			      double stepsize,
			      double & gx,
			      double & gy,
			      double & dx,
			      double & dy) const
  {
    dx = 0;
    dy = 0;
    gx = 0;			// redundant with ComputeGradient()
    gy = 0;			// but prudent about future changes
    bool const ok(ComputeGradient(node, gx, gy));
    bool heur(false);
    if (ok) {
      const double alpha(stepsize / (sqrt(square(gx) + square(gy))));
      if (alpha < epsilon)
	heur = true;
      else {
	dx = gx * alpha;
	dy = gy * alpha;
      }
    }
    if (heur || ( ! ok)) {
      if (gx > 0)      dx =   stepsize / 2;
      else if (gx < 0) dx = - stepsize / 2;
      if (gy > 0)      dy =   stepsize / 2;
      else if (gy < 0) dy = - stepsize / 2;
    }
    if ( ! ok)
      return 1;
    if (heur)
      return 2;
    return 0;
  }
  
} // namespace estar
