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


#ifndef ESTAR_FLEXGRID_TRAITS_HPP
#define ESTAR_FLEXGRID_TRAITS_HPP


#include <estar/sdeque.hpp>


namespace estar {
  
  
  template<typename value_t>
  class flexgrid_traits
  {
  public:
    typedef sdeque<value_t> line_t;
    typedef typename line_t::iterator cell_iterator;
    typedef typename line_t::const_iterator const_cell_iterator;
    
    typedef sdeque<line_t> grid_t;
    typedef typename grid_t::iterator line_iterator;
    typedef typename grid_t::const_iterator const_line_iterator;
  };
  
}

#endif // ESTAR_FLEXGRID_TRAITS_HPP
