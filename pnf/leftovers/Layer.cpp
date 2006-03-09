/* 
 * Copyright (C) 2004
 * Swiss Federal Institute of Technology, Lausanne. All rights reserved.
 * 
 * Author: Roland Philippsen <roland dot philippsen at gmx net>
 *         Autonomous Systems Lab <http://asl.epfl.ch/>
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


#include "Layer.hpp"
#include <estar/Grid.hpp>
#include <estar/Algorithm.hpp>
#include <estar/LSMKernel.hpp>


using estar::Grid;
using estar::Algorithm;
using estar::LSMKernel;
using boost::scoped_ptr;


namespace pnf {
  
  
  Layer::
  Layer(size_t xsize, size_t ysize)
    : m_algo(new Algorithm()),
      m_grid(new Grid(*m_algo, xsize, ysize)),
      m_kernel(new LSMKernel(*m_grid)),
      m_value_map(m_algo->GetValueMap()),
      m_meta_map(m_algo->GetMetaMap()),
      freespace_meta(m_kernel->freespace_meta),
      obstacle_meta(m_kernel->obstacle_meta)
  {
  }
  
  
  void Layer::
  SetMeta(size_t ix, size_t iy, double meta)
  { m_algo->SetMeta(m_grid->GetVertex(ix, iy), meta, *m_kernel); }
  
  
  double Layer::
  GetMeta(size_t ix, size_t iy)
    const
  { return get(m_meta_map, m_grid->GetVertex(ix, iy)); }
  

  bool Layer::
  HaveWork()
    const
  { return m_algo->HaveWork(); }
  
  
  void Layer::
  ComputeOne()
  { m_algo->ComputeOne(*m_kernel); }


  double Layer::
  GetValue(size_t ix, size_t iy)
    const
  { return get(m_value_map, m_grid->GetVertex(ix, iy)); }
  
}
