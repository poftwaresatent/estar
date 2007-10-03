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


#include <estar/base.hpp>
#include <estar/numeric.hpp>
#include <estar/Queue.hpp>
#include <estar/Upwind.hpp>
#include <map>


namespace estar {
  
  
  class Kernel;
  
  
  /** Medium-level layer for controlling E*. Uses the underlying
      C-space graph etc for implementing E*. Use Facade for higher
      level access. */
  class Algorithm {
  public:
    Algorithm();
    
    vertex_t AddVertex(double value = infinity,
		       double meta = 1,
		       double rhs = infinity,
		       flag_t flag = NONE);
    
    void AddNeighbor(vertex_t from, vertex_t to);
    
    void AddGoal(vertex_t vertex, double value);
    
    /** \todo PLEASE TEST ME! And: How to make sure that the vertex
	then has a reasonable meta? */
    void RemoveGoal(vertex_t vertex);
    
    /** \todo PLEASE TEST ME! And: How to make sure that the vertex
	then has a reasonable meta? */
    void RemoveAllGoals();
    
    /**
       Reset the algorithm, preserving only goal information:
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
    
    bool IsGoal(vertex_t vertex) const;

    void SetMeta(vertex_t vertex, double meta, const Kernel & kernel);
    
    /** \note Only for intialisation! Use SetMeta() to correctly
	enqueue the node and its neighbors. */
    void InitMeta(vertex_t vertex, double meta);
    void InitAllMeta(double meta);
    
    void ComputeOne(const Kernel & kernel, double slack);
    bool HaveWork() const;
    
    const cspace_t &       GetCSpace() const      { return m_cspace; }
    const value_map_t &    GetValueMap() const    { return m_value; }
    const meta_map_t &     GetMetaMap() const     { return m_meta; }
    const rhs_map_t &      GetRhsMap() const      { return m_rhs; }
    const flag_map_t &     GetFlagMap() const     { return m_flag; }
    const vertexid_map_t & GetVertexIdMap() const { return m_vertexid; }
    const Queue &          GetQueue() const       { return m_queue; }
    const Upwind &         GetUpwind() const      { return m_upwind; }

    Queue & GetQueue() { return m_queue; }
    
    size_t GetStep()                 const { return m_step; }
    double GetLastComputedValue()    const { return m_last_computed_value; }
    vertex_t GetLastComputedVertex() const { return m_last_computed_vertex; }
    double GetLastPoppedKey()        const { return m_last_popped_key; }
    
    
  private:
    typedef std::set<vertex_t> goalset_t;
    
    
    void UpdateVertex(vertex_t vertex, const Kernel & kernel);
    
    
    Queue m_queue;
    Upwind m_upwind;
    
    cspace_t m_cspace;
    goalset_t m_goalset;
    
    value_map_t    m_value;
    meta_map_t     m_meta;
    rhs_map_t      m_rhs;
    flag_map_t     m_flag;
    vertexid_map_t m_vertexid;
    
    size_t m_step;
    double   m_last_computed_value;
    vertex_t m_last_computed_vertex;
    double   m_last_popped_key;
    
    bool m_pending_reset;
  };
  
  
} // namespace estar

#endif // ESTAR_ALGORITHM_HPP
