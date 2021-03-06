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


#include "LSMKernel.hpp"
#include "GridNode.hpp"
#include "Propagator.hpp"
#include "numeric.hpp"
#include "util.hpp"
#include "pdebug.hpp"


using namespace boost;


namespace estar {
  
  
  LSMKernel::
  LSMKernel(shared_ptr<GridCSpace const> cspace, double scale)
    : SubKernel<LSMKernel>(scale),
      m_cspace(cspace)
  {
  }
  
  
  bool LSMKernel::
  ChangeWouldRaise(double oldmeta, double newmeta) const
  {
    return newmeta < oldmeta;
  }
  
  
  double LSMKernel::
  DoCompute(Propagator & propagator) const
  {
    vdebugos dbg;
    
    double const target_meta(propagator.GetTargetMeta());
    if(target_meta <= epsilon){	// Check for obstacles (numeric stability)!
      PVDEBUG("OBSTACLE: target_meta == %g <= epsilon\n", target_meta);
      return infinity;
    }
    
    // radius of the LSM's geometric interpretation
    double const radius(scale / target_meta);
    
    const_queue_it iq, qend;
    tie(iq, qend) = propagator.GetUpwindNeighbors();
    
    // There's always a primary propagator and thus at least one
    // backpointer. This is ensured by Kernel::Compute() before
    // calling this method.
    const_queue_it const primary(iq);
    propagator.AddBackpointer(primary->second);
    shared_ptr<GridNode const> const
      primary_node(m_cspace->Lookup(primary->second));
    double const primary_value(primary->first);
    
    dbg << "\n  target_meta: " << target_meta
	<< "\n  radius: " << radius
	<< "\n  primary_node: " << *primary_node
	<< "\n  primary_value: " << primary_value;
    
    // Search for a secondary backpointer, which has to lie along
    // another axis than the primary. Use the fallback solution if no
    // such upwind neighbor exists.
    shared_ptr<GridNode const> const
      target_node(m_cspace->Lookup(propagator.GetTargetVertex()));
    bool const primary_is_along_x(target_node->ix != primary_node->ix);
    for (++iq; iq != qend; ++iq) {
      shared_ptr<GridNode const> const
	secondary_node(m_cspace->Lookup(iq->second));
      bool const secondary_is_along_x(target_node->ix != secondary_node->ix);
      if(primary_is_along_x ^ secondary_is_along_x)
	break;
    }
    if (iq == qend) {
      PVDEBUG("NO SECONDARY, fb: %g + %g == %g%s\n",
	      primary_value, radius, primary_value + radius,
	      dbg.str().c_str());
      return primary_value + radius;
    }
    const_queue_it const secondary(iq);
    double const secondary_value(secondary->first);
    dbg	<< "\n  secondary_node: " << secondary->second
	<< "\n  secondary_value: " << secondary_value;
    
    // Check if we can interpolate. In terms of solving the quadratic
    // equation: Is there a real solution that satisfies T > T_C ?
    if (radius <= secondary_value - primary_value) {
      PVDEBUG("INVALID SECONDARY %g <= %g - %g == %g fb: %g%s\n",
	      radius, secondary_value, primary_value,
	      secondary_value - primary_value, primary_value + radius,
	      dbg.str().c_str());
      return primary_value + radius;
    }
    
    // The secondary propagator is valid, add it to the backpointers.
    propagator.AddBackpointer(secondary->second);
    
    // Solve (T-T_A)^2 + (T-T_C)^2 = (h/F)^2
    // The higher of the two solutions is the one where T > T_C
    double const b(primary_value + secondary_value);
    double const c((square(primary_value)
		    + square(secondary_value)
		    - square(radius)) / 2);
    double const root(square(b) - 4 * c);
    
    // "The math" ensures that root >= 0
    double const result((b + sqrt(root)) / 2);
    
    PVDEBUG("SUCCESS %g%s\n", result, dbg.str().c_str());
    
    return result;
  }
  
} // namespace estar
