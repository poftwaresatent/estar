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


#include "AlphaKernel.hpp"
#include "Propagator.hpp"
#include "util.hpp"
#include "pdebug.hpp"


namespace estar {
  
  
  AlphaKernel::
  AlphaKernel(double scale)
    : SubKernel<AlphaKernel>(scale),
      alpha(2.0)
  {
  }
  
  
  double AlphaKernel::
  DoCompute(Propagator & propagator) const
  {
    const double meta(propagator.GetTargetMeta());
    if(meta == infinity){
      PVDEBUG("OBSTACLE\n", meta);
      return infinity;
    }
    const_queue_it iq, qend;
    boost::tie(iq, qend) = propagator.GetUpwindNeighbors();
    propagator.AddBackpointer(iq->second);
    const double v1(iq->first);
    const double tmax(v1 + alpha * scale * meta);
    vdebugos dbg;
    dbg << "\n  meta: " << meta	<< "   prim i: " << iq->second << " v: " << v1
	<< "   tmax: " << tmax;
    ++iq;
    if(iq == qend){
      PVDEBUG("NO SECONDARY, fb: %g%s\n", tmax, dbg.str().c_str());
      return tmax;
    }
    const double v2(iq->first);
    const double tnonfb(v1 + meta*meta * (2*scale+v2-v1) / (1+meta));
    dbg	<< "\n  sec i: " << iq->second << " v: " << v2
	<< "   tnonfb: " << tnonfb;
    if(tnonfb > tmax){
      dbg << " > tmax";
      PVDEBUG("INVALID secondary, fb: %g%s\n", tmax, dbg.str().c_str());
      return tmax;
    }
    propagator.AddBackpointer(iq->second); // IMPORTANT!
    PVDEBUG("SUCCESS %g%s\n", tnonfb, dbg.str().c_str());
    return tnonfb;
  }
  
} // namespace estar
