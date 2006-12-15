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


#include "Propagator.hpp"
#include "Upwind.hpp"
#include <estar/util.hpp>

  
#ifdef ESTAR_VERBOSE_DEBUG
# define ESTAR_PROPAGATOR_DEBUG
#else
# undef ESTAR_PROPAGATOR_DEBUG
#endif
  
#ifdef ESTAR_PROPAGATOR_DEBUG
# define PDEBUG PDEBUG_OUT
# include <sstream>
namespace local { typedef std::ostringstream debugstream; }
#else
# define PDEBUG PDEBUG_OFF
namespace local { typedef estar::fake_os debugstream; }
#endif


using namespace local;
using std::make_pair;


namespace estar {
  
  
  Propagator::
  Propagator(vertex_t target,
	     std::pair<adjacency_it, adjacency_it> neighbors,
	     const Upwind & upwind,
	     const flag_map_t & flag,
	     const value_map_t & value,
	     const meta_map_t & meta)
    : m_target_meta(get(meta, target)),
      m_target_vertex(target)
  {    
    if(get(flag, target) & GOAL){
      PDEBUG("target %lu is GOAL\n", target);
      return;
    }
    
    debugstream dbg;
    dbg << "\n ";
    for(/**/; neighbors.first != neighbors.second; ++neighbors.first){
      vertex_t nbor(*neighbors.first);
      if(upwind.HaveEdge(target, nbor)){
#define DONT_CHECK_DOWNWIND
#ifdef DONT_CHECK_DOWNWIND
	dbg << " (" << nbor << " dw)";
#else
	dbg << " " << nbor << " downwind  ";
	continue;
#endif
      }
      const double nbor_value(get(value, nbor));
      if(nbor_value < infinity){
	dbg << " " << nbor << " VALID  ";
	m_nbor.insert(make_pair(nbor_value, nbor));
      }
      else
	dbg << " " << nbor << " inf  ";
    }
    PDEBUG("target %lu%s\n", target, dbg.str().c_str());
  }
  
  
  double Propagator::
  GetTargetMeta() const
  {
    return m_target_meta;
  }
  
  
  std::pair<const_queue_it, const_queue_it> Propagator::
  GetUpwindNeighbors() const
  {
    return make_pair(m_nbor.begin(), m_nbor.end());
  }
  
  
  void Propagator::
  AddBackpointer(vertex_t cell)
  {
    m_bp.push_back(cell);
  }
  
  
  std::pair<Propagator::backpointer_it, Propagator::backpointer_it>
  Propagator::
  GetBackpointers()
  {
    return make_pair(m_bp.begin(), m_bp.end());
  }
  
  
  std::size_t Propagator::
  GetNBackpointers() const
  {
    return m_bp.size();
  }
  
  
  std::size_t Propagator::
  GetNUpwindNeighbors() const
  {
    return m_nbor.size();
  }
  
  
  vertex_t Propagator::
  GetTargetVertex() const
  {
    return m_target_vertex;
  }
  
  
} // namespace estar
