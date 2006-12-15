/* 
 * Copyright (C) 2006
 * Swiss Federal Institute of Technology, Zurich. All rights reserved.
 * 
 * Author: Roland Philippsen <roland dot philippsen at gmx net>
 *         Autonomous Systems Lab <http://www.asl.ethz.ch/>
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


#include "Region.hpp"
#include "numeric.hpp"
#include "util.hpp"
#include <iostream>

using namespace boost;
using namespace std;

#ifdef ESTAR_VERBOSE_DEBUG
# define PDEBUG PDEBUG_OUT
#else // ! ESTAR_DEBUG
# define PDEBUG PDEBUG_OFF
#endif // ESTAR_DEBUG


namespace estar {
  
  
  Region::
  Region(shared_ptr<Sprite> sprite,
	 double _x0, double _y0, ssize_t xsize, ssize_t ysize)
    : x0(_x0),
      y0(_y0),
      m_sprite(sprite)
  {
    Init(sprite->radius, sprite->scale, x0, y0, xsize, ysize);
  }
  
  
  Region::
  Region(double radius, double scale,
	 double _x0, double _y0, ssize_t xsize, ssize_t ysize)
    : x0(_x0),
      y0(_y0),
      m_sprite(new Sprite(radius, scale))
  {
    Init(radius, scale, x0, y0, xsize, ysize);
  }
  
  
  /** \todo braindead implementation! */  
  void Region::
  Init(double radius, double scale,
       double x0, double y0, ssize_t xsize, ssize_t ysize)
  {
    const ssize_t xmax(xsize - 1);
    const ssize_t ymax(ysize - 1);
    const ssize_t
      ix0(boundval(0, static_cast<ssize_t>(rint((x0-radius)/scale)), xmax));
    const ssize_t
      iy0(boundval(0, static_cast<ssize_t>(rint((y0-radius)/scale)), ymax));
    const ssize_t
      ix1(boundval(0, static_cast<ssize_t>(rint((x0+radius)/scale)), xmax));
    const ssize_t
      iy1(boundval(0, static_cast<ssize_t>(rint((y0+radius)/scale)), ymax));
    for(ssize_t ix(ix0); ix <= ix1; ++ix)
      for(ssize_t iy(iy0); iy <= iy1; ++iy){
	const double dr(sqrt(square(ix*scale-x0) + square(iy*scale-y0)));
	if(dr <= radius){
	  m_area.push_back(sindex(ix, iy, dr));
	  if(dr >= radius - scale)
	    m_border.push_back(sindex(ix, iy, dr));
	}
      }
    if(m_area.empty()){
      const ssize_t xx0(static_cast<ssize_t>(rint(x0/scale)));
      const ssize_t yy0(static_cast<ssize_t>(rint(y0/scale)));
      if((xx0 >= 0) && (xx0 < xsize) && (yy0 >= 0) && (yy0 < ysize)){
	PDEBUG("empty area, repairing with center index\n");
	m_area.push_back(sindex(xx0, yy0, 0));
      }
      else
	PDEBUG("WARNING: cannot repair empty area!\n");
    }
    if(m_border.empty()){
      const ssize_t xx0(static_cast<ssize_t>(rint(x0/scale)));
      const ssize_t yy0(static_cast<ssize_t>(rint(y0/scale)));
      if((xx0 >= 0) && (xx0 < xsize) && (yy0 >= 0) && (yy0 < ysize)){
	PDEBUG("empty border, repairing with center index\n");
	m_border.push_back(sindex(xx0, yy0, 0));
      }
      else
	PDEBUG("WARNING: cannot repair empty border!\n");
    }
  }

}
