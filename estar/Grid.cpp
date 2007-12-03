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


namespace estar {
  
  
  Grid::
  Grid(Algorithm & algo, size_t _xsize, size_t _ysize,
       connectedness_t _connect):
    xsize(_xsize),
    ysize(_ysize),
    connect(_connect),
    value_map(algo.GetValueMap()),
    meta_map(algo.GetMetaMap()),
    rhs_map(algo.GetRhsMap()),
    flag_map(algo.GetFlagMap()),
    m_node_array(new GridNode[_xsize * _ysize]),
    m_node_map(m_node_array.get(), algo.GetVertexIdMap())
  {
    for(size_t iy(0); iy < _ysize; ++iy)
      for(size_t ix(0); ix < _xsize; ++ix){
	const vertex_t v(algo.AddVertex());
	put(m_node_map, v, GridNode(ix, iy, v, Index2Index(ix, iy)));
      }
    
    for(size_t ix(0); ix < _xsize-1; ++ix)
      for(size_t iy(0); iy < _ysize-1; ++iy){
	algo.AddNeighbor(Index2Vertex(ix, iy), Index2Vertex(ix+1, iy  ));
	algo.AddNeighbor(Index2Vertex(ix, iy), Index2Vertex(ix  , iy+1));
      }
    for(size_t ix(0); ix < _xsize-1; ++ix)
      algo.AddNeighbor(Index2Vertex(ix  , _ysize-1),
		       Index2Vertex(ix+1, _ysize-1));
    for(size_t iy(0); iy < _ysize-1; ++iy)
      algo.AddNeighbor(Index2Vertex(_xsize-1, iy  ),
		       Index2Vertex(_xsize-1, iy+1));
    
    switch(connect){
    case FOUR_CONNECTED:
      PVDEBUG("four-connected grid\n");
      break;
    case EIGHT_CONNECTED:
      PVDEBUG("eight-connected grid\n");
      for(size_t ix(0); ix < _xsize-1; ++ix)
	for(size_t iy(0); iy < _ysize-1; ++iy){
	  algo.AddNeighbor(Index2Vertex(ix  , iy  ), Index2Vertex(ix+1, iy+1));
	  algo.AddNeighbor(Index2Vertex(ix+1, iy  ), Index2Vertex(ix  , iy+1));
	}
      break;
    case HEX_GRID:
      PVDEBUG("hex grid\n");
      for(size_t iy(0); iy < _ysize-1; iy += 2)
	for(size_t ix(1); ix < _xsize; ++ix)
	  algo.AddNeighbor(Index2Vertex(ix  , iy  ), Index2Vertex(ix-1, iy+1));
      for(size_t iy(1); iy < _ysize-1; iy += 2)
	for(size_t ix(0); ix < _xsize-1; ++ix)
	  algo.AddNeighbor(Index2Vertex(ix  , iy  ), Index2Vertex(ix+1, iy+1));
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
  
  
  bool Grid::
  ComputeGradient(size_t ix, size_t iy,
		   double & gradx, double & grady) const
  {
#define UPWIND_GRADIENT
#ifdef UPWIND_GRADIENT
    
    const double baseval(get(value_map, Index2Vertex(ix, iy)));
    bool have_x(false);
    bool have_y(false);
    gradx = 0;
    grady = 0;
    
    if(ix < xsize - 1){
      const double dxplus(get(value_map, Index2Vertex(ix + 1, iy)) - baseval);
      if(dxplus < 0){
	have_x = true;
	gradx = dxplus;
      }
    }
    if(ix > 0){
      const double dxminus(baseval - get(value_map, Index2Vertex(ix - 1, iy)));
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
      const double dyplus(get(value_map, Index2Vertex(ix, iy + 1)) - baseval);
      if(dyplus < 0){
	have_y = true;
	grady = dyplus;
      }
    }
    if(iy > 0){
      const double dyminus(baseval - get(value_map, Index2Vertex(ix, iy - 1)));
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
    
#else // ! UPWIND_GRADIENT
    
    const double baseval(get(value_map, Index2Vertex(ix, iy)));
    gradx = 0;
    double xscale(0);
    grady = 0;
    double yscale(0);
    
    if(ix < xsize - 1){
      gradx += get(value_map, Index2Vertex(ix + 1, iy)) - baseval;
      xscale += 1;
    }
    if(ix > 0){
      gradx += baseval - get(value_map, Index2Vertex(ix - 1, iy));
      xscale += 1;
    }
    if(iy < ysize - 1){
      grady += get(value_map, Index2Vertex(ix, iy + 1)) - baseval;
      yscale += 1;
    }
    if(iy > 0){
      grady += baseval - get(value_map, Index2Vertex(ix, iy - 1));
      yscale += 1;
    }
    
    gradx /= xscale;
    grady /= yscale;
    return (absval(gradx) > epsilon) || (absval(grady) > epsilon);
    
#endif // UPWIND_GRADIENT
  }
  
} // namespace estar
