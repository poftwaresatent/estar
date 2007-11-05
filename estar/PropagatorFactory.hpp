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


#ifndef ESTAR_PROPAGATOR_FACTORY_HPP
#define ESTAR_PROPAGATOR_FACTORY_HPP


#include <estar/base.hpp>


namespace estar {
  
  
  class Upwind;
  class Queue;
  class Algorithm;
  class Propagator;
  
  
  class PropagatorFactory
  {
  public:
    PropagatorFactory(Queue const & queue,
		      Upwind const & upwind,
		      cspace_t const & cspace,
		      value_map_t const & value,
		      meta_map_t const & meta,
		      rhs_map_t const & rhs,
		      flag_map_t const & flag,
		      /** use false to mimic old behavior */
		      bool check_upwind,
		      /** use false to mimic old behavior */
		      bool check_local_consistency,
		      /** use false to mimic old behavior */
		      bool check_queue_key);
    
    Propagator * Create(vertex_t target);
    
  private:
    Queue const & m_queue;
    Upwind const & m_upwind;
    cspace_t const & m_cspace;
    value_map_t const & m_value;
    meta_map_t const & m_meta;
    rhs_map_t const & m_rhs;
    flag_map_t const & m_flag;
    
    bool m_check_upwind;
    bool m_check_local_consistency;
    bool m_check_queue_key;
  };
  
} // namespace estar

#endif // ESTAR_PROPAGATOR_FACTORY_HPP
