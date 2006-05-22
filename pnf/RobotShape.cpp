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
#include <iostream>

// only for hacked draw_region() function
#include <gfx/wrap_gl.hpp>

// tmp manual override
#define DEBUG
#ifdef DEBUG
# define PNF_SPRITE_DEBUG
#else // DEBUG
# undef PNF_SPRITE_DEBUG
#endif // DEBUG

#ifdef PNF_SPRITE_DEBUG
# define PDEBUG PDEBUG_OUT
#else
# define PDEBUG PDEBUG_OFF
#endif

#define PVDEBUG PDEBUG_OFF


using estar::square;
using estar::minval;
using estar::boundval;
using estar::array;
using std::vector;


namespace pnf {
  
  
  /** \todo braindead implementation! */  
  Sprite::
  Sprite(double _radius, double _scale)
    : radius(_radius),
      scale(_scale)
  {
    const ssize_t offset(static_cast<ssize_t>(ceil(_radius / _scale)));
    for(ssize_t ix(-offset); ix <= offset; ++ix){
      const double x2(square(ix * _scale));
      for(ssize_t iy(-offset); iy <= offset; ++iy){
	const double rr(sqrt(square(iy * _scale) + x2));
	if(rr <= _radius){
	  m_area.push_back(sindex(ix, iy, rr));
	  if(rr >= _radius - _scale)
	    m_border.push_back(sindex(ix, iy, rr));
	}
      }
    }
    if(m_area.empty()){
      PDEBUG("empty area, repairing with center index\n");
      m_area.push_back(sindex(0, 0, 0));
    }
    if(m_border.empty()){
      PDEBUG("empty border, repairing with center index\n");
      m_border.push_back(sindex(0, 0, 0));
    }
  }
  
  
  void Sprite::
  Dump(std::ostream & os) const
  {
    const ssize_t offset(static_cast<ssize_t>(ceil(radius / scale)));
    const ssize_t dim(2 * offset + 1);
    array<char> sprite(dim, dim, '.'); // '.' means empty
    for(size_t ib(0); ib < m_border.size(); ++ib){
      ssize_t ix(m_border[ib].x + offset);
      ssize_t iy(m_border[ib].y + offset);
      if('.' == sprite[ix][iy])
	sprite[ix][iy] = 'x';	// 'x' means only border error
      else
	sprite[ix][iy] = '#';	// '#' means duplicate border error
    }
    for(size_t ia(0); ia < m_area.size(); ++ia){
      ssize_t ix(m_area[ia].x + offset);
      ssize_t iy(m_area[ia].y + offset);
      if('.' == sprite[ix][iy])
	sprite[ix][iy] = 'o';	// 'o' means only area
      else if('x' == sprite[ix][iy])
	sprite[ix][iy] = '*';	// '*' means border and area
      else
	sprite[ix][iy] = '%';	// '%' means area and duplicate border error
    }
    for(ssize_t iy(dim - 1); iy >= 0; --iy){
      for(ssize_t ix(0); ix < dim; ++ix)
	os << sprite[ix][iy] << ' ';
      os << "\n";
    }
  }
  
  
  Region::
  Region(boost::shared_ptr<Sprite> sprite,
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


namespace gfx {
  
  void draw_region(const pnf::Region & region,
		   double red, double green, double blue)
  {
    glColor3d(red, green, blue);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1);
    const pnf::Region::indexlist_t & area(region.GetArea());
    for(size_t ia(0); ia < area.size(); ++ia)
      glRectd(area[ia].x, area[ia].y, area[ia].x + 1, area[ia].y + 1);
  }
  
}
