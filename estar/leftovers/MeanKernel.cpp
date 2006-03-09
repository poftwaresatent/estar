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


#include "MeanKernel.hpp"
#include "Propagator.hpp"
#include "numeric.hpp"


using boost::tie;


#undef DEBUG_ESTAR_MEAN_KERNEL
#ifdef DEBUG_ESTAR_MEAN_KERNEL
#include <iostream>
#endif // DEBUG_ESTAR_MEAN_KERNEL


namespace estar {
  
  
  MeanKernel::
  MeanKernel(std::size_t nmax)
    : Kernel(0, infinity),
      m_nmax(nmax)
  {
  }
  
  
  double MeanKernel::
  DoCompute(Propagator & propagator) const
  {
#ifdef DEBUG_ESTAR_MEAN_KERNEL
    std::cout << "DEBUG MeanKernel:";
#endif // DEBUG_ESTAR_MEAN_KERNEL

    double sum(0);
    const_queue_it iq, qend;
    for(tie(iq, qend) = propagator.GetUpwindNeighbors(); iq != qend; ++iq){
      const double nval(iq->first);
#ifdef DEBUG_ESTAR_MEAN_KERNEL
      std::cout << " " << nval;
#endif // DEBUG_ESTAR_MEAN_KERNEL
      
      sum += nval;
      propagator.AddBackpointer(iq->second);
      if(propagator.GetNBackpointers() >= m_nmax)
	break;
    }
    
#ifdef DEBUG_ESTAR_MEAN_KERNEL
    std::cout << "\n  sum: " << sum
	      << " N: " << propagator.GetNBackpointers()
	      << " res: "
	      << propagator.GetTargetMeta()
                 + sum / propagator.GetNBackpointers() << "\n";
#endif // DEBUG_ESTAR_MEAN_KERNEL
    
    return propagator.GetTargetMeta() + sum / propagator.GetNBackpointers();
  }
  
  
} // namespace estar
