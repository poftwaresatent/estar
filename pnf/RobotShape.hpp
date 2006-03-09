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


namespace pnf {
  
  
  /** Two-dimensional correlation mask. Utility for C-space transform,
      but (currently) independent of exact robot shape, that is to say
      it performs C-space projection. */
  class RobotShape
  {
  public:
    RobotShape(double radius, double scale);
    
    double FuseRisk(size_t uix, size_t uiy,
		    size_t uxsize, size_t uysize,
		    const estar::array<double> & cooc) const;
    
  private:
    /** Utility for holding "relative" nodes. */
    class offset {
    public:
      offset(ssize_t _x, ssize_t _y): x(_x), y(_y) { }
      ssize_t x, y;
    };
    typedef std::vector<offset> offsetlist_t;
    offsetlist_t m_offsetlist;
  };
  
}

#endif // PNF_ROBOT_SHAPE_HPP
