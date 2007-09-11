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


#ifndef ESTAR_UPWIND_HPP
#define ESTAR_UPWIND_HPP


#include <estar/base.hpp>
#include <set>
#include <map>


namespace estar {
  
  
  /** Upwind graph for tracing propagation order. */
  class Upwind {
  public:
    typedef std::set<vertex_t> set_t;
    typedef std::map<vertex_t, set_t > map_t;

    bool HaveEdge(vertex_t from, vertex_t to) const;
    void AddEdge(vertex_t from, vertex_t to);

    /** \note Beware when feeding vertices from iterators to this
	function, as it modifies the underlying tree structures this
	can wreak havoc. Use a temporary structure to collect the
	vertices to be removed, then loop over the temporary data.
    */
    void RemoveEdge(vertex_t from, vertex_t to);
    
    /** see also RemoveEdge() */
    void RemoveIncoming(vertex_t to);
    
    const map_t & GetMap() const { return m_from_to; }
    const set_t & GetDownwind(vertex_t from) const;
    
  private:
    mutable map_t m_from_to;
    mutable map_t m_to_from;
  };
  
} // namespace estar

#endif // ESTAR_UPWIND_HPP
