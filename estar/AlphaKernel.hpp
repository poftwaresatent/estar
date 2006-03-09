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


#ifndef ESTAR_ALPHA_KERNEL_HPP
#define ESTAR_ALPHA_KERNEL_HPP


#include <estar/Kernel.hpp>


namespace estar {
  

  /** Simple interpolating kernel. Not as powerfull as LSMKernel, but
      doesn't require the C-space graph to be a regular 4-connected
      grid.
      
      \todo TEST ME! This is just an initial implementation.
  */
  class AlphaKernel:
    public Kernel {
  public:
    const double scale;
    const double alpha;
    
    AlphaKernel(double scale);
    
  protected:
    virtual double DoCompute(Propagator & propagator) const;
  };
  
} // namespace estar

#endif // ESTAR_ALPHA_KERNEL_HPP
