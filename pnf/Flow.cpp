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


#include "Flow.hpp"
#include "pnf_cooc.h"
#include "BufferZone.hpp"
#include "../estar/RiskMap.hpp"
#include "../estar/numeric.hpp"
#include "../estar/Facade.hpp"
#include "../estar/Algorithm.hpp"
#include "../estar/Grid.hpp"
#include "../estar/Kernel.hpp"
#include "../estar/dump.hpp"
#include "../estar/Region.hpp"
#include "../estar/pdebug.hpp"
#include "../estar/GridNode.hpp"
#include <iostream>
#include <cmath>

#define BOOST_ENABLE_ASSERT_HANDLER
#include <boost/assert.hpp>


using estar::Region;
using estar::Sprite;
using estar::Facade;
using estar::square;
using estar::minval;
using estar::value_map_t;
using estar::vertexid_map_t;
using estar::vertex_read_iteration;
using estar::GridCSpace;
using estar::Grid;
using estar::Algorithm;
using estar::Kernel;
using estar::dump_probabilities;
using estar::RiskMap;
using estar::array;
using estar::infinity;
using estar::GridNode;
using boost::shared_ptr;
using boost::scoped_ptr;
using std::make_pair;
using std::cerr;


namespace local {
  
  
  /** Utility for representing the robot. */
  class Robot
  {
  public:
    Robot(double _radius, double _speed,
	  shared_ptr<Region> _region, shared_ptr<Facade> _dist,
	  size_t xsize, size_t ysize)
      : radius(_radius), speed(_speed), max_lambda(-1),
	region(_region), dist(_dist),
	lambda(new array<double>(xsize, ysize))
    {}
    const double radius;
    const double speed;
    double max_lambda;
    shared_ptr<Region> region;
    shared_ptr<Facade> dist;
    shared_ptr<array<double> > lambda;
  };
  
  
  /** Utility for representing an object. */
  class Object
    : public Robot
  {
  public:
    Object(size_t _id, double radius, double speed,
	   shared_ptr<Region> region, shared_ptr<Facade> dist,
	   size_t xsize, size_t ysize)
      : Robot(radius, speed, region, dist, xsize, ysize),
	id(_id), max_cooc(-1),
	cooc(new array<double>(xsize, ysize))
    {}
    const size_t id;
    double max_cooc;
    shared_ptr<array<double> > cooc;
  };
  
}


using namespace local;


namespace pnf {
  
  
  Flow::
  Flow(ssize_t _xsize, ssize_t _ysize, double _resolution,
       bool _perform_convolution, bool _alternate_worst_case)
    : xsize(_xsize),
      ysize(_ysize),
      resolution(_resolution),
      half_diagonal(0.707106781187 * _resolution), // sqrt(1/2)
      perform_convolution(_perform_convolution),
      alternate_worst_case(_alternate_worst_case),
      m_envdist(Facade::Create("lsm",
			       _resolution,
			       estar::GridOptions(0, _xsize, 0, _ysize),
			       estar::AlgorithmOptions(),
			       0)),
      // m_robot invalid until SetRobot()
      // m_object to be populated by SetDynamicObject()
      // m_risk invalid until ComputeRisk()
      m_pnf(Facade::Create("lsm",
			   _resolution,
			   estar::GridOptions(0, _xsize, 0, _ysize),
			   estar::AlgorithmOptions(),
			   0))
    // m_goal invalid until SetGoal()
  {
  }
  
  
  Flow::
  ~Flow()
  {
  }
  
  
  Flow * Flow::
  Create(ssize_t xsize, ssize_t ysize, double resolution,
	 bool perform_convolution, bool alternate_worst_case)
  {
    Flow * flow(new Flow(xsize, ysize, resolution,
			 perform_convolution, alternate_worst_case));
    if(( ! flow->m_envdist) || ( ! flow->m_pnf)){
      delete flow;
      return 0;
    }
    return flow;
  }
  
  
  void Flow::
  AddStaticObject(ssize_t ix, ssize_t iy)
  {
    m_envdist->AddGoal(ix, iy, 0);
  }
  
  
  void Flow::
  RemoveStaticObject(ssize_t ix, ssize_t iy)
  {
    m_envdist->RemoveGoal(ix, iy);
  }
  
  
  bool Flow::
  SetDynamicObject(size_t id, double x, double y, double r, double v)
  {
    const double region_radius(r - half_diagonal);
    shared_ptr<Region>
      region(new Region(region_radius, resolution, x, y, 0, xsize, 0, ysize));
    if(region->GetArea().empty())
      return false;
    
    shared_ptr<Facade>
      dist(Facade::Create("lsm",
			  resolution,
			  estar::GridOptions(0, xsize, 0, ysize),
			  estar::AlgorithmOptions(),
			  0));
    BOOST_ASSERT( dist );
    const double object_radius(r + half_diagonal);
    shared_ptr<Object>
      obj(new Object(id, object_radius, v, region, dist, xsize, ysize));
    objectmap_t::iterator io(m_object.find(id));
    if(m_object.end() == io)
      m_object.insert(make_pair(id, obj));
    else
      io->second = obj;
    
    return true;
  }
  
  
  void Flow::
  RemoveDynamicObject(size_t id)
  {
    // silently ignore invalid ids (could check retval of erase())
    m_object.erase(id);
  }
  
  
  /** \todo URGENT! Goal and obstacle handling not made for replanning. */
  bool Flow::
  DoSetRobot(double x, double y, ssize_t ix, ssize_t iy, double r, double v)
  {
    const double region_radius(r - half_diagonal);
    shared_ptr<Region>
      region(new Region(region_radius, resolution, x, y, 0, xsize, 0, ysize));
    if(region->GetArea().empty())
      return false;		// actually already checked by caller...
    
    shared_ptr<Facade>
      dist(Facade::Create("lsm",
			  resolution,
			  estar::GridOptions(0, xsize, 0, ysize),
			  estar::AlgorithmOptions(),
			  0));
    BOOST_ASSERT( dist );
    const double robot_radius(r + half_diagonal);
    m_robot.reset(new Robot(robot_radius, v, region, dist, xsize, ysize));
    
    return true;
  }
  
  
  /** \todo Can return true after reaching max {object, robot} radius,
      see Algorithm::GetMaxKnownValue(). */
  bool Flow::
  HaveEnvdist()
    const
  {
    return ! m_envdist->HaveWork();
  }
  
  
  /** \todo Can stop after reaching max {object, robot} radius, see
      Algorithm::GetMaxKnownValue(). */
  void Flow::
  PropagateEnvdist(bool step)
  {
    if(step){
      if(m_envdist->HaveWork())
	m_envdist->ComputeOne();
    }
    else{
      while(m_envdist->HaveWork())
	m_envdist->ComputeOne();
    }
  }
  
  
  void Flow::
  MapEnvdist()
  {
    // Threshold envdist value with robot and object radii, use
    // non-Facade access for efficiency.
    
    const value_map_t & envdist(m_envdist->GetAlgorithm().GetValueMap());
    const vertexid_map_t &
      vertexid(m_envdist->GetAlgorithm().GetVertexIdMap());
    
    // if you want to be paranoid, this should be specific for each
    // object as well as the robot... but they're all using the same
    // kernel anyways
    BOOST_ASSERT( m_robot );
    const double freespace(m_robot->dist->GetFreespaceMeta());
    const double obstacle(m_robot->dist->GetObstacleMeta());
    
    { // Robot obstacles:
      Algorithm & robalgo(m_robot->dist->GetAlgorithm());
      const Kernel & robkernel(m_robot->dist->GetKernel());
      
      // BEWARE: we assume vertex IDs are consistent across the
      // various C-spaces!
      for (vertex_read_iteration viter(m_envdist->GetCSpace()->begin());
	   viter.not_at_end(); ++viter)
	if (viter.get(envdist) > m_robot->radius)
	  robalgo.SetMeta(viter.get(vertexid), freespace, robkernel);
	else
	  robalgo.SetMeta(viter.get(vertexid), obstacle, robkernel);
      
      // set robot goal, this will skip obstacles
      m_robot->dist->AddGoal(*m_robot->region);
    }
    
    { // Object obstacles:
      for(objectmap_t::iterator io(m_object.begin()); io != m_object.end();
	  ++io){
	Facade & objdist(*io->second->dist);
	Algorithm & objalgo(objdist.GetAlgorithm());
	const Kernel & objkernel(objdist.GetKernel());
	
	for (vertex_read_iteration viter(m_envdist->GetCSpace()->begin());
	     viter.not_at_end(); ++viter)
	  if (viter.get(envdist) > io->second->radius)
	    objalgo.SetMeta(viter.get(vertexid), freespace, objkernel);
	  else
	    objalgo.SetMeta(viter.get(vertexid), obstacle, objkernel);
	
	// set object goal, this will skip obstacles
	objdist.AddGoal(*io->second->region);
      }
    }
  }
  
  
  bool Flow::
  HaveObjdist(size_t id)
    const
  {
    objectmap_t::const_iterator io(m_object.find(id));
    if(m_object.end() == io){
      cerr << "ERROR in " << __FUNCTION__ << "(): invalid id " << id << "\n"
	   << "  valid ids:";
      for(io = m_object.begin(); io != m_object.end(); ++io)
	cerr << " " << io->first;
      cerr << "\n";
      exit(EXIT_FAILURE);
    }
    return ! io->second->dist->HaveWork();
  }
  
  
  void Flow::
  PropagateObjdist(size_t id)
  {
    objectmap_t::iterator io(m_object.find(id));
    if(m_object.end() == io){
      cerr << "ERROR in " << __FUNCTION__ << "(): invalid id " << id << "\n"
	   << "  valid ids:";
      for(io = m_object.begin(); io != m_object.end(); ++io)
	cerr << " " << io->first;
      cerr << "\n";
      exit(EXIT_FAILURE);
    }
    while(io->second->dist->HaveWork())
      io->second->dist->ComputeOne();
    DoComputeLambda(*io->second);
  }
  
  
  bool Flow::
  HaveAllObjdist()
    const
  {
    for(objectmap_t::const_iterator io(m_object.begin());
	io != m_object.end(); ++io)
      if(io->second->dist->HaveWork())
	return false;
    return true;
  }
  
  
  void Flow::
  PropagateAllObjdist()
  {
    for(objectmap_t::iterator io(m_object.begin());
	io != m_object.end(); ++io){
      while(io->second->dist->HaveWork())
	io->second->dist->ComputeOne();
      DoComputeLambda(*io->second);
    }
  }
  
  
  bool Flow::
  HaveRobdist()
    const
  {
    BOOST_ASSERT( m_robot );
    return ! m_robot->dist->HaveWork();
  }
  
  
  void Flow::
  PropagateRobdist()
  {
    BOOST_ASSERT( m_robot );
    while(m_robot->dist->HaveWork())
      m_robot->dist->ComputeOne();
    DoComputeLambda(*m_robot);
  }
  
  
  /** \todo EASY SPEEDUP: loop over border instead of area after
      checking that the object center is not within the sprite! */
  void Flow::
  DoComputeLambda(Robot & obj)
  {
    const Sprite::indexlist_t & area(obj.region->GetSprite().GetArea());
    ////const value_map_t & objdist(obj.dist->GetAlgorithm().GetValueMap());
    shared_ptr<Grid const> objgrid(obj.dist->GetGrid());
    shared_ptr<GridCSpace const> objcspace(obj.dist->GetCSpace());
    obj.max_lambda = -1;	// could be more paranoid...
    for(ssize_t ix(0); ix < xsize; ++ix)
      for(ssize_t iy(0); iy < ysize; ++iy){
	double lambda(infinity);
	for(size_t ia(0); ia < area.size(); ++ia){
	  const ssize_t jx(ix + area[ia].x);
	  if((jx < 0) || (jx >= xsize))
	    continue;
	  const ssize_t jy(iy + area[ia].y);
	  if((jy < 0) || (jy >= ysize))
	    continue;
	  shared_ptr<GridNode const> node(objgrid->GetNode(jx, jy));
	  if (node) {
	    double const ll(objcspace->GetValue(node->vertex));
	    if(ll < lambda)
	      lambda = ll;
	  }
	}
	(*obj.lambda)[ix][iy] = lambda;
	if((lambda < infinity) && (lambda > obj.max_lambda))
	  obj.max_lambda = lambda;
      }
  }
  
  
  void Flow::
  DoComputeCooc(Object & obj)
  {
    BOOST_ASSERT( m_robot );
    obj.max_cooc = 0;
    if(alternate_worst_case){
      const unsigned int nvisteps(100);
      for(ssize_t ix(0); ix < xsize; ++ix)
	for(ssize_t iy(0); iy < ysize; ++iy){
	  const double cooc(pnf_cooc_test_alt((*obj.lambda)[ix][iy],
					      (*m_robot->lambda)[ix][iy],
					      obj.speed, m_robot->speed,
					      resolution, nvisteps));
	  if(cooc > obj.max_cooc)
	    obj.max_cooc = cooc;
	  (*obj.cooc)[ix][iy] = cooc;
	}
    }
    else
      for(ssize_t ix(0); ix < xsize; ++ix)
	for(ssize_t iy(0); iy < ysize; ++iy){
	  const double cooc(pnf_cooc((*obj.lambda)[ix][iy],
				     (*m_robot->lambda)[ix][iy],
				     obj.speed, m_robot->speed, resolution));
	  if(cooc > obj.max_cooc)
	    obj.max_cooc = cooc;
	  (*obj.cooc)[ix][iy] = cooc;
	}
  }
  
  
  void Flow::
  ComputeAllCooc(double static_buffer_factor,
		 double static_buffer_degree)
  {
    PVDEBUG("f: %g   d: %g\n", static_buffer_factor, static_buffer_degree);
    BOOST_ASSERT( m_robot );
    shared_ptr<BufferZone> buffer;
    if((0 < static_buffer_factor) && (0 < static_buffer_degree)){
      buffer.reset(new BufferZone(perform_convolution ? 0 : m_robot->radius,
				  m_robot->radius * static_buffer_factor,
				  static_buffer_degree));
      PVDEBUG("buffer (convolute: %s)   r: %g   b: %g   d: %g\n"
	      "  example d = r * {0, 0.5, 1, 1.5, 2}\n"
	      "  %g   %g   %g   %g   %g\n",
	      perform_convolution ? "yes" : "no",
	      perform_convolution ? 0 : m_robot->radius,
	      m_robot->radius * static_buffer_factor,
	      static_buffer_degree,
	      buffer->DistanceToRisk(0),
	      buffer->DistanceToRisk(m_robot->radius * 0.5),
	      buffer->DistanceToRisk(m_robot->radius),
	      buffer->DistanceToRisk(m_robot->radius * 1.5),
	      buffer->DistanceToRisk(m_robot->radius * 2));
    }
    else
      PVDEBUG("static buffer factor and/or degree invalid ==> binary map\n");
    m_env_cooc.reset(new array<double>(xsize, ysize));
    m_max_env_cooc = 0;
    const value_map_t & envdist(m_envdist->GetAlgorithm().GetValueMap());
    shared_ptr<GridCSpace const> const envcspace(m_envdist->GetCSpace());
    
    for (vertex_read_iteration viter(m_envdist->GetCSpace()->begin());
	 viter.not_at_end(); ++viter) {
      double const dist(viter.get(envdist));
      double cooc;
      
      if (buffer)
	cooc = buffer->DistanceToRisk(dist);
      else if(perform_convolution)
	cooc = (dist <= estar::epsilon ? 1 : 0);
      else
	cooc = (dist <= m_robot->radius ? 1 : 0);
      
      shared_ptr<estar::GridNode const> const gg(envcspace->Lookup(*viter));
      if ((gg->ix < xsize) && (gg->iy < ysize)) { // parano check
	(*m_env_cooc)[gg->ix][gg->iy] = cooc;
	if(cooc > m_max_env_cooc)
	  m_max_env_cooc = cooc;
      }
    }
    
    for(objectmap_t::iterator io(m_object.begin());
	io != m_object.end(); ++io)
      DoComputeCooc(*io->second);
  }
  
  
  void Flow::
  ComputeRisk(const RiskMap & risk_map)
  {
    BOOST_ASSERT( m_robot );
    m_risk.reset(new array<double>(xsize, ysize));
    m_max_risk = 0;
    m_dynamic_cooc.reset(new array<double>(xsize, ysize));
    m_max_dynamic_cooc = 0;
    
    if( ! perform_convolution){
      for(ssize_t ix(0); ix < xsize; ++ix)
	for(ssize_t iy(0); iy < ysize; ++iy){
	  double risk(1); // could be more direct, but caching m_dynamic_cooc
	  for(objectmap_t::const_iterator io(m_object.begin());
	      io != m_object.end(); ++io)
	    risk *= 1 - (*io->second->cooc)[ix][iy];
	  risk = 1 - risk;
	  (*m_dynamic_cooc)[ix][iy] = risk;
	  if(risk > m_max_dynamic_cooc)
	    m_max_dynamic_cooc = risk;
	  risk = 1 - (1 - risk) * (1 - (*m_env_cooc)[ix][iy]);
	  (*m_risk)[ix][iy] = risk;
	  if(risk > m_max_risk)
	    m_max_risk = risk;
	  m_pnf->SetMeta(ix, iy, risk_map.RiskToMeta(risk));	
	}
    }
    
    else{ // perform_convolution
      array<double> accu(xsize, ysize);
      for(ssize_t ix(0); ix < xsize; ++ix)
	for(ssize_t iy(0); iy < ysize; ++iy){
	  double risk(1); // could be more direct, but caching m_dynamic_cooc
	  for(objectmap_t::const_iterator io(m_object.begin());
	      io != m_object.end(); ++io)
	    risk *= 1 - (*io->second->cooc)[ix][iy];
	  risk = 1 - risk;
	  (*m_dynamic_cooc)[ix][iy] = risk;
	  if(risk > m_max_dynamic_cooc)
	    m_max_dynamic_cooc = risk;
	  accu[ix][iy] = 1 - (1 - risk) * (1 - (*m_env_cooc)[ix][iy]);
	}
      const Sprite::indexlist_t & area(m_robot->region->GetSprite().GetArea());
      for(ssize_t ix(0); ix < xsize; ++ix)
	for(ssize_t iy(0); iy < ysize; ++iy){
	  double risk(1);
	  for(ssize_t ia(0); ia < static_cast<ssize_t>(area.size()); ++ia){
	    const ssize_t jx(ix + area[ia].x);
	    const ssize_t jy(iy + area[ia].y);
	    if((jx < 0) || (jx >= xsize)
	       || (jy < 0) || (jy >= ysize)){
	      risk = 0;           // grid edges are walls
	      break;
	    }
	    risk *= 1 - accu[jx][jy];
	  }
	  risk = 1 - risk;
	  (*m_risk)[ix][iy] = risk;
	  if(risk > m_max_risk)
	    m_max_risk = risk;
	  m_pnf->SetMeta(ix, iy, risk_map.RiskToMeta(risk));	
	}
    }
    
    // this is a bit of a hack that depend on the exact call order
    BOOST_ASSERT( m_goal );
    m_pnf->AddGoal(*m_goal);
  }
  
  
  bool Flow::
  HavePNF()
    const
  {
    BOOST_ASSERT( m_goal );
    return ! m_pnf->HaveWork();
  }


  void Flow::
  PropagatePNF()
  {
    BOOST_ASSERT( m_goal );
    while(m_pnf->HaveWork())
      m_pnf->ComputeOne();
  }
  
  
  bool Flow::
  AddStaticObject(double x, double y)
  {
    if((x < 0) || (y < 0))
      return false;
    const ssize_t ix(static_cast<ssize_t>(rint(x / resolution)));
    if(ix >= xsize)
      return false;
    const ssize_t iy(static_cast<ssize_t>(rint(y / resolution)));
    if(iy >= ysize)
      return false;
    AddStaticObject(ix, iy);
    return true;
  }
  
  
  bool Flow::
  RemoveStaticObject(double x, double y)
  {
    if((x < 0) || (y < 0))
      return false;
    const ssize_t ix(static_cast<ssize_t>(rint(x / resolution)));
    if(ix >= xsize)
      return false;
    const ssize_t iy(static_cast<ssize_t>(rint(y / resolution)));
    if(iy >= ysize)
      return false;
    RemoveStaticObject(ix, iy);
    return true;
  }
  
  
  bool Flow::
  SetRobot(double x, double y, double r, double v)
  {
    ssize_t ix, iy;
    if( ! CompIndices(x, y, ix, iy))
      return false;
    return DoSetRobot(x, y, ix, iy, r, v);
  }
  
  
  /** \todo URGENT! The goal vs. obstacle problem IS NOT SOLVED! */
  bool Flow::
  SetGoal(double x, double y, double r)
  {
    if(m_goal)
      m_pnf->RemoveGoal(*m_goal);
    scoped_ptr<Region>
      goal(new Region(r, resolution, x, y, 0, xsize, 0, ysize));
    if(goal->GetArea().empty()){
      PVDEBUG("WARNING: empty goal area, treated like invalid goal!\n");
      m_goal.reset();
      return false;
    }
    m_goal.swap(goal);
    return true;
  }
  
  
  void Flow::
  DumpEnvdist(FILE * stream)
    const
  {
    fprintf(stream,
	    "--------------------------------------------------\n"
	    "Environment distance %p:\n", m_envdist.get());
    m_envdist->DumpPointers(stream);
    m_envdist->DumpGrid(stream);
  }
  
  
  void Flow::
  DumpRobdist(FILE * stream)
    const
  {
    BOOST_ASSERT( m_robot );
    fprintf(stream,
	    "--------------------------------------------------\n"
	    "Robot distance %p:\n", m_robot->dist.get());
    m_robot->dist->DumpPointers(stream);
    m_robot->dist->DumpGrid(stream);
  }
  
  
  void Flow::
  DumpObjdist(size_t id, FILE * stream)
    const
  {
    fprintf(stream,
	    "--------------------------------------------------\n"
	    "Object %zd distance:\n", id);
    objectmap_t::const_iterator io(m_object.find(id));
    if(m_object.end() == io)
      fprintf(stream, "NO SUCH OBJECT\n");
    else{
      fprintf(stream, "distance from object %p:\n", io->second.get());
      io->second->dist->DumpPointers(stream);
      io->second->dist->DumpGrid(stream);
    }
  }
  
  
  void Flow::
  DumpObjCooc(size_t id, FILE * stream)
    const
  {
    fprintf(stream,
	    "--------------------------------------------------\n"
	    "Object %zd co-occurrence:\n", id);
    objectmap_t::const_iterator io(m_object.find(id));
    if(m_object.end() == io)
      fprintf(stream, "NO SUCH OBJECT\n");
    else
      dump_probabilities(*io->second->cooc, 0, 0, xsize-1, ysize-1, stream);
  }
  
  
  void Flow::
  DumpRisk(FILE * stream)
    const
  {
    fprintf(stream,
	    "--------------------------------------------------\n"
	    "Risk %p:\n", m_risk.get());
    if(m_risk)
      dump_probabilities(*m_risk, 0, 0, xsize-1, ysize-1, stream);
  }
  
  
  void Flow::
  DumpPNF(FILE * stream)
    const
  {
    fprintf(stream,
	    "--------------------------------------------------\n"
	    "Probabilistic Navigation Function %p:\n", m_pnf.get());
    m_pnf->DumpPointers(stream);
    m_pnf->DumpGrid(stream);
  }
  
  
  const Facade * Flow::
  GetRobdist()
    const
  {
    if( ! m_robot)
      return 0;
    return m_robot->dist.get();
  }
  
  
  Facade * Flow::
  GetObjdist(size_t id, FILE * verbose_stream)
    const
  {
    objectmap_t::const_iterator io(m_object.find(id));
    if(m_object.end() == io){
      if(verbose_stream){
	fprintf(verbose_stream, "%s(): invalid id %zd, valid ones are",
		__func__, id);
	for(io = m_object.begin(); io != m_object.end(); ++io)
	  fprintf(verbose_stream, " %zd", io->first);
	fprintf(verbose_stream, "\n");
      }
      return 0;
    }
    return io->second->dist.get();
  }
  
  
  const Region * Flow::
  GetRobot()
    const
  {
    if(!m_robot)
      return 0;
    return m_robot->region.get();
  }
  
  
  const Region * Flow::
  GetRegion(size_t id)
    const
  {
    objectmap_t::const_iterator io(m_object.find(id));
    if(m_object.end() == io)
      return 0;
    return io->second->region.get();
  }
  
  
  Flow::array_info_t Flow::
  GetRobotLambda()
    const
  {
    if( ! m_robot)
      return make_pair(static_cast<const array<double> *>(0), -1.0);
    return make_pair(m_robot->lambda.get(), m_robot->max_lambda);
  }
  
  
  Flow::array_info_t Flow::
  GetObjectLambda(size_t id)
    const
  {
    objectmap_t::const_iterator io(m_object.find(id));
    if(m_object.end() == io)
      return make_pair(static_cast<const array<double> *>(0), -1.0);
    return make_pair(io->second->lambda.get(), io->second->max_lambda);
  }
  
  
  Flow::array_info_t Flow::
  GetEnvCooc()
    const
  {
    if(!m_env_cooc)
      return make_pair(static_cast<const array<double> *>(0), -1.0);
    return make_pair(m_env_cooc.get(), m_max_env_cooc);
  }
  
  
  Flow::array_info_t Flow::
  GetObjCooc(size_t id)
    const
  {
    objectmap_t::const_iterator io(m_object.find(id));
    if(m_object.end() == io)
      return make_pair(static_cast<const array<double> *>(0), -1.0);
    return make_pair(io->second->cooc.get(), io->second->max_cooc);
  }
  
  
  Flow::array_info_t Flow::
  GetDynamicCooc()
    const
  {
    if( ! m_dynamic_cooc)
      return make_pair(static_cast<const array<double> *>(0), -1.0);
    return make_pair(m_dynamic_cooc.get(), m_max_dynamic_cooc);
  }
  
  
  Flow::array_info_t Flow::
  GetRisk()
    const
  {
    if( ! m_risk)
      return make_pair(static_cast<const array<double> *>(0), -1.0);
    return make_pair(m_risk.get(), m_max_risk);
  }
  
  
  bool Flow::
  CompIndices(double x, double y, ssize_t & ix, ssize_t & iy)
    const
  {
    if((x < 0) || (y < 0))
      return false;
    ix = static_cast<ssize_t>(rint(x / resolution));
    if(ix >= xsize)
      return false;
    iy = static_cast<ssize_t>(rint(y / resolution));
    return iy < ysize;
  }
  
  
  boost::shared_ptr<estar::Grid const> Flow::
  GetEnvdistGrid() const
  {
    return m_envdist->GetGrid();
  }
  
}


void boost::assertion_failed(char const * expr, char const * function,
			     char const * file, long line)
{
  const std::string sexpr(expr);
  const char * hint("no hint available");
  if(sexpr == "m_robot")
    hint = "m_robot only valid after pnf::Flow::SetRobot()";
  if(sexpr == "m_goal")
    hint =
      "m_goal only valid after pnf::Flow::SetGoal()\n"
      "    can be caused by empty goal sets\n"
      "    see also 'repair' code in pnf::Sprite and pnf::Region ctors";
  fprintf(stderr,
	  "\n%s: %ld: assertion \"%s\" failed\n"
	  "  function: %s\n"
	  "  hint: %s\n",
	  file, line, expr, function, hint);
  reinterpret_cast<void (*)()>(0)(); // this might be a bit harsh...
}
