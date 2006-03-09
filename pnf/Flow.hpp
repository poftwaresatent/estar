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


namespace pnf {
  
  
  class RobotShape;
  

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
    
    
    bool HaveEnvdist() const;
    void PropagateEnvdist(bool step);
    
    void MapEnvdist();
    
    bool HaveObjdist(size_t id) const;
    void PropagateObjdist(size_t id);
    bool HaveAllObjdist() const;
    void PropagateAllObjdist();
    bool HaveRobdist() const;
    void PropagateRobdist();

    void ComputeAllCooc();
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
    
    /** \return 0 if no robot defined */
    const estar::Facade * GetRobdist() const;
    
    /** \todo non-const only for debugging? */
    estar::Facade & GetEnvdist() { return * m_envdist; }
    
    const estar::Facade & GetPNF()     const { return * m_pnf; }
    
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
    /** Utility for holding a node (C-space vertex). */
    class node {
    public:
      node(size_t _ix, size_t _iy, double _val)
	: ix(_ix), iy(_iy), val(_val) { }
      size_t ix, iy;
      double val;
    };
    
    /** Utility for holding a region, ie list of node instances.
	
	\todo idea of goals that can be diffed against each other and
	patches used on estar::Facade for goal modifications, eg via
	visitor pattern... */
    class region {
    public:
      /** \note populates the nodelist by filling a circle */
      region(double x, double y, double rad,
	     double scale, size_t xsize, size_t ysize);
      const double x, y, rad;
      typedef std::vector<node> nodelist_t;
      nodelist_t nodelist;
    };
    
    /** Utility base class for holding objects or the robot. */
    class baseobject {
    public:
      /** \note transfers ownership, pure container, no computations */
      baseobject(region * footprint, double speed, estar::Facade * dist);
      boost::shared_ptr<region> footprint;
      const double speed;
      boost::shared_ptr<estar::Facade> dist;
    };
    
    /** Utility for representing an object. */
    class object: public baseobject {
    public:
      /** \note transfers ownership, pure container, no computations */
      object(region * footprint, double speed, estar::Facade * dist,
	     const size_t id, estar::array<double> * cooc);
      const size_t id;
      boost::shared_ptr<estar::array<double> > cooc;
      double max_cooc;
    };
    
    /** Utility for representing the robot. */
    class robot: public baseobject {
    public:
      /** \note transfers ownership, pure container, no computations */
      robot(region * footprint, double speed, estar::Facade * dist,
	    RobotShape * mask);
      boost::shared_ptr<RobotShape> mask;
    };
    
    typedef std::map<size_t, boost::shared_ptr<object> > objectmap_t;
    
    boost::scoped_ptr<estar::Facade>  m_envdist;
    boost::scoped_ptr<robot>          m_robot;
    objectmap_t                       m_object;
    boost::scoped_ptr<estar::Facade>  m_pnf;
    boost::scoped_ptr<region>         m_goal;
    
    boost::scoped_ptr<estar::array<double> > m_wspace_risk; // for plotting
    double m_max_wspace_risk;	// for plotting
    boost::scoped_ptr<estar::array<double> > m_risk; // for plotting
    double m_max_risk;		// for plotting
    
    /** \todo do something like this in estar::Facade API */
    static void DoAddGoal(estar::Facade & facade, const region & goal);
    static void DoRemoveGoal(estar::Facade & facade, const region & goal);
    
    bool CompIndices(double x, double y, size_t & ix, size_t & iy) const;
    estar::Facade * GetObjdist(size_t id, FILE * verbose_stream) const;

    void DoSetRobot(double x, double y, size_t ix, size_t iy,
		    double r, double v);
    void DoComputeCooc(object & obj);
  };
  
}

#endif // PNF_FLOW_HPP
