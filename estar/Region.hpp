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


#ifndef ESTAR_REGION_HPP
#define ESTAR_REGION_HPP


#include <estar/Sprite.hpp>
#include <boost/shared_ptr.hpp>


namespace estar {
  
  
  /**
     A Region is a Sprite that has been placed at a specific location.
  */
  class Region
  {
  public:
    typedef Sprite::sindex sindex;
    typedef Sprite::indexlist_t indexlist_t;
    
    const double x0;
    const double y0;
    
    Region(boost::shared_ptr<Sprite> sprite,
	   double x0, double y0, ssize_t xsize, ssize_t ysize);
    
    Region(double radius, double scale,
	   double x0, double y0, ssize_t xsize, ssize_t ysize);
    
    const Sprite & GetSprite() const { return * m_sprite; }
    const indexlist_t & GetBorder() const { return m_border; }
    const indexlist_t & GetArea() const { return m_area; }
    
  private:
    boost::shared_ptr<Sprite> m_sprite;
    indexlist_t m_border;
    indexlist_t m_area;
    
    void Init(double radius, double scale,
	      double x0, double y0, ssize_t xsize, ssize_t ysize);
  };
  
}

#endif // ESTAR_REGION_HPP
