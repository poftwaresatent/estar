/* 
 * Copyright (C) 2005 Roland Philippsen <roland dot philippsen at gmx net>
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


#include "MetaMousehandler.hpp"
#include <estar/Algorithm.hpp>
#include <estar/Grid.hpp>
#include <iostream>		// dbg


using estar::vertex_t;


namespace gfx {


  MetaMousehandler::
  MetaMousehandler(estar::Algorithm & algo,
		   const estar::Grid & grid,
		   const estar::Kernel & kernel,
		   double obstacle_meta,
		   double freespace_meta)
    : m_algo(algo),
      m_grid(grid),
      m_kernel(kernel),
      m_meta_map(algo.GetMetaMap()),
      m_obst(obstacle_meta),
      m_free(freespace_meta)
  {
  }


  void MetaMousehandler::
  HandleClick(double x, double y)
  {
    std::cerr << "**************************************************\n"
	      << "* hello click " << x << " " << y << "\n"
	      << "**************************************************\n";
  
    if((0 > x)
       || (0 > y)
       || (m_grid.GetXSize() <= x)
       || (m_grid.GetYSize() <= y))
      return;
  
    const vertex_t vertex(m_grid.GetVertex(static_cast<size_t>(x),
						  static_cast<size_t>(y)));
    if(get(m_meta_map, vertex) == m_free)
      m_algo.SetMeta(vertex, m_obst, m_kernel);
    else
      m_algo.SetMeta(vertex, m_free, m_kernel);
  }

}
