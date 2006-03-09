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


#ifndef ESTAR_FACADE_HPP
#define ESTAR_FACADE_HPP


#include <estar/base.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>
#include <iosfwd>


namespace estar {
  
  
  class Algorithm;
  class Grid;
  class Kernel;

  
  /**
     High-level interface for E-star approach to smooth dynamic
     navigation functions. Hides the semantics for accessing the
     underlying Algorithm, Grid, Kernel, and associated objects.
     
     Use the Create() factory method to allocate a new Facade.
     
     \todo Provide a way to "plug-in" existing instances, eg for user
     extensions.
  */
  class Facade {
  private:
    Facade(Algorithm * algo, Grid * grid, Kernel * kernel, double scale);
    
  public:
    const size_t xsize, ysize;
    const double scale;
    
    static Facade * Create(const std::string & kernel_name,
			   size_t xsize,
			   size_t ysize,
			   double scale,
			   int connect_diagonal,
			   FILE * dbgstream);
    
    double GetFreespaceMeta() const;
    double GetObstacleMeta() const;
    double GetValue(size_t ix, size_t iy) const;
    double GetMeta(size_t ix, size_t iy) const;
    void SetMeta(size_t ix, size_t iy, double meta);
    void AddGoal(size_t ix, size_t iy, double value);
    void RemoveGoal(size_t ix, size_t iy);
    bool IsGoal(size_t ix, size_t iy) const;
    
    bool HaveWork() const;
    void ComputeOne();
    
    void DumpGrid(FILE * stream) const;
    void DumpQueue(FILE * stream, size_t limit) const;
    void DumpPointers(FILE * stream) const;
    
    const Algorithm   & GetAlgorithm() const { return * m_algo; }
    const Grid        & GetGrid()      const { return * m_grid; }
    const Kernel      & GetKernel()    const { return * m_kernel; }
    
    Algorithm & GetAlgorithm() { return * m_algo; }
    Kernel    & GetKernel()    { return * m_kernel; }
    
    
  private:
    boost::scoped_ptr<Algorithm> m_algo;
    boost::scoped_ptr<Grid> m_grid;
    boost::scoped_ptr<Kernel> m_kernel;
  };
  
} // namespace estar

#endif // ESTAR_FACADE_HPP
