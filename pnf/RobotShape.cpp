/* 
 * Copyright (C) 2004
 * Swiss Federal Institute of Technology, Lausanne. All rights reserved.
 * 
 * Author: Roland Philippsen <roland dot philippsen at gmx net>
 *         Autonomous Systems Lab <http://asl.epfl.ch/>
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


#include "RobotShape.hpp"
#include <estar/numeric.hpp>


using estar::square;


namespace pnf {
  
  
  /** \todo could do a quarter of the computations if taking advantage
      of circle symetries */
  RobotShape::
  RobotShape(double radius, double scale)
  {
    const double r2(square(radius));
    const ssize_t range(static_cast<ssize_t>(ceil(radius / scale)));
    for(ssize_t ix(-range); ix <= range; ++ix)
      for(ssize_t iy(-range); iy <= range; ++iy){
	const double dr2(square(ix * scale) + square(iy * scale));
	if(dr2 <= r2)
	  m_offsetlist.push_back(offset(ix, iy));
      }
  }
  
  
  double RobotShape::
  FuseRisk(size_t uix, size_t uiy,
	   size_t uxsize, size_t uysize,
	   const estar::array<double> & cooc)
    const
  {
    static const ssize_t signmask(std::numeric_limits<ssize_t>::max());
    const ssize_t ix(uix & signmask);
    const ssize_t iy(uiy & signmask);
    const ssize_t xsize(uxsize & signmask);
    const ssize_t ysize(uysize & signmask);
    double result(1 - cooc[ix][iy]);
    for(offsetlist_t::const_iterator im(m_offsetlist.begin());
	im != m_offsetlist.end(); ++im){
      const ssize_t x(ix + im->x);
      if((x < 0) || (x >= xsize))
	continue;
      const ssize_t y(iy + im->y);
      if((y < 0) || (y >= ysize))
	continue;
      result *= 1 - cooc[x][y];
    }
    return 1 - result;
  }
  
}
