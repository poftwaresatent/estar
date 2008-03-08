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


#ifndef ESTAR_NF1_KERNEL_HPP
#define ESTAR_NF1_KERNEL_HPP


#include <estar/Kernel.hpp>
#include <estar/numeric.hpp>


namespace estar {
  
  
  /** Non-interpolating kernel, mimics NF1 but allows continuous risk. */
  class NF1Kernel
    : public SubKernel<NF1Kernel>
  {
  public:
    NF1Kernel();
    
  protected:
    virtual double DoCompute(Propagator & propagator) const;
  };
  
  
  template<>
  class KernelTraits<NF1Kernel> {
  public:
    static double freespace_meta() { return 0; }
    static double obstacle_meta() { return infinity; }
  };
  
  
} // namespace estar

#endif // ESTAR_NF1_KERNEL_HPP
