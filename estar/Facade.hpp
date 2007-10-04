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
     underlying Algorithm, Grid, Kernel, and associated objects. The
     Facade is specialized for grid-based environment representations.
     
     The recommended way to create and initialize a new Facade is to
     use one of the factory methods Create() or CreateDefault(). You
     can also allocate and initialize separate Algorithm, Grid, and
     Kernel objects and wrap a Facade around them.
     
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
    
    
    const size_t xsize;	///< number of cells along X of the underlying grid
    const size_t ysize;	///< number of cells along Y of the underlying grid
    const double scale; ///< edge length of square cells in the grid
    
    
    /**
       Create a Facade and make it ready to be used. Most people can
       call CreateDefault() instead. Typically, the first things you
       will do with a freshly created Facade are:
       
       -# Initialize traversability information using
          InitMeta(). However, once you have started propagating, you
          should not use InitMeta() anymore. Use SetMeta() instead.
       -# Set the goal(s) by calling AddGoal().
       -# Repeatedly call ComputeOne(), HaveWork(), and/or GetStatus()
          to compute the navigation function until you reach the robot
          or until the whole grid has been propagated.
       -# Use trace_carrot() (defined in util.hpp) to compute a path
          to the goal; you can also use
          compute_stable_scaled_gradient() or the more basic
          Grid::ComputeGradient() to devise a control that, at each
          timestep, requires just the "best" direction to the goal.
       
       These steps interweave quite easily with updates to the
       traversability information, because SetMeta() will create
       appropriate entries on the wavefront Queue. However, when
       multi-threading you have to create your own mutexes and such
       (for an example, see the asl-mcontrol code that is provided
       with the estar-devkit project, available from
       http://estar.sourceforge.net/).
       
       \return A fresh Facade instance, or null if something went
       wrong, in which case a message will have been written to
       dbgstream unless you set that to null.
       
       \note If you're wondering why we use FILE * instead of
       std::ostream and int instead of bool: this makes it much easier
       to implement a C-wrapper and does not cost the C++ programmer
       too much.
    */
    static Facade *
    Create(/** Name of the Kernel subclass to use. Currently accepts
	       "nf1" for NF1Kernel, "alpha" for AlphaKernel, and "lsm"
	       for LSMKernel. The latter is preferred. */
	   const std::string & kernel_name,
	   /** dimension along X of the grid */
	   size_t xsize,
	   /** dimension along Y of the grid */
	   size_t ysize,
	   /** size of the square grid cells */
	   double scale,
	   /** if true (non-zero), then the grid will be 8-connected,
	       which might be appropriate in some cases. However, for
	       the preferred LSMKernel, you should set this to zero in
	       order to create a 4-connected grid. */
	   int connect_diagonal,
	   /** If non-null, error messages are written to this
	       stream. This currently only happens when you specify an
	       invalid kernel_name. */
	   FILE * dbgstream);
    
    /** Like Create(), but doesn't give you a choice of Kernel (it's
	always LSMKernel) or of diagonally connected cells (that's
	always disabled). */
    static Facade *
    CreateDefault(size_t xsize,
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
       Implements FacadeReadInterface::GetMeta(). See also
       Algorithm::SetMeta() for more details.
    */
    virtual double GetMeta(size_t ix, size_t iy) const;
    
    /**
       Implements FacadeWriteInterface::SetMeta(). See also
       Algorithm::SetMeta() for more details.
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
    
    /**
       Write the underlying Grid to a stream. Actually just calls
       dump_grid() in dump.hpp, so if you want finer control have a
       look at what else is offered there.
    */
    void DumpGrid(FILE * stream) const;
    
    /**
       Write the contents of the wavefront Queue to a stream. Actually
       just calls dump_queue() in dump.hpp, look there for more
       options.
    */
    void DumpQueue(FILE * stream,
		   /** if >0 then output stops after that number of
		       entries (the queue can get really long even in
		       medium-sized grids) */
		   size_t limit) const;
    
    /**
       Write the addresses of the Algorithm, Grid, and Kernel
       instances. This is mostly for debugging, for moments when
       you're doubting that the right objects get called...
    */
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
    
    /**
       \return A non-const reference to the underlying Algorithm.

       \note Should only be necessary if you're experimenting with new
       things to put into Facade, so consider extending this class
       once you're done.
    */
    Algorithm & GetAlgorithm() { return * m_algo; }
    
    /**
       \return A non-const reference to the underlying Kernel.

       \note Should only be necessary if you're experimenting with new
       things to put into Facade, so consider extending this class
       once you're done.
    */
    Kernel & GetKernel() { return * m_kernel; }
    
    
  private:
    boost::shared_ptr<Algorithm> m_algo;
    boost::shared_ptr<Grid> m_grid;
    boost::shared_ptr<Kernel> m_kernel;
  };
  
} // namespace estar

#endif // ESTAR_FACADE_HPP
