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


#include "Grid.hpp"
#include "Algorithm.hpp"
#include "util.hpp"
#include "pdebug.hpp"


namespace local {
  // copy paste from graphics.cpp
  static const double hex_yscale(0.866025403784); // sqrt(3)/2
}

using namespace local;
using namespace boost;
using namespace std;


namespace estar {
  
  
  std::pair<double, double> grid_postransform_cartesian::
  operator()(double ix, double iy) const
  { return make_pair(ix, iy); }
  
  
  std::pair<double, double> grid_postransform_hexgrid::
  operator()(double ix, double iy) const
  {
    if(absval(fmod(iy, 2) - 1) > 0.5)
      return make_pair(ix + 0.5, hex_yscale * iy + 0.5);
    return make_pair(ix + 1.0, hex_yscale * iy + 0.5);
  }
  
  
  Grid::
  Grid(ssize_t _xsize, ssize_t _ysize, connectedness_t _connect)
    : xsize(_xsize),
      ysize(_ysize),
      connect(_connect),
      m_node_array(_xsize, _ysize)
  {
    boost::shared_ptr<grid_bbox_compute const> bbox_compute;
    if (HEX_GRID == _connect) {
      bbox_compute.
	reset(new grid_bbox_compute_fixed(0,
					  0,
					  _xsize + 0.5,
					  hex_yscale * (_ysize - 1) + 1));
      m_postransform.reset(new grid_postransform_hexgrid());
    }
    else {
      bbox_compute.reset(new grid_bbox_compute_fixed(-0.5,
						     -0.5,
						     _xsize - 0.5,
						     _ysize - 0.5));
      m_postransform.reset(new grid_postransform_cartesian());
    }
    m_cspace.reset(new GridCSpace(m_postransform, bbox_compute));
    
    for(ssize_t iy(0); iy < _ysize; ++iy)
      for(ssize_t ix(0); ix < _xsize; ++ix){
	shared_ptr<GridNode> gn(new GridNode());
	gn->ix = ix;
	gn->iy = iy;
	gn->vertex = m_cspace->AddVertex(gn);
	m_node_array[ix][iy] = gn;
      }
    
    for(ssize_t ix(0); ix < _xsize-1; ++ix)
      for(ssize_t iy(0); iy < _ysize-1; ++iy){
	m_cspace->AddNeighbor(Index2Vertex(ix, iy), Index2Vertex(ix+1, iy  ));
	m_cspace->AddNeighbor(Index2Vertex(ix, iy), Index2Vertex(ix  , iy+1));
      }
    for(ssize_t ix(0); ix < _xsize-1; ++ix)
      m_cspace->AddNeighbor(Index2Vertex(ix  , _ysize-1),
			    Index2Vertex(ix+1, _ysize-1));
    for(ssize_t iy(0); iy < _ysize-1; ++iy)
      m_cspace->AddNeighbor(Index2Vertex(_xsize-1, iy  ),
			    Index2Vertex(_xsize-1, iy+1));
    
    switch(connect){
    case FOUR_CONNECTED:
      PVDEBUG("four-connected grid\n");
      break;
    case EIGHT_CONNECTED:
      PVDEBUG("eight-connected grid\n");
      for(ssize_t ix(0); ix < _xsize-1; ++ix)
	for(ssize_t iy(0); iy < _ysize-1; ++iy){
	  m_cspace->AddNeighbor(Index2Vertex(ix  , iy  ),
				Index2Vertex(ix+1, iy+1));
	  m_cspace->AddNeighbor(Index2Vertex(ix+1, iy  ),
				Index2Vertex(ix  , iy+1));
	}
      break;
    case HEX_GRID:
      PVDEBUG("hex grid\n");
      for(ssize_t iy(0); iy < _ysize-1; iy += 2)
	for(ssize_t ix(1); ix < _xsize; ++ix)
	  m_cspace->AddNeighbor(Index2Vertex(ix  , iy  ),
				Index2Vertex(ix-1, iy+1));
      for(ssize_t iy(1); iy < _ysize-1; iy += 2)
	for(ssize_t ix(0); ix < _xsize-1; ++ix)
	  m_cspace->AddNeighbor(Index2Vertex(ix  , iy  ),
				Index2Vertex(ix+1, iy+1));
      break;
    default:
      PDEBUG_ERR("BUG: Invalid connect parameter %d\n", connect);
      exit(EXIT_FAILURE);
    }
  }
  
  
  std::ostream & operator << (std::ostream & os, const GridNode & gn)
  {
    return os << "[(" << gn.ix << ", " << gn.iy << ") i:" << gn.vertex << "]";
  }
  
  
  /**
     \todo This can now be moved away from any specific grid
     implementation, given that the local grid neighbor is entirely
     defined in GridNode.
  */
  bool Grid::
  ComputeGradient(ssize_t ix, ssize_t iy,
		   double & gradx, double & grady) const
  {
    const double baseval(m_cspace->GetValue(Index2Vertex(ix, iy)));
    bool have_x(false);
    bool have_y(false);
    gradx = 0;
    grady = 0;
    
    if(ix < xsize - 1){
      const double
	dxplus(m_cspace->GetValue(Index2Vertex(ix + 1, iy)) - baseval);
      if(dxplus < 0){
	have_x = true;
	gradx = dxplus;
      }
    }
    if(ix > 0){
      const double
	dxminus(baseval - m_cspace->GetValue(Index2Vertex(ix - 1, iy)));
      if(dxminus > 0){
	if(have_x){
	  gradx += dxminus;
	  gradx /= 2;
	}
	else{
	  have_x = true;
	  gradx = dxminus;
	}
      }
    }
    
    if(iy < ysize - 1){
      const double
	dyplus(m_cspace->GetValue(Index2Vertex(ix, iy + 1)) - baseval);
      if(dyplus < 0){
	have_y = true;
	grady = dyplus;
      }
    }
    if(iy > 0){
      const double
	dyminus(baseval - m_cspace->GetValue(Index2Vertex(ix, iy - 1)));
      if(dyminus > 0){
	if(have_y){
	  grady += dyminus;
	  grady /= 2;
	}
	else{
	  have_y = true;
	  grady = dyminus;
	}
      }
    }
    
    if(( ! have_x) || ( ! have_y))
      PVDEBUG("WARNING: have_x = %s   have_y = %s\n",
	      have_x ? "TRUE" : "FALSE", have_y ? "TRUE" : "FALSE");
    return have_x && have_y;
  }
  
  
  int Grid::
  ComputeStableScaledGradient(ssize_t ix, ssize_t iy,
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
    const bool ok(ComputeGradient(ix, iy, gx, gy));
    bool heur(false);
    if(ok){
      const double alpha(stepsize / (sqrt(square(gx) + square(gy))));
      if(alpha < epsilon)
	heur = true;
      else{
	dx = gx * alpha;
	dy = gy * alpha;
      }
    }
    if(heur || ( ! ok)){
      if(gx > 0)      dx =   stepsize / 2;
      else if(gx < 0) dx = - stepsize / 2;
      if(gy > 0)      dy =   stepsize / 2;
      else if(gy < 0) dy = - stepsize / 2;
    }
    if( ! ok)
      return 1;
    if(heur)
      return 2;
    return 0;
  }
  
} // namespace estar
