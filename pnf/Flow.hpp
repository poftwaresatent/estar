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
    Flow(size_t xsize, size_t ysize, double resolution,
	 bool perform_convolution);
    
    
  public:
    typedef std::pair<const estar::array<double> *, double> array_info_t;
    
    const size_t xsize, ysize;
    const double resolution;
    const double half_diagonal;
    const bool perform_convolution;
    
    static Flow * Create(size_t xsize, size_t ysize, double resolution,
			 bool perform_convolution);
    ~Flow();    
    
    bool HaveEnvdist() const;
    void PropagateEnvdist(bool step);
    
    void MapEnvdist();
    
    bool HaveObjdist(size_t id) const;
    void PropagateObjdist(size_t id);
    bool HaveAllObjdist() const;
    void PropagateAllObjdist();
    bool HaveRobdist() const;
    void PropagateRobdist();
    
    /**
       Buffer zones around static objects: If the buffer factor or
       degree are non-positive, then simple "on/off" information is
       used. Otherwise, a buffer is constructed on the environment
       distance. Sensible values would be static_buffer_factor=1 and
       static_buffer_degree=2, resulting in a quadratic risk decrease
       extending one extra robot radius from the walls.
     */
    void ComputeAllCooc(double static_buffer_factor,
			double static_buffer_degree);
    void ComputeRisk(const estar::RiskMap & risk_map);
    
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
	infinity) for the robot */
    array_info_t GetRobotLambda() const;
    
    /** \return pair of pointer and max lambda value (excluding
	infinity) for an object, or std::make_pair(0, -1) if the id is
	invalid (no such dynamic object) */
    array_info_t GetObjectLambda(size_t id) const;
    
    /** \return pair of pointer and max cooc value, if the environment
	co-occurrence has been computed using ComputeAllCooc(),
	otherwise (0, -1). */
    array_info_t GetEnvCooc() const;
    
    /** \return pair of pointer and max cooc value, or
	std::make_pair(0, -1) if invalid id or you forgot to call
	ComputeAllCooc() */
    array_info_t GetObjCooc(size_t id) const;
    
    /** \return pair of pointer and max cooc value, or
	std::make_pair(0, -1) if invalid. */
    array_info_t GetDynamicCooc() const;
    
    /** \return pair of pointer and max risk value, or
	std::make_pair(0, -1) if risk not available yet
	\note the "risk" is "just" the fusion of all co-occurrences */
    array_info_t GetRisk() const;
    
    
  private:
    typedef std::map<size_t, boost::shared_ptr<local::Object> > objectmap_t;
    
    boost::scoped_ptr<estar::Facade>  m_envdist;
    boost::scoped_ptr<local::Robot>   m_robot;
    objectmap_t                       m_object;
    boost::scoped_ptr<estar::Facade>  m_pnf;
    boost::scoped_ptr<Region>         m_goal;
    
    boost::scoped_ptr<estar::array<double> > m_env_cooc;
    double m_max_env_cooc;
    boost::scoped_ptr<estar::array<double> > m_dynamic_cooc;
    double m_max_dynamic_cooc;
    boost::scoped_ptr<estar::array<double> > m_risk;
    double m_max_risk;
    
    /** \todo do something like this in estar::Facade API */
    static void DoAddGoal(estar::Facade & facade, const Region & goal);
    static void DoRemoveGoal(estar::Facade & facade, const Region & goal);
    
    bool CompIndices(double x, double y, size_t & ix, size_t & iy) const;
    estar::Facade * GetObjdist(size_t id, FILE * verbose_stream) const;

    bool DoSetRobot(double x, double y, size_t ix, size_t iy,
		    double r, double v);
    void DoComputeLambda(local::Robot & obj);
    void DoComputeCooc(local::Object & obj);
  };
  
}

#endif // PNF_FLOW_HPP
