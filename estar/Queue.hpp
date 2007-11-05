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


#ifndef ESTAR_QUEUE_HPP
#define ESTAR_QUEUE_HPP


#include <estar/base.hpp>
#include <map>


namespace estar {
  
  
  typedef std::multimap<double, vertex_t> queue_t;
  typedef queue_t::iterator queue_it;
  typedef queue_t::const_iterator const_queue_it;
  
  typedef std::map<vertex_t, double> queue_map_t;
  
  
  /** Wavefront propagation queue. Sorted by ascending key, used
      (mainly) by Algorithm. */
  class Queue {
  public:
    bool IsEmpty() const { return m_queue.empty(); }
    
    vertex_t Pop(flag_map_t & flag_map);

    /** \note flag_map has to reflect the actual presence of vertex in queue */
    void Requeue(vertex_t vertex,
		 flag_map_t & flag_map,
		 const value_map_t & value_map,
		 const rhs_map_t & rhs_map);
    void Clear();
    
    /**
       For debugging: Puts the vertex (if present) to the top of the
       queue, returns true on success. If you then call check_queue(),
       it will fail... should only be done just prior to Pop().
    */
    bool VitaminB(vertex_t vertex);
    
    const queue_t &     Get() const    { return m_queue; }
    const queue_map_t & GetMap() const { return m_map; }
    
  private:
    queue_t m_queue;
    queue_map_t m_map;
    
    /** \note WARNING doesn't update m_map. */
    void DoDequeue(vertex_t vertex, queue_t::iterator iq);
  };
  
} // namespace estar

#endif // ESTAR_QUEUE_HPP
