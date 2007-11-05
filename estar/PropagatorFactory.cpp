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


#include "PropagatorFactory.hpp"
#include "Propagator.hpp"
#include "Queue.hpp"
#include "Upwind.hpp"
// #include "util.hpp"
// #include "pdebug.hpp"


using namespace std;


namespace estar {
  
  
  PropagatorFactory::
  PropagatorFactory(Queue const & queue,
		    Upwind const & upwind,
		    cspace_t const & cspace,
		    value_map_t const & value,
		    meta_map_t const & meta,
		    rhs_map_t const & rhs,
		    flag_map_t const & flag,
		    bool check_upwind,
		    bool check_local_consistency,
		    bool check_queue_key)
    : m_queue(queue),
      m_upwind(upwind),
      m_cspace(cspace),
      m_value(value),
      m_meta(meta),
      m_rhs(rhs),
      m_flag(flag),
      m_check_upwind(check_upwind),
      m_check_local_consistency(check_local_consistency),
      m_check_queue_key(check_queue_key)
  {
  }
  
  
  Propagator * PropagatorFactory::
  Create(vertex_t target)
  {
    Propagator * prop(new Propagator(target, get(m_meta, target)));
    
    // Goal nodes never get expanded, so this check "should not" be
    // necessary, but in case we get here make sure that there are no
    // neighbors to interpolate from.
    if (get(m_flag, target) & GOAL)
      return prop;
    
    // This threshold fullfills two checks:
    // * if m_check_queue_key is false, it reverts to checking that a
    //   candidate neighbor is at least not an obstacle or un-computed
    //   or on the raising wake of an update
    // * if m_check_queue_key is true, we make sure that we only
    //   propagate from neighbors that lie below the current queue
    // * if m_check_queue_key is true but the queue is empty, we
    //   revert to the infinity check
    double queue_bottom(infinity);
    if (m_check_queue_key && ( ! m_queue.IsEmpty()))
      queue_bottom = m_queue.Get().begin()->first;
    
    // Loop over all neighbors, applying a configurable set of checks
    // on them.
    adjacency_it i_nbor;
    adjacency_it end_nbor;
    tie(i_nbor, end_nbor) = adjacent_vertices(target, m_cspace);
    for (/**/; i_nbor != end_nbor; ++i_nbor) {
      // If the target has been used to compute the neighbor's value,
      // then do not use the neighbor to compute ours as that would
      // risk creating loops in the upwind graph. The effectiveness of
      // this check somewhat depends on when Algorithm clears the
      // upwind neighborhood of the target, which might be before or
      // after calling this method.
      if (m_check_upwind && m_upwind.HaveEdge(target, *i_nbor))
	continue;
      
      const double nbor_value(get(m_value, *i_nbor));
      
      // If enabled, accept only neighbors that are locally consistent
      // (ie their rhs is the same as their value). The usefullness of
      // this check might be somewhat limited by the fact that local
      // consistency is maintained in invalid areas until the
      // reparation wavefront has passed through, which might take a
      // while.
      if (m_check_local_consistency && (nbor_value != get(m_rhs, *i_nbor)))
	continue;
      
      // The queue bottom check is probably the one with the most
      // "global" vision, as it excludes any neighbors that are either
      // on the queue or downwind from it. Note that if m_check_queue
      // is false, or the queue is empty, this test reverts to just
      // checking that a neighbor is not at infinity.
      if (nbor_value >= queue_bottom)
	continue;
      
      prop->m_nbor.insert(make_pair(nbor_value, *i_nbor));
    }
    
    return prop;
  }


} // namespace estar
