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


#include "CSpace.hpp"


using namespace boost;


namespace estar {
  
  
  BaseCSpace::
  BaseCSpace()
    : m_value(get(value_p(), m_cspace)),
      m_meta(get(meta_p(), m_cspace)),
      m_rhs(get(rhs_p(), m_cspace)),
      m_flag(get(flag_p(), m_cspace)),
      m_vertexid(get(vertex_index, m_cspace))
  {
  }
  
  
  double BaseCSpace::
  GetValue(vertex_t vertex) const
  {
    return get(m_value, vertex);
  }

  
  double BaseCSpace::
  GetMeta(vertex_t vertex) const
  {
    return get(m_meta, vertex);
  }

  
  double BaseCSpace::
  GetRhs(vertex_t vertex) const
  {
    return get(m_rhs, vertex);
  }

  
  flag_t BaseCSpace::
  GetFlag(vertex_t vertex) const
  {
    return get(m_flag, vertex);
  }

  
  void BaseCSpace::
  SetValue(vertex_t vertex, double value)
  {
    put(m_value, vertex, value);
  }

  
  void BaseCSpace::
  SetMeta(vertex_t vertex, double meta)
  {
    put(m_meta, vertex, meta);
  }

  
  void BaseCSpace::
  SetRhs(vertex_t vertex, double rhs)
  {
    put(m_rhs, vertex, rhs);
  }

  
  void BaseCSpace::
  SetFlag(vertex_t vertex, flag_t flag)
  {
    put(m_flag, vertex, flag);
  }

  
  void BaseCSpace::
  AddNeighbor(vertex_t from, vertex_t to)
  {
    add_edge(from, to, m_cspace);
  }

    
  vertex_t BaseCSpace::
  AddVertex(double value, double meta, double rhs, flag_t flag)
  {
    vertex_t cv(add_vertex(m_cspace));

    put(m_value, cv, value);
    put(m_meta,  cv, meta);
    put(m_rhs,   cv, rhs);
    put(m_flag,  cv, flag);

    return cv;
  }
  
} // namespace estar
