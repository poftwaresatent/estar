/* 
 * Copyright (C) 2007 Roland Philippsen <roland dot philippsen at gmx net>
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

#include "Supergrid.hpp"
#include "Grid.hpp"
#include "flexgrid.hpp"


namespace estar {
  
  
  Supergrid::
  Supergrid(boost::shared_ptr<Algorithm> algo,
	    ssize_t sub_xsize, ssize_t sub_ysize)
    : m_sub_xsize(sub_xsize),
      m_sub_ysize(sub_ysize),
      m_flexgrid(new flexgrid_t()),
      m_algo(algo)
  {
  }
  
  
  bool Supergrid::
  AddGrid(ssize_t ix, ssize_t iy, subgrid_ptr grid)
  {
    subgrid_ptr & slot(m_flexgrid->smart_at(ix, iy));
    if (slot) {
      if (slot != grid)
	return false;		// occupied by someone else
      return true;		// already in supergrid
    }
    slot = grid;
    return true;
  }
  
  
  Supergrid::subgrid_ptr Supergrid::
  GetGrid(ssize_t ix, ssize_t iy)
  {
    subgrid_ptr result;
    try {
      result = m_flexgrid->at(ix, iy);
    }
    catch (std::out_of_range) {	// would be nice to do without exceptions
    }
    return result;
  }
  
  
  Supergrid::const_subgrid_ptr Supergrid::
  GetGrid(ssize_t ix, ssize_t iy) const
  {
    return const_cast<Supergrid*>(this)->GetGrid(ix, iy);
  }

} // namespace estar
