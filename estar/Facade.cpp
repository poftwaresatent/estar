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


#include "Facade.hpp"
#include "Algorithm.hpp"
#include "Grid.hpp"
#include "NF1Kernel.hpp"
#include "AlphaKernel.hpp"
#include "LSMKernel.hpp"
#include "dump.hpp"
#include "Region.hpp"


#ifdef DEBUG
# define ESTAR_FACADE_DEBUG
#else
# undef ESTAR_FACADE_DEBUG
#endif

#ifdef ESTAR_FACADE_DEBUG
# define PDEBUG PDEBUG_OUT
#else
# define PDEBUG PDEBUG_OFF
#endif

  
namespace estar {
  
  
  Facade::
  Facade(Algorithm * algo, Grid * grid, Kernel * kernel, double _scale)
    : xsize(grid->xsize),
      ysize(grid->ysize),
      scale(_scale),
      m_algo(algo),
      m_grid(grid),
      m_kernel(kernel)
  {
  }
  
  
  Facade * Facade::
  Create(const std::string & kernel_name,
	 size_t xsize,
	 size_t ysize,
	 double scale,
	 int connect_diagonal,
	 FILE * dbgstream)
  {
    Algorithm * algo(new Algorithm());
    Grid * grid;
    if(connect_diagonal)
      grid = new Grid(*algo, xsize, ysize, EIGHT_CONNECTED);
    else
      grid = new Grid(*algo, xsize, ysize, FOUR_CONNECTED);
    Kernel * kernel(0);
    if(kernel_name == "nf1")
      kernel = new NF1Kernel();
    else if(kernel_name == "alpha")
      kernel = new AlphaKernel(scale);
    else if(kernel_name == "lsm")
      kernel = new LSMKernel(*grid, scale);
    else{
      if(0 != dbgstream)
	fprintf(dbgstream,
		"ERROR in %s():\n"
		"  invalid kernel_name \"%s\"\n"
		"  known kernels: nf1, alpha, lsm\n",
		__FUNCTION__, kernel_name.c_str());
      delete grid;
      delete algo;
      return 0;
    }
    
    return new Facade(algo, grid, kernel, scale);
  }
  
  
  Facade * Facade::
  CreateDefault(size_t xsize,
		size_t ysize,
		double scale)
  {
    Algorithm * algo(new Algorithm());
    Grid * grid(new Grid(*algo, xsize, ysize, FOUR_CONNECTED));
    Kernel * kernel(new LSMKernel(*grid, scale));
    return new Facade(algo, grid, kernel, scale);
  }
  
  
  double Facade::
  GetFreespaceMeta()
    const
  {
    return m_kernel->freespace_meta;
  }
  
  
  double Facade::
  GetObstacleMeta()
    const
  {
    return m_kernel->obstacle_meta;
  }
  
  
  double Facade::
  GetValue(size_t ix, size_t iy)
    const
  {
    return get(m_algo->GetValueMap(), m_grid->GetVertex(ix, iy));
  }
  
  
  double Facade::
  GetMeta(size_t ix, size_t iy)
    const
  {
    return get(m_algo->GetMetaMap(), m_grid->GetVertex(ix, iy));
  }
  
  
  void Facade::
  SetMeta(size_t ix, size_t iy, double meta)
  {
    m_algo->SetMeta(m_grid->GetVertex(ix, iy), meta, *(m_kernel));
  }
  
  
  void Facade::
  AddGoal(size_t ix, size_t iy, double value)
  {
    m_algo->AddGoal(m_grid->GetVertex(ix, iy), value);
  }
  
  
  void Facade::
  AddGoal(const Region & goal)
  {
    const double obstacle(m_kernel->obstacle_meta);
    const meta_map_t & meta_map(m_algo->GetMetaMap());
    for(Region::indexlist_t::const_iterator in(goal.GetArea().begin());
	in != goal.GetArea().end(); ++in){
      const vertex_t vertex(m_grid->GetVertex(in->x, in->y));
      if(get(meta_map, vertex) != obstacle)
	m_algo->AddGoal(vertex, in->r);
    }
  }
  
  
  void Facade::
  RemoveGoal(size_t ix, size_t iy)
  {
    m_algo->RemoveGoal(m_grid->GetVertex(ix, iy));
  }


  void Facade::
  RemoveGoal(const Region & goal)
  {
    for(Region::indexlist_t::const_iterator in(goal.GetArea().begin());
	in != goal.GetArea().end(); ++in)
      m_algo->RemoveGoal(m_grid->GetVertex(in->x, in->y));
  }
  
  
  void Facade::
  RemoveAllGoals()
  {
    m_algo->RemoveAllGoals();
  }
  
  
  bool Facade::
  IsGoal(size_t ix, size_t iy)
    const
  {
    return m_algo->IsGoal(m_grid->GetVertex(ix, iy));
  }
  
  
  bool Facade::
  HaveWork()
    const
  {
    return m_algo->HaveWork();
  }
  
  
  void Facade::
  ComputeOne()
  {
    m_algo->ComputeOne(*(m_kernel));
  }
  
  
  void Facade::
  DumpGrid(FILE * stream)
    const
  {
    dump_grid(*m_grid, stream);
  }
  
  
  void Facade::
  DumpQueue(FILE * stream, size_t limit)
    const
  {
    dump_queue(*m_algo, m_grid.get(), limit, stream);
  }
  

  void Facade::
  DumpPointers(FILE * stream)
    const
  {
    fprintf(stream,
	    "estar::Facade 0x%08X\n"
	    "    Algorithm 0x%08X\n"
	    "    Grid      0x%08X\n"
	    "    Kernel    0x%08X\n",
	    this, m_algo.get(), m_grid.get(), m_kernel.get());
  }
  
} // namespace estar
