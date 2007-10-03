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


#ifndef ESTAR_LSM_KERNEL_HPP
#define ESTAR_LSM_KERNEL_HPP


#include <estar/Kernel.hpp>
#include <cstddef>


namespace estar {
  
  
  class Grid;
  
  
  /** Interpolation kernel based on Level Set Method. Needs a
      4-connected regular grid as C-space, ie an instance of Grid with
      connectedness_t FOUR_CONNECTED. */
  class LSMKernel:
    public Kernel {
  public:
    LSMKernel(const Grid & grid, double scale);
    
    virtual bool ChangeWouldRaise(double oldmeta, double newmeta) const;
    
  protected:
    virtual double DoCompute(Propagator & propagator) const;
    
    const Grid & m_grid;
  };
  
  
} // namespace estar

#endif // ESTAR_LSM_KERNEL_HPP
