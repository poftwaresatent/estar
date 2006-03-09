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


#ifndef GFX_META_MOUSEHANDLER_HPP
#define GFX_META_MOUSEHANDLER_HPP


#include <gfx/Mousehandler.hpp>
#include <estar/base.hpp>


namespace estar {
  class Algorithm;
  class Grid;
  class Kernel;
}


namespace gfx {


  /** For interactively changing the meta information of a node. Not
      used in quite a while though (at least since summer 2005), so
      probably needs an update. */
  class MetaMousehandler:
    public Mousehandler
  {
  public:
    /** \todo
	REFACTOR: obstacle and freespace meta should be queried from kernel. */
    MetaMousehandler(estar::Algorithm & algo,
		     const estar::Grid & grid,
		     const estar::Kernel & kernel,
		     double obstacle_meta,
		     double freespace_meta);
  
    virtual void HandleClick(double x, double y);

  protected:
    estar::Algorithm & m_algo;
    const estar::Grid & m_grid;
    const estar::Kernel & m_kernel;
    const estar::meta_map_t & m_meta_map;
    const double m_obst;
    const double m_free;
  };

}

#endif // GFX_META_MOUSEHANDLER_HPP
