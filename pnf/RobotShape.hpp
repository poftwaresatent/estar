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


#ifndef PNF_ROBOT_SHAPE_HPP
#define PNF_ROBOT_SHAPE_HPP


#include <pnf/Flow.hpp>
#include <vector>
#include <iosfwd>


namespace pnf {
  
  
  class Sprite
  {
  public:
    /** Utility for holding "relative" nodes. */
    class sindex {
    public:
      sindex(ssize_t _x, ssize_t _y, double _r): x(_x), y(_y), r(_r) { }
      ssize_t x, y;
      double r;
    };
    
    typedef std::vector<sindex> indexlist_t;
    
    const double radius;
    const double scale;
    
    Sprite(double radius, double scale);
    
    void Dump(std::ostream & os) const;
    
    const indexlist_t & GetBorder() const { return m_border; }
    const indexlist_t & GetArea() const { return m_area; }
    
  private:
    indexlist_t m_border;
    indexlist_t m_area;
  };
  
  
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


namespace gfx {
  
  /** \todo HACK: this shouldn't live here, but it's in the middle of
      a refactoring anyways... */
  void draw_region(const pnf::Region & region,
		   double red, double green, double blue);
  
}

#endif // PNF_ROBOT_SHAPE_HPP
