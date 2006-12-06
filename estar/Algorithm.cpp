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


#include "Algorithm.hpp"
#include "Kernel.hpp"
#include "Propagator.hpp"
#include <estar/util.hpp>
#include <boost/assert.hpp>
#include <iostream>


using boost::vertex_index;
using boost::tie;
using std::cerr;
using std::make_pair;


#ifdef DEBUG
# define ESTAR_ALGORITHM_DEBUG
#else
# undef ESTAR_ALGORITHM_DEBUG
#endif

#ifdef ESTAR_ALGORITHM_DEBUG
# define PDEBUG PDEBUG_OUT
#else
# define PDEBUG PDEBUG_OFF
#endif


namespace estar {
  
  
  Algorithm::
  Algorithm():
    m_step(0),
    m_last_computed(make_pair<vertex_t, double>(0, -1)),
    m_pending_reset(false)
  {
    m_value    = get(value_p(),    m_cspace);
    m_meta     = get(meta_p(),     m_cspace);
    m_rhs      = get(rhs_p(),      m_cspace);
    m_flag     = get(flag_p(), m_cspace);
    m_vertexid = get(vertex_index, m_cspace);
  }
  
  
  vertex_t Algorithm::
  AddVertex(double value,
	    double meta,
	    double rhs,
	    flag_t flag)
  {
    vertex_t cv(add_vertex(m_cspace));
    
    put(m_value,    cv, value);
    put(m_meta,     cv, meta);
    put(m_rhs,      cv, rhs);
    put(m_flag,     cv, flag);
    
    return cv;
  }
  
  
  void Algorithm::
  AddNeighbor(vertex_t from, vertex_t to)
  {
    add_edge(from, to, m_cspace);
  }
  
  
  void Algorithm::
  SetMeta(vertex_t vertex, double meta, const Kernel & kernel)
  {
    if(get(m_meta, vertex) == meta)
      return;
    put(m_meta, vertex, meta);
    UpdateVertex(vertex, kernel);
  }
  
  
  void Algorithm::
  ComputeOne(const Kernel & kernel)
  {
    if(m_pending_reset){
      Reset();
      m_pending_reset = false;
    }
    if(m_queue.IsEmpty())
      return;
    ++m_step;
    
    const vertex_t vertex(m_queue.Pop(m_flag));
    const double rhs(get(m_rhs, vertex));

    if(get(m_value, vertex) > rhs){
      put(m_value, vertex, rhs);
      m_last_computed = make_pair(vertex, rhs);
      adjacency_it in, nend;
      tie(in, nend) = adjacent_vertices(vertex, m_cspace);
      for(/**/; in != nend; ++in)
	UpdateVertex(*in, kernel);
    }
    
    else if(get(m_value, vertex) < rhs){
      put(m_value, vertex, infinity);
      m_last_computed = make_pair(vertex, rhs);
      
#define RE_PROPAGATE_LAST
#ifndef RE_PROPAGATE_LAST
      PDEBUG("variant: update raised node before others\n");
      UpdateVertex(vertex, kernel);
#endif // ! RE_PROPAGATE_LAST
      
#define RAISE_DOWNWIND_ONLY
#ifdef RAISE_DOWNWIND_ONLY
      PDEBUG("variant: expand only downwind neighbors after raise\n");
      // IMPORTANT: Get a copy, because m_upwind is modified inside
      // the loop by UpdateVertex()!!!
      Upwind::set_t dwnbors(m_upwind.GetDownwind(vertex));
      for(Upwind::set_t::const_iterator id(dwnbors.begin());
	  id != dwnbors.end(); ++id)
	UpdateVertex(*id, kernel);
#else // RAISE_DOWNWIND_ONLY
      PDEBUG("variant: expand all neighbors after raise\n");
      adjacency_it in, nend;
      tie(in, nend) = adjacent_vertices(vertex, m_cspace);
      for(/**/; in != nend; ++in)
	UpdateVertex(*in, kernel);
#endif // RAISE_DOWNWIND_ONLY
      
#ifdef RE_PROPAGATE_LAST
      PDEBUG("variant: update raised node after others\n");
      UpdateVertex(vertex, kernel);
#endif // RE_PROPAGATE_LAST
    }
    
    else{ // might happen due to goal tweaking (?)
      PDEBUG_OUT("yes, this actually happens!");
      m_last_computed = make_pair(vertex, rhs);
    }
  }
  
  
  bool Algorithm::
  HaveWork() const
  {
    return m_pending_reset || ( ! m_queue.IsEmpty());
  }
  
  
  void Algorithm::
  AddGoal(vertex_t vertex, double value)
  {
    const flag_t flag(get(m_flag, vertex));
    if((flag & GOAL) && (absval(get(m_rhs, vertex) - value) < epsilon)){
      PDEBUG("no change");
      return;
    }
    put(m_rhs,   vertex, value);
    put(m_flag,  vertex, static_cast<flag_t>(flag | GOAL));
    m_goalset.insert(vertex);
    if(absval(get(m_value, vertex) - value) < epsilon){
      PDEBUG("same value");
      return;
    }
    PDEBUG("needs (re)queuing\n");
    put(m_value, vertex, infinity);
    m_queue.Requeue(vertex, m_flag, m_value, m_rhs);
  }
  
  
  void Algorithm::
  RemoveGoal(vertex_t vertex)
  {
    if(get(m_flag, vertex) & GOAL){
      m_goalset.erase(vertex);
      put(m_flag, vertex, NONE);
      m_pending_reset = true;
    }
  }
  
  
  void Algorithm::
  RemoveAllGoals()
  {
    if(m_goalset.empty())
      return;
    for(goalset_t::iterator ig(m_goalset.begin()); ig != m_goalset.end(); ++ig)
      put(m_flag, *ig, NONE);
    m_goalset.clear();
    m_pending_reset = true;
  }
  
  
  bool Algorithm::
  IsGoal(vertex_t vertex)
    const
  {
    return get(m_flag, vertex) & GOAL;
  }
  
  
  void Algorithm::
  InitMeta(vertex_t vertex, double meta)
  {
    put(m_meta, vertex, meta);
  }
  
  
  void Algorithm::
  InitAllMeta(double meta)
  {
    vertex_it iv, vend;
    for(tie(iv, vend) = vertices(m_cspace); iv != vend; ++iv)
      put(m_meta, *iv, meta);
  }
  
  
  double Algorithm::
  GetLastComputedValue()
    const
  {
    return m_last_computed.second;
  }
  
  
  vertex_t Algorithm::
  GetLastComputedVertex()
    const
  {
    return m_last_computed.first;
  }
  
  
  void Algorithm::
  Reset()
  {
    m_queue.Clear();
    vertex_it iv, vend;
    tie(iv, vend) = vertices(m_cspace);
    for(/**/; iv != vend; ++iv){
      //clear_vertex(get(m_upwind_v, *iv), m_upwind);
      if(get(m_flag, *iv) & GOAL){
	put(m_value, *iv, infinity);
	put(m_flag,  *iv, GOAL);
	m_queue.Requeue(*iv, m_flag, m_value, m_rhs);
      }
      else{
	put(m_value, *iv, infinity);
	put(m_rhs,   *iv, infinity);
	put(m_flag,  *iv, NONE);
      }
    }
  }
  

  void Algorithm::
  UpdateVertex(vertex_t vertex, const Kernel & kernel)
  {
    const flag_t flag(get(m_flag, vertex));
    if(flag & GOAL){
      PDEBUG("i: %lu f: %s special goal handling\n", vertex, flag_name(flag));
      return;
    }
    else{
      Propagator prop(vertex, adjacent_vertices(vertex, m_cspace),
		      m_upwind, m_flag, m_value, m_meta);
      const double rhs(kernel.Compute(prop));
      put(m_rhs, vertex, rhs);

      m_upwind.RemoveIncoming(vertex);
      Propagator::backpointer_it ibp, bpend;
      tie(ibp, bpend) = prop.GetBackpointers();
      for(/**/; ibp != bpend; ++ibp)
	m_upwind.AddEdge(*ibp, vertex);
      
      PDEBUG("i: %lu f: %s v: %g rhs: %g\n",
	     vertex, flag_name(flag), get(m_value, vertex), rhs);

      m_queue.Requeue(vertex, m_flag, m_value, m_rhs);
    }
  }
  
} // namespace estar
