/* 
 * Copyright (C) 2004
 * Swiss Federal Institute of Technology, Lausanne. All rights reserved.
 * 
 * Author: Roland Philippsen <roland dot philippsen at gmx net>
 *         Autonomous Systems Lab <http://asl.epfl.ch/>
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


#ifndef PNF_FLOW_HPP
#define PNF_FLOW_HPP


#include <estar/util.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <map>
#include <vector>


namespace estar {
  class Facade;
  class RiskMap;
}


namespace local {
  class Object;
  class Robot;
}


namespace pnf {
  
  
  class Sprite;
  class Region;
  class BufferZone;
  

  /** High-level interface for PNF. Uses underlying estar::Facade
      instances etc for implementing multi-layered propagation and
      risk computations. */
  class Flow
  {
  private:
    Flow(size_t xsize, size_t ysize, double resolution);
    
    
  public:
    const size_t xsize, ysize;
    const double resolution;
    const double half_diagonal;
    
    
    static Flow * Create(size_t xsize, size_t ysize, double resolution);
    ~Flow();    
    
    bool HaveEnvdist() const;
    void PropagateEnvdist(bool step);
    
    /**
       buffer zones around static objects: If the buffer factor is <=1
       or the degree <=0 or the riskmap NULL, then simple "on/off"
       information is used. Otherwise, a buffer is constructed on the
       environment distance, which helps with discontinuity problems
       at object boundaries such as the robot being fatally attrackted
       to walls.
     */
    void _MapEnvdist(double robot_buffer_factor,
		    double robot_buffer_degree,
		    double object_buffer_factor,
		    double object_buffer_degree,
		    const estar::RiskMap * riskmap);
    
    /** Convenient legacy wrapper. */
    void MapEnvdist() { _MapEnvdist(0, 0, 0, 0, 0); }
    
    bool HaveObjdist(size_t id) const;
    void PropagateObjdist(size_t id);
    bool HaveAllObjdist() const;
    void PropagateAllObjdist();
    bool HaveRobdist() const;
    void PropagateRobdist();

    void ComputeAllCooc();
    void ComputeRisk(const estar::RiskMap & risk_map,
		     /** buffer around static obstacles BEFORE
			 convolution with robot shape */
		     const BufferZone & buffer);
    
    bool HavePNF() const;
    void PropagatePNF();
    
    void AddStaticObject(size_t ix, size_t iy);
    void RemoveStaticObject(size_t ix, size_t iy);
    void RemoveDynamicObject(size_t id);
    
    bool AddStaticObject(double x, double y);
    bool RemoveStaticObject(double x, double y);
    bool SetDynamicObject(size_t id, double x, double y, double r, double v);
    bool SetRobot(double x, double y,double r,  double v);
    bool SetGoal(double x, double y, double r);
    
    void DumpEnvdist(FILE * stream) const;
    void DumpRobdist(FILE * stream) const;
    void DumpObjdist(size_t id, FILE * stream) const;
    void DumpObjCooc(size_t id, FILE * stream) const;
    void DumpRisk(FILE * stream) const;
    void DumpPNF(FILE * stream) const;
    
    /** \return 0 if invalid id */
    const estar::Facade * GetObjdist(size_t id) const
    { return GetObjdist(id, 0); }
    
    /** \return 0 if invalid id */
    const pnf::Region * GetRegion(size_t id) const;
    
    /** \return 0 if invalid */
    const pnf::Region * GetGoal() const { return m_goal.get(); }
    
    /** \return 0 if invalid */
    const pnf::Region * GetRobot() const;
    
    /** \return 0 if no robot defined */
    const estar::Facade * GetRobdist() const;
    
    /** \todo non-const only for debugging? */
    estar::Facade & GetEnvdist() { return * m_envdist; }
    
    const estar::Facade & GetPNF()     const { return * m_pnf; }
    
    /** \return pair of pointer and max lambda value (excluding
	infinity), or std::make_pair(0, -1) if invalid id */
    std::pair<const estar::array<double> *, double> GetLambda(size_t id) const;
    
    /** \return pair of pointer and max cooc value, or
	std::make_pair(0, -1) if invalid id */
    std::pair<const estar::array<double> *, double> GetCooc(size_t id) const;
    
    /** \return pair of pointer and max risk value, or
	std::make_pair(0, -1) if risk not available yet */
    std::pair<const estar::array<double> *, double> GetRisk() const;
    
    /** \return pair of pointer and max risk value, or
	std::make_pair(0, -1) if risk not available yet */
    std::pair<const estar::array<double> *, double> GetWSpaceRisk() const;
    
    
  private:
    typedef std::map<size_t, boost::shared_ptr<local::Object> > objectmap_t;
    
    boost::scoped_ptr<estar::Facade>  m_envdist;
    boost::scoped_ptr<local::Robot>   m_robot;
    objectmap_t                       m_object;
    boost::scoped_ptr<estar::Facade>  m_pnf;
    boost::scoped_ptr<Region>         m_goal;
    
    boost::scoped_ptr<estar::array<double> > m_wspace_risk; // for plotting
    double m_max_wspace_risk;	// for plotting
    boost::scoped_ptr<estar::array<double> > m_risk; // for plotting
    double m_max_risk;		// for plotting
    
    /** \todo do something like this in estar::Facade API */
    static void DoAddGoal(estar::Facade & facade, const Region & goal);
    static void DoRemoveGoal(estar::Facade & facade, const Region & goal);
    
    bool CompIndices(double x, double y, size_t & ix, size_t & iy) const;
    estar::Facade * GetObjdist(size_t id, FILE * verbose_stream) const;

    bool DoSetRobot(double x, double y, size_t ix, size_t iy,
		    double r, double v);
    void DoComputeLambda(local::Object & obj);
    void DoComputeCooc(local::Object & obj);
  };
  
}

#endif // PNF_FLOW_HPP
