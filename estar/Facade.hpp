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


#include <estar/FacadeWriteInterface.hpp>
#include <estar/FacadeReadInterface.hpp>
#include <estar/base.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <iosfwd>


namespace estar {
  
  
  class Region;

  
  /**
     High-level interface for E-star approach to smooth dynamic
     navigation functions. Hides the semantics for accessing the
     underlying Algorithm, Grid, Kernel, and associated objects.
     
     Use the Create() factory method to allocate a new Facade.
     
     \todo Provide a way to "plug-in" existing instances, eg for user
     extensions.
     
     \note We are using multiple inheritance on two completely
     abstract base classes, so we do not risk too many of the pitfalls
     involved in multiple inheritance.
  */
  class Facade
    : public FacadeWriteInterface,
      public FacadeReadInterface
  {
  public:
    /**
       \note This typedef keeps code intact that was written before
       the FacadeWriteInterface/FacadeReadInterface split.
    */
    typedef FacadeReadInterface::node_status_t node_status_t;
    
    
    const size_t xsize, ysize;
    const double scale;
    
    static Facade * Create(const std::string & kernel_name,
			   size_t xsize,
			   size_t ysize,
			   double scale,
			   int connect_diagonal,
			   FILE * dbgstream);
    
    static Facade * CreateDefault(size_t xsize,
				  size_t ysize,
				  double scale);
    
    /**
       Create a Facade from existing instances of Algorithm, Grid, and
       Kernel.
       
       \note Create() and CreateDefault() are much more convenient if
       you're creating new objects anyway.
    */
    Facade(boost::shared_ptr<Algorithm> algo,
	   boost::shared_ptr<Grid> grid,
	   boost::shared_ptr<Kernel> kernel);
    
    
    /**
       Implements FacadeReadInterface::GetXSize().
    */
    virtual size_t GetXSize() const { return xsize; }
    
    /**
       Implements FacadeReadInterface::GetYSize().
    */
    virtual size_t GetYSize() const { return ysize; }
    
    /**
       Implements FacadeReadInterface::GetScale().
    */
    virtual double GetScale() const { return scale; }
    
    /**
       Implements FacadeReadInterface::GetFreespaceMeta().
    */
    virtual double GetFreespaceMeta() const;
    
    /**
       Implements FacadeReadInterface::GetObstacleMeta().
    */
    virtual double GetObstacleMeta() const;
    
    /**
       Implements FacadeReadInterface::GetValue().
    */
    virtual double GetValue(size_t ix, size_t iy) const;
    
    /**
       Implements FacadeReadInterface::GetMeta().
    */
    virtual double GetMeta(size_t ix, size_t iy) const;
    
    /**
       Implements FacadeWriteInterface::SetMeta().
    */
    virtual void SetMeta(size_t ix, size_t iy, double meta);
    
    /**
       Implements FacadeWriteInterface::InitMeta().
    */
    virtual void InitMeta(size_t ix, size_t iy, double meta);
    
    /**
       Implements FacadeWriteInterface::AddGoal().
       
       \note Unlike AddGoal(const Region &), obstacle vertices are not
       ignored.
    */
    virtual void AddGoal(size_t ix, size_t iy, double value);
    
    /**
       An alternative to AddGoal(size_t, size_t, double) which ignores
       obstacles [vertices with meta equal to GetObstacleMeta()].
       
       \note FacadeWriteInterface has no corresponding abstract method.
    */
    void AddGoal(const Region & goal);
    
    /**
       Revert a goal cell to normal status. Unless the cell happens to
       be non-goal before calling this method, this sets a flag which
       causes the next call to ComputeOne() to reinitialize the
       navigation function and wavefront.
    */
    void RemoveGoal(size_t ix, size_t iy);
    
    /**
       Revert all cells in a goal region to normal status. Basically
       just calls RemoveGoal(size_t, size_t) for all the cells in the
       region.
    */
    void RemoveGoal(const Region & goal);
    
    /**
       Implements FacadeWriteInterface::RemoveAllGoals().
    */
    virtual void RemoveAllGoals();
    
    /**
       Implements FacadeReadInterface::IsGoal().
    */
    virtual bool IsGoal(size_t ix, size_t iy) const;
    
    /**
       Implements FacadeReadInterface::HaveWork().
    */
    virtual bool HaveWork() const;
    
    /**
       Implements FacadeWriteInterface::ComputeOne().
    */
    virtual void ComputeOne();
    
    /**
       Implements FacadeWriteInterface::Reset().
    */
    virtual void Reset();
    
    /**
       Implements FacadeReadInterface::GetStatus().
    */
    virtual node_status_t GetStatus(size_t ix, size_t iy) const;
    
    /**
       Implements FacadeReadInterface::GetLowestInconsistentValue().
    */
    virtual bool GetLowestInconsistentValue(double & value) const;
    
    void DumpGrid(FILE * stream) const;
    void DumpQueue(FILE * stream, size_t limit) const;
    void DumpPointers(FILE * stream) const;
    
    /**
       Implements FacadeReadInterface::GetAlgorithm().
    */
    virtual const Algorithm & GetAlgorithm() const { return * m_algo; }
    
    /**
       Implements FacadeReadInterface::GetGrid().
    */
    virtual const Grid & GetGrid() const { return * m_grid; }
    
    /**
       Implements FacadeReadInterface::GetKernel().
    */
    virtual const Kernel & GetKernel() const { return * m_kernel; }
    
    Algorithm & GetAlgorithm() { return * m_algo; }
    Kernel    & GetKernel()    { return * m_kernel; }
    
    
  private:
    boost::shared_ptr<Algorithm> m_algo;
    boost::shared_ptr<Grid> m_grid;
    boost::shared_ptr<Kernel> m_kernel;
  };
  
} // namespace estar

#endif // ESTAR_FACADE_HPP
