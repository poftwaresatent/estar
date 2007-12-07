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


#ifndef ESTAR_SUPERGRID_HPP
#define ESTAR_SUPERGRID_HPP


#include <boost/shared_ptr.hpp>


namespace estar {
  
  
  class Grid;
  class Algorithm;
  template<typename value_t> class flexgrid;
  
  
  class Supergrid
  {
  public:
    typedef boost::shared_ptr<Grid> subgrid_ptr;
    typedef boost::shared_ptr<const Grid> const_subgrid_ptr;
    
//     struct index {
//       index(ssize_t _virtual_idx, ssize_t _super_idx, size_t _sub_idx)
// 	: virtual_idx(_virtual_idx), super_idx(_super_idx) sub_idx(_sub_idx) {}
//       ssize_t virtual_idx;
//       ssize_t super_idx;
//       size_t sub_idx;
//     };
    
    
    Supergrid(boost::shared_ptr<Algorithm> algo,
	      ssize_t sub_xsize, ssize_t sub_ysize);
    
//     index ComputeIndex(ssize_t virtual_index) const {
//       if (virtual_index > 0) {
// 	index idx(virtual_index,
// 		  virtual_index / static_cast<ssize_t>(m_subgrid_size),
// 		  static_cast<size_t>(virtual_index) % m_subgrid_size);
// 	return idx;
//       }
//       else if (virtual_index < 0) {
// 	index idx(virtual_index,
// 		  virtual_index / static_cast<ssize_t>(m_subgrid_size),
// 		  m_subgrid_size - (static_cast<size_t>(1 - virtual_index)
// 				    % m_subgrid_size));
// 	return idx;
//       }
//       index idx(0, 0, 0);
//       return idx;
//     }
    
    bool AddGrid(ssize_t ix, ssize_t iy, subgrid_ptr grid);
    subgrid_ptr GetGrid(ssize_t ix, ssize_t iy);
    const_subgrid_ptr GetGrid(ssize_t ix, ssize_t iy) const;
    
  private:
    typedef flexgrid<subgrid_ptr> flexgrid_t;
    
    ssize_t const m_sub_xsize;
    ssize_t const m_sub_ysize;
    
    boost::shared_ptr<flexgrid_t> m_flexgrid;
    boost::shared_ptr<Algorithm> m_algo;
  };
  
  
} // namespace estar

#endif // ESTAR_SUPERGRID_HPP
