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


#include "check.hpp"
#include "Algorithm.hpp"
#include "Grid.hpp"
#include "util.hpp"
#include "pdebug.hpp"
#include <map>
#include <iostream>
#include <sstream>


using namespace boost;
using namespace std;


namespace estar {
  
  
  bool check_queue(const Algorithm & algo, const Grid * grid,
		   const char * prefix, std::ostream & os)
  {
    ostringstream err;
    bool result(true);
    const queue_t & queue(algo.GetQueue().Get());
    const queue_map_t & queue_map(algo.GetQueue().GetMap());
    const flag_map_t & flag(algo.GetFlagMap());	// check OPEN flag
    const value_map_t & value(algo.GetValueMap()); // check key
    const rhs_map_t & rhs(algo.GetRhsMap()); // check key
    std::map<vertex_t, double> queued_vs;	// check duplicates
    for(const_queue_it iq(queue.begin()); iq != queue.end(); ++iq){
      PVDEBUG("i: %lu   k: %g\n", iq->second, iq->first);
      if( ! (get(flag, iq->second) & OPEN)){
	result = false;
	err << prefix << "  lacking OPEN flag on i: " << iq->second;
	if(0 != grid){
	  const GridNode & gn(grid->Vertex2Node(iq->second));
	  err << " (" << gn.ix << ", " << gn.iy << ")";
	}
	err << "\n";
      }
      if(minval(get(value, iq->second), get(rhs, iq->second)) != iq->first){
	result = false;
	err << prefix << "  wrong key on i: " << iq->second;
	if(0 != grid){
	  const GridNode & gn(grid->Vertex2Node(iq->second));
	  err << " (" << gn.ix << ", " << gn.iy << ")";
	}
	err << " k: " << iq->first << " v: " << get(value, iq->second)
	    << " rhs: " << get(rhs, iq->second) << "\n";
      }
      queue_map_t::const_iterator iqm(queue_map.find(iq->second));
      if(iqm == queue_map.end()){
	result = false;
	err << prefix << "  missing queue_map entry for i: " << iq->second;
	if(0 != grid){
	  const GridNode & gn(grid->Vertex2Node(iq->second));
	  err << " (" << gn.ix << ", " << gn.iy << ")";
	}
	err << "\n";
      }
      else if(iqm->second != iq->first){
	result = false;
	err << prefix << "  queue_map mismatch for i: " << iq->second;
	if(0 != grid){
	  const GridNode & gn(grid->Vertex2Node(iq->second));
	  err << " (" << gn.ix << ", " << gn.iy << ")";
	}
	err << " queue: " << iq->first
	    << " queue_map: " << iqm->second << "\n";
      }
      if(queued_vs.find(iq->second) != queued_vs.end()){
	result = false;
	err << prefix << "  duplicate i: " << iq->second;
	if(0 != grid){
	  const GridNode & gn(grid->Vertex2Node(iq->second));
	  err << " (" << gn.ix << ", " << gn.iy << ")";
	}
	err << " k: " << iq->first
	    << " previous k: " << queued_vs[iq->second] << "\n";
      }
      queued_vs.insert(make_pair(iq->second, iq->first));
    }
    const cspace_t & cspace(algo.GetCSpace());
    vertex_it vi, vend;
    for(tie(vi, vend) = vertices(cspace); vi != vend; ++vi){
      if((get(flag, *vi) & OPEN) && (queued_vs.find(*vi) == queued_vs.end())){
 	result = false;
 	err << prefix << "  " << flag_name(get(flag, *vi))
 	    << " not in queue i: ";
 	if(0 != grid){
 	  const GridNode & gn(grid->Vertex2Node(*vi));
 	  err << " (" << gn.ix << ", " << gn.iy << ")";
 	}
 	err << "\n";
      }
    }
    if( ! result)
      os << prefix << __func__ << "() log:\n" << err.str();
    else
      os << prefix << __func__ << "(): OK\n";
    return result;
  }
  
  
  bool check_cspace(const cspace_t & cspace,
		    const char * prefix, std::ostream & os)
  {
    ostringstream err;
    bool result(true);
    vertex_it vi, vend;
    for(tie(vi, vend) = vertices(cspace); vi != vend; ++vi){
      typedef map<vertex_t, size_t> nbcount_t;
      nbcount_t nbcount;
      typedef graph_traits<cspace_t>::out_edge_iterator eit_t;
      eit_t ei, eend;
      for(tie(ei, eend) = out_edges(*vi, cspace); ei != eend; ++ei){
	vertex_t nbv(target(*ei, cspace));
	nbcount_t::iterator nbci(nbcount.find(nbv));
	if(nbcount.find(nbv) != nbcount.end())
	  ++(nbci->second);
	else
	  nbcount.insert(make_pair(nbv, 1));
      }
      nbcount_t::iterator si(nbcount.find(*vi));
      if(si != nbcount.end()){
	result = false;
	err << prefix << "self-neighboring vertex: " << *vi
	    << " *" << si->second << "\n";
      }
      for(nbcount_t::iterator ni(nbcount.begin()); ni != nbcount.end(); ++ni)
	if((ni->second > 1) && (ni->first != *vi)){
	  result = false;
	  err << prefix << "duplicate neighbor: " << *vi << " => " << ni->first
	      << " *" << ni->second << "\n";
	}
    }
    if( ! result)
      os << err.str();
    return result;
  }
  
  
} // namespace estar
