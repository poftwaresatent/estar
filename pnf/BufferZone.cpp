/* 
 * Copyright (C) 2006 Roland Philippsen <roland dot philippsen at gmx net>
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


#include "BufferZone.hpp"
#include <cmath>


namespace pnf {
  
  
  BufferZone::
  BufferZone(double _radius, double _buffer, double _degree)
    : radius(_radius),
      buffer(_buffer),
      degree(_degree)
  {
  }
  
  
  double BufferZone::
  DistanceToRisk(double distance) const
  {
    if(distance <= radius)
      return 1;
    if(distance > radius + buffer)
      return 0;
    return pow(1 - (distance - radius) / buffer, degree);
  }
  
}
