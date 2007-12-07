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
#include "Supergrid.hpp"
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
  
  
  Facade::
  Facade(shared_ptr<Algorithm> algo,
	 shared_ptr<Grid> grid,
	 shared_ptr<Kernel> kernel)
    : scale(kernel->scale),
      m_cspace(grid->GetCSpace()),
      m_algo(algo),
      m_supergrid(new Supergrid(algo, grid->xsize, grid->ysize)),
      m_kernel(kernel)
  {
    if ( ! m_supergrid->AddGrid(0, 0, grid)) {
      cerr << "ERROR in estar::Facade::Facade(): Supergrid::AddGrid().\n";
#warning "Very bad idea for library code."
      exit(EXIT_FAILURE);
    }
  }
  
  
  Facade * Facade::
  Create(const std::string & kernel_name,
	 ssize_t xsize,
	 ssize_t ysize,
	 double scale,
	 FacadeOptions const & options,
	 FILE * dbgstream)
  {
    shared_ptr<Grid> grid;
    if (options.connect_diagonal)
      grid.reset(new Grid(xsize, ysize, EIGHT_CONNECTED));
    else
      grid.reset(new Grid(xsize, ysize, FOUR_CONNECTED));
    shared_ptr<Algorithm> algo(new Algorithm(grid->GetCSpace(),
					     options.check_upwind,
					     options.check_local_consistency,
					     options.check_queue_key,
					     options.auto_reset,
					     options.auto_flush));
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
		"ERROR in %s():\n"
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
    FacadeOptions options;
    shared_ptr<Grid> grid;
    if (options.connect_diagonal)
      grid.reset(new Grid(xsize, ysize, EIGHT_CONNECTED));
    else
      grid.reset(new Grid(xsize, ysize, FOUR_CONNECTED));
    shared_ptr<Algorithm> algo(new Algorithm(grid->GetCSpace(),
					     options.check_upwind,
					     options.check_local_consistency,
					     options.check_queue_key,
					     options.auto_reset,
					     options.auto_flush));
    shared_ptr<Kernel> kernel(new LSMKernel(grid->GetCSpace(), scale));
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
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid)
      return infinity;
    return get(m_algo->GetValueMap(), grid->Index2Vertex(ix, iy));
  }
  
  
  double Facade::
  GetMeta(ssize_t ix, ssize_t iy)
    const
  {
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid)
      return m_kernel->obstacle_meta;
    return get(m_algo->GetMetaMap(), grid->Index2Vertex(ix, iy));
  }
  
  
  void Facade::
  SetMeta(ssize_t ix, ssize_t iy, double meta)
  {
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid)
      return;		      // should create new grid and link it in
    m_algo->SetMeta(grid->Index2Vertex(ix, iy), meta, *(m_kernel));
  }
  
  
  void Facade::
  InitMeta(ssize_t ix, ssize_t iy, double meta)
  {
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid)
      return;		      // should create new grid and link it in
    m_algo->InitMeta(grid->Index2Vertex(ix, iy), meta);
  }
  
  
  void Facade::
  AddGoal(ssize_t ix, ssize_t iy, double value)
  {
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid)
      return;		      // should create new grid and link it in
    m_algo->AddGoal(grid->Index2Vertex(ix, iy), value);
  }
  
  
  void Facade::
  AddGoal(const Region & goal)
  {
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid)
      return; // should create new grid and link it in, individually
	      // for each vertex in the region
    const double obstacle(m_kernel->obstacle_meta);
    const meta_map_t & meta_map(m_algo->GetMetaMap());
    for(Region::indexlist_t::const_iterator in(goal.GetArea().begin());
	in != goal.GetArea().end(); ++in){
      const vertex_t vertex(grid->Index2Vertex(in->x, in->y));
      if(get(meta_map, vertex) != obstacle)
	m_algo->AddGoal(vertex, in->r);
    }
  }
  
  
  void Facade::
  RemoveGoal(ssize_t ix, ssize_t iy)
  {
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid)
      return;			// what if? probably safely ignore
    m_algo->RemoveGoal(grid->Index2Vertex(ix, iy));
  }


  void Facade::
  RemoveGoal(const Region & goal)
  {
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid)
      return;			// what if? probably safely ignore
    for(Region::indexlist_t::const_iterator in(goal.GetArea().begin());
	in != goal.GetArea().end(); ++in)
      m_algo->RemoveGoal(grid->Index2Vertex(in->x, in->y));
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
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid)
      return false;
    return m_algo->IsGoal(grid->Index2Vertex(ix, iy));
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
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid)
      fprintf(stream, "WARNING in Facade::DumpGrid(): no grid (bizarre)\n");
    dump_grid(*grid, stream);
  }
  
  
  void Facade::
  DumpQueue(FILE * stream, size_t limit)
    const
  {
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid)
      fprintf(stream, "WARNING in Facade::DumpQueue(): no grid (bizarre)\n");
    dump_queue(*m_algo, grid.get(), limit, stream);
  }
  

  void Facade::
  DumpPointers(FILE * stream)
    const
  {
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    fprintf(stream,
	    "estar::Facade %p\n"
	    "    Algorithm %p\n"
	    "    Grid      %p\n"
	    "    Kernel    %p\n",
	    this, m_algo.get(), grid.get(), m_kernel.get());
  }
  
  
  Facade::node_status_t Facade::
  GetStatus(ssize_t ix, ssize_t iy) const
  {
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid)
      return OUT_OF_GRID;
    if((ix >= grid->xsize) || (iy >= grid->ysize))
      return OUT_OF_GRID;
    return GetStatus(grid->Index2Vertex(ix, iy));
  }
  
  
  Facade::node_status_t Facade::
  GetStatus(vertex_t vertex) const
  {
    const flag_t flag(get(m_algo->GetFlagMap(), vertex));
    if(estar::GOAL == flag)
      return GOAL;
    if((OPEN == flag) || (OPNG == flag))
      return WAVEFRONT;
    // could be paranoid and assert (NONE == flag) here
    if(get(m_algo->GetMetaMap(), vertex) == m_kernel->obstacle_meta)
      return OBSTACLE;
    const queue_t & queue(m_algo->GetQueue().Get());
    if(queue.empty())
      return UPWIND;
    const double value(get(m_algo->GetValueMap(), vertex));
    if(value < queue.begin()->first)
      return UPWIND;
    if(value >= queue.rbegin()->first)
      return DOWNWIND;
    return WAVEFRONT;
  }
  
  
  bool Facade::
  GetLowestInconsistentValue(double & value) const
  {
    const queue_t & queue(m_algo->GetQueue().Get());
    if(queue.empty())
      return false;
    value = queue.begin()->first;
    return true;
  }
  
  
  void Facade::
  Reset()
  {
    m_algo->Reset();
  }
  
  
  const Grid & Facade::
  GetGrid() const
  {
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid) {
      cerr << "ERROR in estar::Facade::GetGrid(): no grid\n";
#warning "Very bad idea for library code."
      exit(EXIT_FAILURE);
    }
    return *grid;
  }


  bool Facade::
  IsValidIndex(ssize_t ix, ssize_t iy) const
  {
    shared_ptr<Grid> grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid)
      return false;
    return (ix < grid->GetXSize()) && (iy < grid->GetYSize());
  }
  
  
  int Facade::
  TraceCarrot(double robot_x, double robot_y,
	      double distance, double stepsize,
	      size_t maxsteps,
	      carrot_trace & trace) const
  {
    PVDEBUG("(%g   %g)   d: %g   s: %g   N: %lu\n",
	    robot_x, robot_y, distance, stepsize, maxsteps);
    if((robot_x < 0) || (robot_y < 0)){
      PDEBUG("FAIL (robot_x < 0) || (robot_y < 0)\n");
      return -1;
    }
    robot_x /= scale;
    robot_y /= scale;
    distance /= scale;
    const double unscaled_stepsize(stepsize);
    stepsize /= scale;
    PVDEBUG("scaled: (%g   %g)   d: %g   s: %g\n",
	   robot_x, robot_y, distance, stepsize);
    ssize_t ix(static_cast<ssize_t>(rint(robot_x)));
    ssize_t iy(static_cast<ssize_t>(rint(robot_y)));
    const Supergrid::const_subgrid_ptr grid(m_supergrid->GetGrid(0, 0));
    if ( ! grid) {
      PDEBUG("FAIL: no (sub)grid\n");
      return -17;
    }
    if((ix >= grid->GetXSize()) || (iy >= grid->GetYSize())){
      PDEBUG("FAIL (ix >= grid->GetXSize()) || (iy >= grid->GetYSize())\n");
      return -1;
    }
    
    if((grid->connect != FOUR_CONNECTED)
       && (grid->connect != EIGHT_CONNECTED)){
      PDEBUG("TODO: Implement carrot for hexgrids!\n");
      return -2;
    }
    const ssize_t max_ix(grid->GetXSize() - 1);
    const ssize_t max_iy(grid->GetYSize() - 1);
    
    trace.clear();
    double cx(robot_x);		// carrot
    double cy(robot_y);
    size_t ii;
    for(ii = 0; ii < maxsteps; ++ii){
      const double value(GetValue(ix, iy));
      double dx, dy, gx, gy;
      const int res(grid->ComputeStableScaledGradient(ix, iy, stepsize,
						      gx, gy, dx, dy));
      if(0 == res)
	trace.push_back(carrot_item(cx * scale,
				    cy * scale,
				    gx / scale,
				    gy / scale,
				    value,
				    false));
      else
	trace.push_back(carrot_item(cx * scale,
				    cy * scale,
				    dx / stepsize,
				    dy / stepsize,
				    value,
				    true));
      cx -= dx;
      cy -= dy;
      PVDEBUG("(%g   %g) ==> (%g   %g)%s\n",
	      dx, dy, cx, cy, (0 != res) ? "[heuristic]" : "");
      
      if(sqrt(square(robot_x - cx) + square(robot_y - cy)) >= distance){
	PVDEBUG("... >= distance");
	break;
      }
      if(value <= unscaled_stepsize){
	PVDEBUG("... value <= unscaled_stepsize");
	break;
      }
      
      ix = boundval<ssize_t>(0, static_cast<ssize_t>(rint(cx)), max_ix);
      iy = boundval<ssize_t>(0, static_cast<ssize_t>(rint(cy)), max_iy);
    }
    // add final point to the trace
    {
      double dx, dy, gx, gy;
      const int res(grid->ComputeStableScaledGradient(ix, iy, stepsize,
						      gx, gy, dx, dy));
      if(0 == res)
	trace.push_back(carrot_item(cx * scale,
				    cy * scale,
				    gx / scale,
				    gy / scale,
				    GetValue(ix, iy),
				    false));
      else
	trace.push_back(carrot_item(cx * scale,
				    cy * scale,
				    dx / stepsize,
				    dy / stepsize,
				    GetValue(ix, iy),
				    true));
    }
    
    if(ii >= maxsteps){
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

} // namespace estar
