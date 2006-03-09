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


#ifndef ESTAR_CHECK_HPP
#define ESTAR_CHECK_HPP


#include <estar/base.hpp>
#include <iosfwd>


namespace estar {
  
  
  class Algorithm;
  class Grid;
  
  
  bool check_cspace(const cspace_t & cspace,
		    const char * prefix, std::ostream & os);
  
  /**
     \note If grid is null it will be ignored.
     \todo Add "smart" Queue class checking.
  */
  bool check_queue(const Algorithm & algo, const Grid * grid,
		   const char * prefix, std::ostream & os);
  
} // namespace estar

#endif // ESTAR_CHECK_HPP
