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
//#include "util.hpp"


using namespace std;


namespace estar {
  
  
  Propagator::
  Propagator(vertex_t target_vertex, double target_meta)
    : m_target_vertex(target_vertex),
      m_target_meta(target_meta)
  {
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
