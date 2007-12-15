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


#ifndef ESTAR_KERNEL_HPP
#define ESTAR_KERNEL_HPP


namespace estar {
  
  
  class Propagator;
  
  
  /**
     Things that are required of specialized  KernelTraits:
     \code
     template<>
     struct KernelTraits<FooBar> {
       static double freespace_meta() { return foo; }
       static double obstacle_meta() { return bar; }
     };
     \endcode
  */
  template<class KernelSubclass>
  struct KernelTraits;
  
  
  /**
     Generic E* kernel interface. The base class ensures some simple
     rules, such as how to react to empty Propagator instances.

     \todo Implement a factory method that allows creating a subclass
     from a parameter object or config string or something...
  */
  class Kernel
  {
  public:
    const double freespace_meta;
    const double obstacle_meta;
    const double scale;
    
    Kernel(double _freespace_meta, double _obstacle_meta, double _scale)
      : freespace_meta(_freespace_meta),
	obstacle_meta(_obstacle_meta),
	scale(_scale) {}
    
    virtual ~Kernel();
    
    double Compute(Propagator & propagator) const;
    
    /**
       Default implementation: Increasing meta leads to RAISE events.
       \todo Doesn't seem to be used anymore.
    */
    virtual bool ChangeWouldRaise(double oldmeta, double newmeta) const;
    
  protected:
    virtual
    double DoCompute(Propagator & propagator) const = 0;
  };
  
  
  template<class KernelSubclass>
  class SubKernel
    : public Kernel
  {
  public:
    typedef class KernelTraits<KernelSubclass> traits;
    
    explicit SubKernel(double scale)
      : Kernel(traits::freespace_meta(), traits::obstacle_meta(), scale) {}
  };
  
} // namespace estar

#endif // ESTAR_KERNEL_HPP
