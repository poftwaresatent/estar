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


#ifndef ESTAR_SPRITE_HPP
#define ESTAR_SPRITE_HPP


#include <vector>
#include <iosfwd>


namespace estar {
  
  
  /**
     Utility for holding a relocatable set of vertices, like a
     computer graphics sprite. For the moment, this provides only
     circular sprites (such as used for typical goal regions). When
     the need arises we can add some other shapes.
  */
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
  
}

#endif // ESTAR_SPRITE_HPP
