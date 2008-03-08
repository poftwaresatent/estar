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
#include "pdebug.hpp"
#include <iostream>

#ifdef WIN32
# include <estar/win32.hpp>
#endif // WIN32


using namespace boost;
using namespace std;


namespace estar {
  
  
  Region::
  Region(shared_ptr<Sprite> sprite,
	 double _x0, double _y0,
	 ssize_t xbegin, ssize_t xend,
	 ssize_t ybegin, ssize_t yend)
    : x0(_x0),
      y0(_y0),
      m_sprite(sprite)
  {
    Init(sprite->radius, sprite->scale, x0, y0, xbegin, xend, ybegin, yend);
  }
  
  
  Region::
  Region(shared_ptr<Sprite> sprite,
	 double _x0, double _y0)
    : x0(_x0),
      y0(_y0),
      m_sprite(sprite)
  {
    Init(sprite->radius, sprite->scale, x0, y0,
	 numeric_limits<ssize_t>::min(), numeric_limits<ssize_t>::max(), 
	 numeric_limits<ssize_t>::min(), numeric_limits<ssize_t>::max());
  }
  
  
  Region::
  Region(double radius, double scale,
	 double _x0, double _y0,
	 ssize_t xbegin, ssize_t xend,
	 ssize_t ybegin, ssize_t yend)
    : x0(_x0),
      y0(_y0),
      m_sprite(new Sprite(radius, scale))
  {
    Init(radius, scale, x0, y0, xbegin, xend, ybegin, yend);
  }
  
  
  Region::
  Region(double radius, double scale,
	 double _x0, double _y0)
    : x0(_x0),
      y0(_y0),
      m_sprite(new Sprite(radius, scale))
  {
    Init(radius, scale, x0, y0,
	 numeric_limits<ssize_t>::min(), numeric_limits<ssize_t>::max(), 
	 numeric_limits<ssize_t>::min(), numeric_limits<ssize_t>::max());
  }
  
  
  /** \todo braindead implementation! */  
  void Region::
  Init(double radius, double scale,
       double x0, double y0,
       ssize_t xbegin, ssize_t xend,
       ssize_t ybegin, ssize_t yend)
  {
    const ssize_t
      ix0(boundval(xbegin,
		   static_cast<ssize_t>(rint((x0-radius)/scale)),
		   xend));
    const ssize_t
      iy0(boundval(ybegin,
		   static_cast<ssize_t>(rint((y0-radius)/scale)),
		   yend));
    const ssize_t
      ix1(boundval(xbegin,
		   static_cast<ssize_t>(rint((x0+radius)/scale)),
		   xend));
    const ssize_t
      iy1(boundval(ybegin,
		   static_cast<ssize_t>(rint((y0+radius)/scale)),
		   yend));
    for(ssize_t ix(ix0); ix < ix1; ++ix)
      for(ssize_t iy(iy0); iy < iy1; ++iy){
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
      if((xx0 >= xbegin) && (xx0 < xend) && (yy0 >= ybegin) && (yy0 < yend)){
	PVDEBUG("empty area, repairing with center index\n");
	m_area.push_back(sindex(xx0, yy0, 0));
      }
      else
	PVDEBUG("WARNING: cannot repair empty area!\n");
    }
    if(m_border.empty()){
      const ssize_t xx0(static_cast<ssize_t>(rint(x0/scale)));
      const ssize_t yy0(static_cast<ssize_t>(rint(y0/scale)));
      if((xx0 >= xbegin) && (xx0 < xend) && (yy0 >= ybegin) && (yy0 < yend)){
	PVDEBUG("empty border, repairing with center index\n");
	m_border.push_back(sindex(xx0, yy0, 0));
      }
      else
	PVDEBUG("WARNING: cannot repair empty border!\n");
    }
  }

}
