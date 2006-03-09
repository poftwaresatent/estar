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


#include "cwrap.h"
#include "Algorithm.hpp"
#include "Grid.hpp"
#include "NF1Kernel.hpp"
#include "AlphaKernel.hpp"
#include "LSMKernel.hpp"
#include "dump.hpp"
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
#include <sstream>


using estar::Algorithm;
using estar::Grid;
using estar::Kernel;
using estar::NF1Kernel;
using estar::AlphaKernel;
using estar::LSMKernel;
using estar::value_map_t;
using estar::meta_map_t;
using boost::scoped_ptr;
using boost::shared_ptr;
using std::map;
using std::string;
using std::make_pair;
using std::ostringstream;


namespace local {
  
  /** Holds the set of instances required for C-wrapper of E*.
      
      \todo Replace by estar::Facade, use facade to implement C-wrapper.
  */
  class Handle {
  public:
    Handle(Algorithm * _algo, Grid * _grid, Kernel * _kernel)
      : algo(_algo), grid(_grid), kernel(_kernel),
	value_map(_algo->GetValueMap()), meta_map(_algo->GetMetaMap()) { }
    scoped_ptr<Algorithm> algo;
    scoped_ptr<Grid> grid;
    scoped_ptr<Kernel> kernel;
    const value_map_t & value_map;
    const meta_map_t & meta_map;
  };
  
  /** Utility for opaque handle access. */
  typedef map<int, shared_ptr<Handle> > handle_map_t;

}

using namespace local;


static handle_map_t handle_map;


int estar_create(const char * kernel_name,
		 unsigned int xsize,
		 unsigned int ysize,
		 double scale,
		 int connect_diagonal,
		 FILE * dbgstream)
{
  Algorithm * algo(new Algorithm());
  Grid * grid;
  if(connect_diagonal)
    grid = new Grid(*algo, xsize, ysize, estar::EIGHT_CONNECTED);
  else
    grid = new Grid(*algo, xsize, ysize, estar::FOUR_CONNECTED);
  Kernel * kernel(0);
  const string kname(kernel_name);
  if(kname == "nf1")
    kernel = new NF1Kernel();
  else if(kname == "alpha")
    kernel = new AlphaKernel(scale);
  else if(kname == "lsm")
    kernel = new LSMKernel(*grid, scale);
  else{
    if(0 != dbgstream)
      fprintf(dbgstream,
	      "ERROR in estar_create():\n"
	      "  invalid kernel_name \"%s\"\n"
	      "  known kernels: nf1, alpha, lsm\n",
	      kernel_name);
    delete grid;
    delete algo;
    return -1;
  }
  
  int handle;
  if(handle_map.empty())
    handle = 0;
  else
    handle = handle_map.rbegin()->first + 1;
  handle_map.insert(make_pair(handle, shared_ptr<Handle>(new Handle(algo,
								    grid,
								    kernel))));
  return handle;
}


void estar_destroy(int handle)
{
  handle_map_t::iterator ih(handle_map.find(handle));
  if(handle_map.end() != ih)
    handle_map.erase(ih);
}


int estar_get_freespace_meta(int handle,
			     double * freespace_meta)
{
  handle_map_t::iterator ih(handle_map.find(handle));
  if(handle_map.end() == ih)
    return -1;
  *freespace_meta = ih->second->kernel->freespace_meta;
  return 0;
}


int estar_get_obstacle_meta(int handle,
			    double * obstacle_meta)
{
  handle_map_t::iterator ih(handle_map.find(handle));
  if(handle_map.end() == ih)
    return -1;
  *obstacle_meta = ih->second->kernel->obstacle_meta;
  return 0;
}


int estar_get_value(int handle,
		    unsigned int ix,
		    unsigned int iy,
		    double * value)
{
  handle_map_t::iterator ih(handle_map.find(handle));
  if(handle_map.end() == ih)
    return -1;
  *value = get(ih->second->value_map, ih->second->grid->GetVertex(ix, iy));
  return 0;
}


int estar_get_meta(int handle,
		   unsigned int ix,
		   unsigned int iy,
		   double * meta)
{
  handle_map_t::iterator ih(handle_map.find(handle));
  if(handle_map.end() == ih)
    return -1;
  *meta = get(ih->second->meta_map, ih->second->grid->GetVertex(ix, iy));
  return 0;
}


int estar_set_meta(int handle,
		   unsigned int ix,
		   unsigned int iy,
		   double meta)
{
  handle_map_t::iterator ih(handle_map.find(handle));
  if(handle_map.end() == ih)
    return -1;
  ih->second->algo->SetMeta(ih->second->grid->GetVertex(ix, iy),
			    meta, *(ih->second->kernel));
  return 0;
}


int estar_add_goal(int handle,
		   unsigned int ix,
		   unsigned int iy,
		   double value)
{
  handle_map_t::iterator ih(handle_map.find(handle));
  if(handle_map.end() == ih)
    return -1;
  ih->second->algo->AddGoal(ih->second->grid->GetVertex(ix, iy), value);
  return 0;
}


// int estar_remove_goal(int handle,
// 		      unsigned int ix,
// 		      unsigned int iy)
// {
//   handle_map_t::iterator ih(handle_map.find(handle));
//   if(handle_map.end() == ih)
//     return -1;
//   return -2;
// }


int estar_is_goal(int handle,
		  unsigned int ix,
		  unsigned int iy)
{
  handle_map_t::iterator ih(handle_map.find(handle));
  if(handle_map.end() == ih)
    return -1;
  return ih->second->algo->IsGoal(ih->second->grid->GetVertex(ix, iy)) ? 1 : 0;
}


int estar_have_work(int handle)
{
  handle_map_t::iterator ih(handle_map.find(handle));
  if(handle_map.end() == ih)
    return -1;
  return ih->second->algo->HaveWork() ? 1 : 0;
}


int estar_compute_one(int handle)
{
  handle_map_t::iterator ih(handle_map.find(handle));
  if(handle_map.end() == ih)
    return -1;
  ih->second->algo->ComputeOne(*(ih->second->kernel));
  return 0;
}


int estar_dump_grid(int handle,
		    FILE * stream)
{
  handle_map_t::iterator ih(handle_map.find(handle));
  if(handle_map.end() == ih)
    return -1;
  dump_grid(*(ih->second->grid), stream);
  return 0;
}


int estar_dump_queue(int handle,
		     FILE * stream,
		     size_t limit)
{
  handle_map_t::iterator ih(handle_map.find(handle));
  if(handle_map.end() == ih)
    return -1;
  dump_queue(*(ih->second->algo), ih->second->grid.get(), limit, stream);
  return 0;
}

