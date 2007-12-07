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


#ifndef ESTAR_ALGORITHM_HPP
#define ESTAR_ALGORITHM_HPP


#include <estar/CSpace.hpp>
#include <estar/Queue.hpp>
#include <estar/Upwind.hpp>
#include <estar/PropagatorFactory.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <map>


namespace estar {
  
  
  class Kernel;
  
  
  /**
     Medium-level layer for controlling E*. It uses the underlying
     C-space graph etc for implementing E*. End-users will probably
     prefer Facade, which provides higher level access.
  */
  class Algorithm {
  public:
    Algorithm(/** The object that represents the planning space. */
	      boost::shared_ptr<BaseCSpace> cspace,
	      /** Whether to check the upwind structure when computing
		  the propagator set of a node. Use false to mimic old
		  behavior. */
	      bool check_upwind,
	      /** Whether to check for local consistency (rhs==value)
		  when computing the propagator set of a node. Use
		  false to mimic old behavior. */
	      bool check_local_consistency,
	      /** Whether to check if a neighbor lies below the
		  wavefront when computing the propagator set of a
		  node. Use false to mimic old behavior */
	      bool check_queue_key,
	      /** Whether to automatically reset the whole E*
		  computations when meta information changes. This is
		  usually not needed. */
	      bool auto_reset,
	      /** Whether to automatically propagate until the entire
		  queue has been emptied each time you call
		  ComputeOne(). This is usually a waste of
		  resources. */
	      bool auto_flush);
    
    /**
       Declare a node to be a goal vertex, and fix its value. This
       only works as expected if the goal vertex is already connected
       to its neighbors. Specifying a value is useful for seeding the
       interpolation with a range of smoothly varying "true distance"
       values along the edges of an extended goal Region.
       
       If the vertex is already a goal, this method still checks if
       the value is different, and queues it if necessary. If the
       vertex is not yet a goal but the value happens to be the same,
       this method essentially "freezes" the vertex such that
       subsequent sweeps of the wavefront will not affect it.
    */
    void AddGoal(vertex_t vertex, double value);
    
    /**
       Declare that a node is not a goal anymore. This causes the
       whole Algorithm to Reset() the next time you call
       ComputeOne(). If the vertex was not a goal to begin with,
       however, this expensive operation is not performed.
    */
    void RemoveGoal(vertex_t vertex);
    
    /**
       Flag all goal nodes as "normal" again, and request a Reset()
       for the next ComputeOne(). Note, however, that ComputeOne()
       will only do something useful if there is at least one goal
       node.
    */
    void RemoveAllGoals();
    
    /**
       Reset the algorithm, preserving only goal information. This means:
       - clear the wavefront queue
       - set all values to infinity
       - set non-goal rhs values to infinity
       - clear non-goal flags
       - requeue goal cells (remember, their rhs values were
         preserved, which will make the original goal values reappear
         as soon as they have been popped off the queue and expanded)
       
       \note Automatically called from ComputeOne() if necessary (ie
       after goal removal).
    */
    void Reset();
    
    /**
       \return True if the node is part of the goal set.
    */
    bool IsGoal(vertex_t vertex) const;
    
    /**
       Set the meta information of a node (if different from what's
       already stored there). Unlike InitMeta() which should only be
       used during initialisation, this method also performs the
       correct updates by estimating the new one-step-lookahead
       rhs-value, entering the vertex into the Queue, and updating the
       upwind graph accordingly.
       
       The interpretation of the meta information, which encodes
       traversability information, depends on the Kernel. For example,
       the LSMKernel uses meta=0 for obstacles and meta=1 for
       freespace. In order to manage the meta information more or less
       generically, you can use Kernel::GetFreespaceMeta(),
       Kernel::GetObstacleMeta(), Kernel::ChangeWouldRaise(), or one
       of the RiskMap subclasses to translate from the general notion
       of "collision risk" to the Kernel-dependent meta
       interpretation.
    */
    void SetMeta(vertex_t vertex, double meta, const Kernel & kernel);
    
    /**
       Blindly set the meta information of a node.
       
       \note Only for intialisation! Use SetMeta() to correctly
       enqueue the node and its neighbors.
    */
    void InitMeta(vertex_t vertex, double meta);
    
    /**
       Blindly set the meta information of all nodes. See the remarks
       of InitMeta().
    */
    void InitAllMeta(double meta);
    
    /**
       Perform an elementary propagation (or "expansion") step. This
       is a no-op if the queue is empty. If there is a pending
       Reset(), such as after RemoveGoal() or any meta modification
       with an enabled m_auto_reset option, the reset is performed
       first. If m_auto_flush is set, the computation is iterated
       until the queue is empty.
       
       Node expansion means:
       -# Take the top vertex from the Queue.
       -# Check if it is "different enough" to warrant computations.
          This is controlled by the "slack" parameter, which should be
          significantly smaller than the smallest meaningful increment
          of value between two neighbors. For example, if you're using
          a grid with a certain "scale" (the distance between two
          neighboring nodes), then slack should be something like
          scale/10000.
       -# If the node's value gets decreased, update all neighbors as
          they might in turn get lowered.
       -# Otherwise, if the value gets raised, set this node to
          infinity, update all its downwind neighbors (whose value was
          computed using the just-raised node), and re-queue the node
          such that it may be lowered on its next turn
	  
       \todo Wasn't "slack" a bit of a hack at one point? Should test
       if we can take it out now, and just use epsilon as everywhere
       else...
    */
    void ComputeOne(const Kernel & kernel,
		    /** Value changes smaller than this are
			ignored. Slack should be significantly smaller
			than the smallest expected useful increment of
			value between two neighboring nodes. For
			example, if you're using a Grid instance, it
			is suggested to use slack=scale/10000 or
			so. */
		    double slack);
    
    /**
       \return True if there is something to do, such as expanding a
       node or re-initializing the algorithm after a goal removal.
    */
    bool HaveWork() const;
    
    /** Read-only access to C-space. */
    boost::shared_ptr<BaseCSpace const> GetCSpace() const { return m_cspace; }
    
    /** Read-only access to the C-space graph. */
    const cspace_t & GetCSpaceGraph() const { return m_cspace_graph; }
    
    /** Read-only access to the values of all C-space nodes. */
    const value_map_t & GetValueMap() const { return m_value; }
    
    /** Read-only access to the meta information of all C-space nodes. */
    const meta_map_t & GetMetaMap() const { return m_meta; }

    /** Read-only access to the rhs-value of all C-space nodes. */
    const rhs_map_t & GetRhsMap() const { return m_rhs; }

    /** Read-only access to the flags (flag_t) of all C-space nodes. */
    const flag_map_t & GetFlagMap() const { return m_flag; }
    
    /**
       Read-only access to the vertex ID of all C-space nodes, useful
       for adding user-defined data to nodes. For example, see Grid
       and GridNode.
       
       \note Check out the CustomCSpace template.
    */
    const vertexid_map_t & GetVertexIdMap() const
    { return m_cspace->GetVertexIdMap(); }
    
    /** Read-only access to the wavefront queue. */
    const Queue & GetQueue() const { return m_queue; }

    /** Read-only access to the upwind graph, which traces on which
	neighbor's a node's value depends. */
    const Upwind & GetUpwind() const { return m_upwind; }
    
    /** Write-access to the upwind queue. Should not be required
	anymore, unless you're developing some new features that
	should probably go into the Algorithm class when you're
	done. */
    Queue & GetQueue() { return m_queue; }
    
    /** \return The number of times that ComputeOne() has actually
	done something. */
    size_t GetStep() const { return m_step; }
    
    /** \return The last value that was used in ComputeOne(). */
    double GetLastComputedValue() const { return m_last_computed_value; }
    
    /** \return The ID of the vertex that was last touched by ComputeOne(). */
    vertex_t GetLastComputedVertex() const { return m_last_computed_vertex; }
    
    /** \return The queue key of the vertex that was last expanded by
	ComputeOne(). */
    double GetLastPoppedKey() const { return m_last_popped_key; }
    
    
  private:
    typedef std::set<vertex_t> goalset_t;
    
    
    void UpdateVertex(vertex_t vertex, const Kernel & kernel);
    void DoComputeOne(const Kernel & kernel, double slack);
    
    boost::shared_ptr<BaseCSpace> m_cspace;
    Queue m_queue;
    Upwind m_upwind;
    
    goalset_t m_goalset;
    
    size_t m_step;
    double m_last_computed_value;
    vertex_t m_last_computed_vertex;
    double m_last_popped_key;
    
    bool m_pending_reset;
    bool m_auto_reset;
    bool m_auto_flush;
    
    cspace_t & m_cspace_graph;
    value_map_t & m_value;
    meta_map_t & m_meta;
    rhs_map_t & m_rhs;
    flag_map_t & m_flag;

    boost::scoped_ptr<PropagatorFactory> m_propfactory;
  };
  
  
} // namespace estar

#endif // ESTAR_ALGORITHM_HPP
