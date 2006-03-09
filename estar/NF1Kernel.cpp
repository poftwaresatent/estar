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


#include "NF1Kernel.hpp"
#include "Propagator.hpp"
#include "numeric.hpp"


using boost::tie;


namespace estar {
  
  
  NF1Kernel::
  NF1Kernel()
    : Kernel(0, infinity)
  {
  }
  
  
  double NF1Kernel::
  DoCompute(Propagator & propagator) const
  {
    const_queue_it iq, qend;
    tie(iq, qend) = propagator.GetUpwindNeighbors();
    propagator.AddBackpointer(iq->second);
    return iq->first + propagator.GetTargetMeta();
  }
  
  
} // namespace estar
