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
#include "NF1Kernel.hpp"
#include "AlphaKernel.hpp"
#include "LSMKernel.hpp"
#include "dump.hpp"
#include "Region.hpp"
#include "pdebug.hpp"
#include "CSpace.hpp"

#include <iostream>		// rfct


using namespace boost;
using namespace std;		// rfct


namespace estar {
  
  
  GridOptions::
  GridOptions(ssize_t _xbegin, ssize_t _xend,
	      ssize_t _ybegin, ssize_t _yend,
	      Grid::neighborhood_t _neighborhood)
    : xbegin(_xbegin),
      xend(_xend),
      ybegin(_ybegin),
      yend(_yend),
      neighborhood(_neighborhood)
  {
  }
  
  
  AlgorithmOptions::
  AlgorithmOptions()
    : check_upwind(false),
      check_local_consistency(false),
      check_queue_key(false),
      auto_reset(false),
      auto_flush(false)
  {
  }
  
  
  AlgorithmOptions::
  AlgorithmOptions(bool _check_upwind,
		   bool _check_local_consistency,
		   bool _check_queue_key,
		   bool _auto_reset,
		   bool _auto_flush)
    : check_upwind(_check_upwind),
      check_local_consistency(_check_local_consistency),
      check_queue_key(_check_queue_key),
      auto_reset(_auto_reset),
      auto_flush(_auto_flush)
  {
  }
  
  
  Facade::
  Facade(shared_ptr<Algorithm> algo,
	 shared_ptr<Grid> grid,
	 shared_ptr<Kernel> kernel)
    : scale(kernel->scale),
      m_cspace(grid->GetCSpace()),
      m_algo(algo),
      m_grid(grid),
      m_kernel(kernel)
  {
  }
  
  
  Facade * Facade::
  Create(const std::string & kernel_name,
	 double scale,
	 GridOptions const & grid_options,
	 AlgorithmOptions const & algo_options,
	 FILE * dbgstream)
  {
    double init_meta;
    if (kernel_name == "nf1")
      init_meta = KernelTraits<NF1Kernel>::freespace_meta();
    else if (kernel_name == "alpha")
      init_meta = KernelTraits<AlphaKernel>::freespace_meta();
    else if (kernel_name == "lsm")
      init_meta = KernelTraits<LSMKernel>::freespace_meta();
    else {
      if(0 != dbgstream)
	fprintf(dbgstream,
		"ERROR in %s():\n"
		"  invalid kernel_name \"%s\"\n"
		"  known kernels: nf1, alpha, lsm\n",
		__FUNCTION__, kernel_name.c_str());
      return 0;
    }
    
    shared_ptr<Grid>
      grid(new Grid(grid_options.xbegin,
		    grid_options.xend,
		    grid_options.ybegin,
		    grid_options.yend,
		    grid_options.neighborhood,
		    init_meta));
    shared_ptr<Algorithm>
      algo(new Algorithm(grid->GetCSpace(),
			 algo_options.check_upwind,
			 algo_options.check_local_consistency,
			 algo_options.check_queue_key,
			 algo_options.auto_reset,
			 algo_options.auto_flush));
    shared_ptr<Kernel> kernel;
    if (kernel_name == "nf1")
      kernel.reset(new NF1Kernel());
    else if (kernel_name == "alpha")
      kernel.reset(new AlphaKernel(scale));
    else if (kernel_name == "lsm")
      kernel.reset(new LSMKernel(grid->GetCSpace(), scale));
    else {
      if(0 != dbgstream)
	fprintf(dbgstream,
		"BUG in %s() [should have checked this at start of method]:\n"
		"  invalid kernel_name \"%s\"\n"
		"  known kernels: nf1, alpha, lsm\n",
		__FUNCTION__, kernel_name.c_str());
      return 0;
    }
    
    return new Facade(algo, grid, kernel);
  }
  
  
  Facade * Facade::
  CreateDefault(ssize_t xsize,
		ssize_t ysize,
		double scale)
  {
    GridOptions grid_options(0, xsize, 0, ysize);
    shared_ptr<Grid>
      grid(new Grid(grid_options.xbegin,
		    grid_options.xend,
		    grid_options.ybegin,
		    grid_options.yend,
		    grid_options.neighborhood,
		    KernelTraits<LSMKernel>::freespace_meta()));
    AlgorithmOptions algo_options;
    shared_ptr<Algorithm>
      algo(new Algorithm(grid->GetCSpace(),
			 algo_options.check_upwind,
			 algo_options.check_local_consistency,
			 algo_options.check_queue_key,
			 algo_options.auto_reset,
			 algo_options.auto_flush));
    shared_ptr<Kernel>
      kernel(new LSMKernel(grid->GetCSpace(), scale));
    return new Facade(algo, grid, kernel);
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
  GetValue(ssize_t ix, ssize_t iy)
    const
  {
    shared_ptr<GridNode const> node(m_grid->GetNode(ix, iy));
    if ( ! node)
      return infinity;
    return m_cspace->GetValue(node->vertex);
  }
  
  
  double Facade::
  GetMeta(ssize_t ix, ssize_t iy)
    const
  {
    shared_ptr<GridNode const> node(m_grid->GetNode(ix, iy));
    if ( ! node)
      return m_kernel->obstacle_meta;
    return m_cspace->GetMeta(node->vertex);
  }
  
  
  bool Facade::
  SetMeta(ssize_t ix, ssize_t iy, double meta)
  {
    shared_ptr<GridNode const> node(m_grid->GetNode(ix, iy));
    if ( ! node)
      return false;		// automatic growing strategy?
    m_algo->SetMeta(node->vertex, meta, *(m_kernel));
    return true;
  }
  
  
  bool Facade::
  AddGoal(ssize_t ix, ssize_t iy, double value)
  {
    shared_ptr<GridNode const> node(m_grid->GetNode(ix, iy));
    if ( ! node)
      return false;		// automatic growing strategy?
    m_algo->AddGoal(node->vertex, value);
    return true;
  }
  
  
  void Facade::
  AddGoal(const Region & goal)
  {
    for (Region::indexlist_t::const_iterator in(goal.GetArea().begin());
	 in != goal.GetArea().end(); ++in) {
      shared_ptr<GridNode const> node(m_grid->GetNode(in->x, in->y));
      if (node && (m_cspace->GetMeta(node->vertex) != m_kernel->obstacle_meta))
	m_algo->AddGoal(node->vertex, in->r);
    }
  }
  
  
  void Facade::
  RemoveGoal(ssize_t ix, ssize_t iy)
  {
    shared_ptr<GridNode const> node(m_grid->GetNode(ix, iy));
    if ( ! node)
      return;
    m_algo->RemoveGoal(node->vertex);
  }


  void Facade::
  RemoveGoal(const Region & goal)
  {
    for(Region::indexlist_t::const_iterator in(goal.GetArea().begin());
	in != goal.GetArea().end(); ++in) {
      shared_ptr<GridNode const> node(m_grid->GetNode(in->x, in->y));
      if (node)
	m_algo->RemoveGoal(node->vertex);
    }
  }
  
  
  void Facade::
  RemoveAllGoals()
  {
    m_algo->RemoveAllGoals();
  }
  
  
  bool Facade::
  IsGoal(ssize_t ix, ssize_t iy)
    const
  {
    shared_ptr<GridNode const> node(m_grid->GetNode(ix, iy));
    if ( ! node)
      return false;
    return m_algo->IsGoal(node->vertex);
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
    m_algo->ComputeOne(*(m_kernel), scale / 10000);
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
	    "estar::Facade %p\n"
	    "    Algorithm %p\n"
	    "    Grid      %p\n"
	    "    Kernel    %p\n",
	    this, m_algo.get(), m_grid.get(), m_kernel.get());
  }
  
  
  Facade::node_status_t Facade::
  GetStatus(ssize_t ix, ssize_t iy) const
  {
    shared_ptr<GridNode const> node(m_grid->GetNode(ix, iy));
    if ( ! node)
      return OUT_OF_GRID;
    return GetStatus(node->vertex);
  }
  
  
  Facade::node_status_t Facade::
  GetStatus(vertex_t vertex) const
  {
    flag_t const flag(m_cspace->GetFlag(vertex));
    if (estar::GOAL == flag)
      return GOAL;
    if ((OPEN == flag) || (OPNG == flag))
      return WAVEFRONT;
    // could be paranoid and assert (NONE == flag) here
    if (m_cspace->GetMeta(vertex) == m_kernel->obstacle_meta)
      return OBSTACLE;
    const queue_t & queue(m_algo->GetQueue().Get());
    if (queue.empty())
      return UPWIND;
    double const value(m_cspace->GetValue(vertex));
    if (value < queue.begin()->first)
      return UPWIND;
    if (value >= queue.rbegin()->first)
      return DOWNWIND;
    return WAVEFRONT;
  }
  
  
  bool Facade::
  GetLowestInconsistentValue(double & value) const
  {
    queue_t const & queue(m_algo->GetQueue().Get());
    if (queue.empty())
      return false;
    value = queue.begin()->first;
    return true;
  }
  
  
  void Facade::
  Reset()
  {
    m_algo->Reset();
  }
  
  
  shared_ptr<Grid const> Facade::
  GetGrid() const
  {
    return m_grid;
  }
  
  
  bool Facade::
  IsValidIndex(ssize_t ix, ssize_t iy) const
  {
    shared_ptr<GridNode const> node(m_grid->GetNode(ix, iy));
    if (node)
      return true;
    return false;
  }
  
  
  int Facade::
  TraceCarrot(double robot_x, double robot_y,
	      double distance, double stepsize,
	      size_t maxsteps,
	      carrot_trace & trace,
	      std::ostream * err_os) const
  {
    PVDEBUG("(%g   %g)   d: %g   s: %g   N: %lu\n",
	    robot_x, robot_y, distance, stepsize, maxsteps);
    
    robot_x /= scale;
    robot_y /= scale;
    distance /= scale;
    const double unscaled_stepsize(stepsize);
    stepsize /= scale;
    PVDEBUG("scaled: (%g   %g)   d: %g   s: %g\n",
	    robot_x, robot_y, distance, stepsize);
    ssize_t ix(static_cast<ssize_t>(rint(robot_x)));
    ssize_t iy(static_cast<ssize_t>(rint(robot_y)));
    shared_ptr<GridNode const> node(m_grid->GetNode(ix, iy));
    if ( ! node) {
      PDEBUG("no node at (ix, iy)\n");
      if (err_os)
	(*err_os) << "ERROR (-1) in estar::Facade::TraceCarrot():\n"
		  << "  no node at [" << ix << "  " << iy << "]\n";
      return -1;
    }
    
    trace.clear();
    double cx(robot_x);		// carrot
    double cy(robot_y);
    size_t ii;
    for (ii = 0; ii < maxsteps; ++ii) {
      double const value(m_cspace->GetValue(node->vertex));
      double dx, dy, gx, gy;
      int const res(m_grid->ComputeStableScaledGradient(node, stepsize,
							gx, gy, dx, dy));
      if (0 > res) {
	PDEBUG("ComputeStableScaledGradient() failed\n");
	if (err_os)
	  (*err_os) << "ERROR (-2) in estar::Facade::TraceCarrot():\n"
		    << "  ComputeStableScaledGradient() failed\n"
		    << "  at node [" << node->ix << "  " << node->iy << "]\n";
	return -2;
      }
      bool const heur(0 != res);
      trace.push_back(carrot_item(cx * scale,
				  cy * scale,
				  gx / scale,
				  gy / scale,
				  value,
				  heur));
      
      cx -= dx;
      cy -= dy;
      PVDEBUG("(%g   %g) ==> (%g   %g)%s\n",
	      dx, dy, cx, cy, heur ? "[heuristic]" : "");
      
      if (sqrt(square(robot_x - cx) + square(robot_y - cy)) >= distance) {
	PVDEBUG("... >= distance");
	break;
      }
      if (value <= unscaled_stepsize) {
	PVDEBUG("... value <= unscaled_stepsize");
	break;
      }
      
      ssize_t const nix(static_cast<ssize_t>(rint(cx)));
      ssize_t const niy(static_cast<ssize_t>(rint(cy)));
      if ((nix != ix) || (niy != iy)) {
	ix = nix;
	iy = niy;
	node = m_grid->GetNode(ix, iy);
	if ( ! node) {
#warning "this seems to happen 'sometimes' with growable cspace"
	  PDEBUG("no node at (nix, niy)\n");
	  if (err_os)
	    (*err_os) << "ERROR (-3) in estar::Facade::TraceCarrot():\n"
		      << "  no neighbor at [" << ix << "  " << iy << "]\n";
	  return -3;
	}
      }
    }
    // add final point to the trace
    // why an extra block???
    {
      double const value(m_cspace->GetValue(node->vertex));
      double dx, dy, gx, gy;
      int const res(m_grid->ComputeStableScaledGradient(node, stepsize,
							gx, gy, dx, dy));
      
      if (0 > res) {
	PDEBUG("ComputeStableScaledGradient() failed\n");
	if (err_os)
	  (*err_os) << "ERROR (-4) in estar::Facade::TraceCarrot():\n"
		    << "  ComputeStableScaledGradient() failed\n"
		    << "  at node [" << node->ix << "  " << node->iy << "]\n";
	return -4;
      }
      bool const heur(0 != res);
      trace.push_back(carrot_item(cx * scale,
				  cy * scale,
				  gx / scale,
				  gy / scale,
				  value,
				  heur));
    }
    
    if (ii >= maxsteps) {
      PVDEBUG("WARNING (ii >= maxsteps)\n");
      return 1;
    }
    PVDEBUG("success: %g   %g\n", cx * scale, cy * scale);
    return 0;
  }
  
  
  shared_ptr<GridCSpace const> Facade::
  GetCSpace() const
  {
    return m_cspace;
  }
  
  
  size_t Facade::
  AddRange(ssize_t xbegin, ssize_t xend,
	   ssize_t ybegin, ssize_t yend,
	   double meta)
  {
    return m_grid->AddRange(xbegin, xend, ybegin, yend, meta,
			    *m_algo, *m_kernel);
  }
  
  
  size_t Facade::
  AddRange(ssize_t xbegin, ssize_t xend,
	   ssize_t ybegin, ssize_t yend,
	   Grid::get_meta const * gm)
  {
    return m_grid->AddRange(xbegin, xend, ybegin, yend, gm,
			    *m_algo, *m_kernel);
  }

  
  bool Facade::
  AddNode(ssize_t ix, ssize_t iy, double meta)
  {
    return m_grid->AddNode(ix, iy, meta, *m_algo, *m_kernel);
  }
  
  
  bool Facade::
  HaveGoal() const
  {
    return m_algo->HaveGoal();
  }

} // namespace estar
