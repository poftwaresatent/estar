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
#include <estar/CSpace.hpp>
#include <estar/Grid.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <iosfwd>


namespace pnf {			// rfct
  class Flow;
}


namespace estar {
  
  
  class Region;
  
  
  class GridOptions
  {
  public:
    GridOptions(ssize_t xbegin, ssize_t xend,
		ssize_t ybegin, ssize_t yend,
		double init_meta = 0,
		Grid::neighborhood_t neighborhood = Grid::FOUR);
    
    ssize_t xbegin, xend, ybegin, yend;
    double init_meta;
    
    /** For the preferred LSMKernel, you should set this to
	Grid::FOUR. */
    Grid::neighborhood_t neighborhood;
  };
  
  
  class AlgorithmOptions
  {
  public:
    /** Default: no special propagator checks, no
	automatic resets or flushing. */
    AlgorithmOptions();
    
    AlgorithmOptions(bool check_upwind,
		     bool check_local_consistency,
		     bool check_queue_key,
		     bool auto_reset,
		     bool auto_flush);
    
    /** Whether to check the upwind structure when computing the
	propagator set of a node. Passed to PropagatorFactory ctor via
	Algorithm ctor. Use false to mimic old behavior. */
    bool check_upwind;
    
    /** Whether to check for local consistency (rhs==value) when
	computing the propagator set of a node. Passed to
	PropagatorFactory ctor via Algorithm ctor. Use false to mimic
	old behavior. */
    bool check_local_consistency;
    
    /** Whether to check if a neighbor lies below the wavefront when
	computing the propagator set of a node. Passed to
	PropagatorFactory ctor via Algorithm ctor. Use false to mimic
	old behavior */
    bool check_queue_key;
    
    /** Whether to automatically reset the whole E* computations when
	meta information changes. This is usually not needed, as the
	algorithm is designed to copy with exactly that, but this flag
	makes it easy to verify that the propagation result is the
	same. Passed to Algorithm ctor. Use false unless you have a
	good reason for wasting processing power. */
    bool auto_reset;
    
    /** Whether to automatically propagate until the entire queue has
	been emptied each time you call Algorithm::ComputeOne() or
	Facade::ComputeOne() or similar. This is usually a waste of
	resources, because you only need to propagate until the
	wavefront has passed over the robot's current location, which
	can be checked by calling Facade::GetStatus(). Use false
	unless you have a good reason for wasting processing power */
    bool auto_flush;
  };
  
  
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
       -# Use TraceCarrot() to compute a path to the goal; you can
          also use Grid::ComputeStableScaledGradient() or the more
          basic Grid::ComputeGradient() to devise a control that, at
          each timestep, requires just the "best" direction to the
          goal.
       
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
	   double scale,
	   GridOptions const & grid_options,
	   AlgorithmOptions const & algo_options,
	   /** If non-null, error messages are written to this
	       stream. This currently only happens when you specify an
	       invalid kernel_name. */
	   FILE * dbgstream);
    
    /** Like Create(), but doesn't give you a choice of Kernel (it's
	always LSMKernel) or of diagonally connected cells (that's
	always disabled). It uses the default AlgorithmOptions ctor. */
    static Facade *
    CreateDefault(ssize_t xsize, ssize_t ysize, double scale);
    
    
    /**
       Create a Facade from existing instances of Algorithm, Grid, and
       Kernel. There is NO CHECK if the separate entities are tied
       together they way they should though.
       
       \note Create() and CreateDefault() are much more convenient if
       you're creating new objects anyway.
    */
    Facade(boost::shared_ptr<Algorithm> algo,
	   boost::shared_ptr<Grid> grid,
	   boost::shared_ptr<Kernel> kernel);
    
    
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
    virtual double GetValue(ssize_t ix, ssize_t iy) const;
    
    /**
       Implements FacadeReadInterface::GetMeta(). See also
       Algorithm::SetMeta() for more details.
    */
    virtual double GetMeta(ssize_t ix, ssize_t iy) const;
    
    /**
       Implements FacadeWriteInterface::SetMeta(). See also
       Algorithm::SetMeta() for more details.
    */
    virtual bool SetMeta(ssize_t ix, ssize_t iy, double meta);
    
    /**
       Implements FacadeWriteInterface::AddGoal().
       
       \note Unlike AddGoal(const Region &), obstacle vertices are not
       ignored.
    */
    virtual bool AddGoal(ssize_t ix, ssize_t iy, double value);
    
    /**
       An alternative to AddGoal(ssize_t, ssize_t, double) which
       ignores obstacles [vertices with meta equal to
       GetObstacleMeta()] and doesn't tell us if it encountered any
       invalid indices.
       
       \note FacadeWriteInterface has no corresponding abstract method.
    */
    void AddGoal(const Region & goal);
    
    /**
       Revert a goal cell to normal status. Unless the cell happens to
       be non-goal before calling this method, this sets a flag which
       causes the next call to ComputeOne() to reinitialize the
       navigation function and wavefront.
    */
    void RemoveGoal(ssize_t ix, ssize_t iy);
    
    /**
       Revert all cells in a goal region to normal status. Basically
       just calls RemoveGoal(ssize_t, ssize_t) for all the cells in the
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
    virtual bool IsGoal(ssize_t ix, ssize_t iy) const;
    
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
    virtual node_status_t GetStatus(ssize_t ix, ssize_t iy) const;
        
    /**
       Implements FacadeReadInterface::GetStatus().
    */
    virtual node_status_t GetStatus(vertex_t vertex) const;
    
    /**
       Implements FacadeReadInterface::IsValidIndex().
    */
    virtual bool IsValidIndex(ssize_t ix, ssize_t iy) const;
    
    /**
       Implements FacadeReadInterface::GetLowestInconsistentValue().
    */
    virtual bool GetLowestInconsistentValue(double & value) const;
    
    /**
       Write the underlying grid to a stream. Actually just calls
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
    
  protected:
    friend class pnf::Flow;
    
    /**
       \note this is a hack for legacy code, don't rely on it
    */
    virtual boost::shared_ptr<Grid const> GetGrid() const;
  public:
    
    /**
       Implements FacadeReadInterface::GetKernel().
    */
    virtual const Kernel & GetKernel() const { return * m_kernel; }
    
    /**
       Implements FacadeReadInterface::TraceCarrot().
    */
    virtual int TraceCarrot(double robot_x, double robot_y,
			    double distance, double stepsize,
			    size_t maxsteps,
			    carrot_trace & trace) const;
    
    /**
       Implements FacadeReadInterface::GetCSpace().
    */
    virtual boost::shared_ptr<GridCSpace const> GetCSpace() const;
    
    
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
    boost::shared_ptr<GridCSpace const> m_cspace;
    boost::shared_ptr<Algorithm> m_algo;
    boost::shared_ptr<Grid> m_grid;
    boost::shared_ptr<Kernel> m_kernel;
    
    node_status_t DoGetStatus(ssize_t ix, ssize_t iy, vertex_t vertex) const;
  };
  
} // namespace estar

#endif // ESTAR_FACADE_HPP
