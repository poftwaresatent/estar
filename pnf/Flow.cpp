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
#include "RobotShape.hpp"
#include "BufferZone.hpp"
#include <estar/RiskMap.hpp>
#include <estar/numeric.hpp>
#include <estar/Facade.hpp>
#include <estar/Algorithm.hpp>
#include <estar/Grid.hpp>
#include <estar/Kernel.hpp>
#include <estar/dump.hpp>
#include <iostream>
#include <cmath>

#ifdef DEBUG
# define PNF_FLOW_DEBUG
#else // DEBUG
# undef PNF_FLOW_DEBUG
#endif // DEBUG

#ifdef PNF_FLOW_DEBUG
# define PDEBUG PDEBUG_OUT
#else
# define PDEBUG PDEBUG_OFF
#endif

#define PVDEBUG PDEBUG_OFF

#define BOOST_ENABLE_ASSERT_HANDLER
#include <boost/assert.hpp>


using estar::Facade;
using estar::square;
using estar::minval;
using estar::value_map_t;
using estar::Grid;
using estar::Algorithm;
using estar::Kernel;
using estar::dump_probabilities;
using boost::shared_ptr;
using boost::scoped_ptr;
using std::make_pair;
using std::cerr;


namespace pnf {
  
  
  Flow::
  Flow(size_t _xsize, size_t _ysize, double _resolution)
    : xsize(_xsize),
      ysize(_ysize),
      resolution(_resolution),
      half_diagonal(0.707106781187 * _resolution), // sqrt(1/2)
      m_envdist(Facade::Create("lsm", _xsize, _ysize, _resolution, false, 0)),
      // m_robot invalid until SetRobot()
      // m_object to be populated by AddDynamicObject()
      // m_risk invalid until ComputeRisk()
      m_pnf(Facade::Create("lsm", _xsize, _ysize, _resolution, false, 0))
      // m_goal invalid until SetGoal()
  {
  }
  
  
  Flow * Flow::
  Create(size_t xsize, size_t ysize, double resolution)
  {
    Flow * flow(new Flow(xsize, ysize, resolution));
    if(( ! flow->m_envdist) || ( ! flow->m_pnf)){
      delete flow;
      return 0;
    }
    return flow;
  }
  
  
  void Flow::
  AddStaticObject(size_t ix, size_t iy)
  {
    m_envdist->AddGoal(ix, iy, 0);
  }
  
  
  void Flow::
  RemoveStaticObject(size_t ix, size_t iy)
  {
    m_envdist->RemoveGoal(ix, iy);
  }
  
  
  bool Flow::
  SetDynamicObject(size_t id, double x, double y, double r, double v)
  {
    { // check if object inside grid
      size_t foo;
      if( ! CompIndices(x, y, foo, foo))
	return false;
    }
    
    // allocate new object
    Facade * dist(Facade::Create("lsm", xsize, ysize, resolution, false, 0));
    BOOST_ASSERT( dist );
    shared_ptr<object>
      obj(new object(new region(x, y, r, resolution, xsize, ysize),
		     v, dist, id, new estar::array<double>(xsize, ysize)));
    
    // do this in MapEnvdist() to keep goal out of obstacles:
    //   DoAddGoal(*obj->dist, *obj->footprint);
    
    // store it
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
  void Flow::
  DoSetRobot(double x, double y, size_t ix, size_t iy, double r, double v)
  {
    Facade * dist(Facade::Create("lsm", xsize, ysize, resolution, false, 0));
    BOOST_ASSERT( dist );
    m_robot.reset(new robot(new region(x, y, r, resolution, xsize, ysize),
			    v, dist, new RobotShape(r, resolution)));
    // do this in MapEnvdist() to keep goal out of obstacles:
    //   DoAddGoal(*m_robot->dist, *m_robot->footprint);
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
    
    const size_t nvertices(xsize * ysize);
    const value_map_t & envdist(m_envdist->GetAlgorithm().GetValueMap());
    const Grid & envgrid(m_envdist->GetGrid());
    
    // if you want to be paranoid, this should be specific for each
    // object as well as the robot... but they're all using the same
    // kernel anyways
    BOOST_ASSERT( m_robot );
    const double freespace(m_robot->dist->GetFreespaceMeta());
    const double obstacle(m_robot->dist->GetObstacleMeta());
    
    { // Robot obstacles:
      const double radius(m_robot->footprint->rad + half_diagonal);
      const Grid & robgrid(m_robot->dist->GetGrid());
      Algorithm & robalgo(m_robot->dist->GetAlgorithm());
      const Kernel & robkernel(m_robot->dist->GetKernel());
      for(size_t iv(0); iv < nvertices; ++iv)
	if(get(envdist, envgrid.GetVertex(iv)) > radius)
	  robalgo.SetMeta(robgrid.GetVertex(iv), freespace, robkernel);
	else
	  robalgo.SetMeta(robgrid.GetVertex(iv), obstacle, robkernel);
      // set robot goal, this will skip obstacles
      DoAddGoal(*m_robot->dist, *m_robot->footprint);
    }
    
    { // Object obstacles:
      for(objectmap_t::iterator io(m_object.begin()); io != m_object.end();
	  ++io){
	const double radius(io->second->footprint->rad + half_diagonal);
	Facade & objdist(*io->second->dist);
	const Grid & objgrid(objdist.GetGrid());
	Algorithm & objalgo(objdist.GetAlgorithm());
	const Kernel & objkernel(objdist.GetKernel());
	for(size_t iv(0); iv < nvertices; ++iv)
	  if(get(envdist, envgrid.GetVertex(iv)) > radius)
	    objalgo.SetMeta(objgrid.GetVertex(iv), freespace, objkernel);
	  else
	    objalgo.SetMeta(objgrid.GetVertex(iv), obstacle, objkernel);
	// set object goal, this will skip obstacles
	DoAddGoal(objdist, *io->second->footprint);
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
  }
  
  
  void Flow::
  DoComputeCooc(object & obj)
  {
    BOOST_ASSERT( m_robot );
    const value_map_t & robdist(m_robot->dist->GetAlgorithm().GetValueMap());
    const Grid & robgrid(m_robot->dist->GetGrid());
    const value_map_t & objdist(obj.dist->GetAlgorithm().GetValueMap());
    const Grid & objgrid(obj.dist->GetGrid());
    obj.max_cooc = 0;
    for(size_t ix(0); ix < xsize; ++ix)
      for(size_t iy(0); iy < ysize; ++iy){
	const double cooc = pnf_cooc(get(objdist, objgrid.GetVertex(ix, iy)),
				     get(robdist, robgrid.GetVertex(ix, iy)),
				     obj.speed, m_robot->speed, resolution);
	if(cooc > obj.max_cooc)
	  obj.max_cooc = cooc;
	(*obj.cooc)[ix][iy] = cooc;
      }
  }
  
  
  void Flow::
  ComputeAllCooc()
  {
    for(objectmap_t::iterator io(m_object.begin());
	io != m_object.end(); ++io)
      DoComputeCooc(*io->second);
  }
  
  
  /** \todo Maybe use non-facade access for efficiency. Hardcoded
      buffer parameters if compiling with STATIC_DIRACS */ 
  void Flow::
  ComputeRisk(const estar::RiskMap & risk_map,
	      const BufferZone & buffer)
  {
    BOOST_ASSERT( m_robot );
    estar::array<double> accu(xsize, ysize);
    
    // - compute combined probability as (1 - Sigma(1 - p))
    // - convolute with robot shape
    // - use risk map to transform result into PNF's meta values
    
    // initialize with environment co-occurrence, ie the static object
    // bitmap: goal cells are static objects, their co-occurrence is
    // 1, thus we initialize with (1-p)=(1-1)=0

    const value_map_t & envdist(m_envdist->GetAlgorithm().GetValueMap());
    const Grid & envgrid(m_envdist->GetGrid());
    for(size_t ix(0); ix < xsize; ++ix)
      for(size_t iy(0); iy < ysize; ++iy){
	const double dist(get(envdist, envgrid.GetVertex(ix, iy)));
	accu[ix][iy] = 1 - buffer.DistanceToRisk(dist);
      }
    
    // loop over all dynamic objects and multiply the corresponding
    // risk by (1 - cooc)
    for(objectmap_t::const_iterator io(m_object.begin());
	io != m_object.end(); ++io)
      for(size_t ix(0); ix < xsize; ++ix)
	for(size_t iy(0); iy < ysize; ++iy)
	  accu[ix][iy] *= 1 - (*io->second->cooc)[ix][iy];
    
    // convolute 1-risk by the robot shape and apply risk map to
    // produce meta information; first calculate all 1-risk because
    // the convolution needs to have them all precalculated!
    // 
    // also keep a copy of the accu array at this point, for plotting
    // the state before the "C-space transform"
    m_wspace_risk.reset(new estar::array<double>(xsize, ysize));
    m_max_wspace_risk = 0;
    for(size_t ix(0); ix < xsize; ++ix)
      for(size_t iy(0); iy < ysize; ++iy){
	const double acc(1 - accu[ix][iy]);
	(*m_wspace_risk)[ix][iy] = acc;
	if((acc < 1) && (acc > m_max_wspace_risk))
	  m_max_wspace_risk = acc;
	accu[ix][iy] = acc;
      }
    
    m_risk.reset(new estar::array<double>(xsize, ysize));
    m_max_risk = 0;
    for(size_t ix(0); ix < xsize; ++ix)
      for(size_t iy(0); iy < ysize; ++iy){
	const double
	  risk(m_robot->mask->FuseRisk(ix, iy, xsize, ysize, accu));
	(*m_risk)[ix][iy] = risk;
	m_pnf->SetMeta(ix, iy, risk_map.RiskToMeta(risk));
	if(risk > m_max_risk)
	  m_max_risk = risk;
      }

    // this is a bit of a hack that depend on the exact call order
    DoAddGoal(*m_pnf, *m_goal);
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
    const size_t ix(static_cast<size_t>(rint(x / resolution)));
    if(ix >= xsize)
      return false;
    const size_t iy(static_cast<size_t>(rint(y / resolution)));
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
    const size_t ix(static_cast<size_t>(rint(x / resolution)));
    if(ix >= xsize)
      return false;
    const size_t iy(static_cast<size_t>(rint(y / resolution)));
    if(iy >= ysize)
      return false;
    RemoveStaticObject(ix, iy);
    return true;
  }
  
  
  bool Flow::
  SetRobot(double x, double y, double r, double v)
  {
    size_t ix, iy;
    if( ! CompIndices(x, y, ix, iy))
      return false;
    DoSetRobot(x, y, ix, iy, r, v);
    return true;
  }
  
  
  /** \todo URGENT! The goal vs. obstacle problem IS NOT SOLVED! */
  bool Flow::
  SetGoal(double x, double y, double r)
  {
    if(m_goal)
      DoRemoveGoal(*m_pnf, *m_goal);
    scoped_ptr<region> goal(new region(x, y, r, resolution, xsize, ysize));
    if(goal->nodelist.empty()){
      m_goal.reset();
      return false;
    }
    // do this in ComputeRisk() to keep goal out of obstacles:
    //    DoAddGoal(*m_pnf, *goal);
    m_goal.swap(goal);
    return true;
  }
  
  
  void Flow::
  DumpEnvdist(FILE * stream)
    const
  {
    fprintf(stream,
	    "--------------------------------------------------\n"
	    "Environment distance 0x%08X:\n", m_envdist.get());
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
	    "Robot distance 0x%08X:\n", m_robot->dist.get());
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
      fprintf(stream, "distance from object %zd 0x%08X:\n",
	      io->second.get(), id);
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
	    "Risk 0x%08X:\n", m_risk.get());
    if(m_risk)
      dump_probabilities(*m_risk, 0, 0, xsize-1, ysize-1, stream);
  }
  
  
  void Flow::
  DumpPNF(FILE * stream)
    const
  {
    fprintf(stream,
	    "--------------------------------------------------\n"
	    "Probabilistic Navigation Function 0x%08X:\n", m_pnf.get());
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
  
  
  std::pair<const estar::array<double> *, double> Flow::
  GetCooc(size_t id)
    const
  {
    objectmap_t::const_iterator io(m_object.find(id));
    if(m_object.end() == io)
      return make_pair(static_cast<const estar::array<double> *>(0), -1.0);
    return make_pair(io->second->cooc.get(), io->second->max_cooc);
  }
  
  
  std::pair<const estar::array<double> *, double> Flow::
  GetRisk()
    const
  {
    if( ! m_risk)
      return make_pair(static_cast<const estar::array<double> *>(0), -1.0);
    return make_pair(m_risk.get(), m_max_risk);
  }
  
  
  std::pair<const estar::array<double> *, double> Flow::
  GetWSpaceRisk()
    const
  {
    if( ! m_wspace_risk)
      return make_pair(static_cast<const estar::array<double> *>(0), -1.0);
    return make_pair(m_wspace_risk.get(), m_max_wspace_risk);
  }
  
  
  bool Flow::
  CompIndices(double x, double y, size_t & ix, size_t & iy)
    const
  {
    if((x < 0) || (y < 0))
      return false;
    ix = static_cast<size_t>(rint(x / resolution));
    if(ix >= xsize)
      return false;
    iy = static_cast<size_t>(rint(y / resolution));
    return iy < ysize;
  }

  
  /** \todo This will add the center vertex twice in most cases! */
  Flow::region::
  region(double _x, double _y, double _rad,
	 double scale, size_t xsize, size_t ysize)
    : x(_x), y(_y), rad(_rad)
  {
    const double r2(square(_rad));
    const size_t xmax(xsize - 1);
    const size_t ymax(ysize - 1);
    
    if((_x >= 0) && (_y >= 0)){
      const size_t xc(static_cast<size_t>(rint(_x / scale)));
      const size_t yc(static_cast<size_t>(rint(_y / scale)));
      if((xc <= xmax) && (yc <= ymax)){
	PVDEBUG("center indices:   %zd   %zd\n", xc, yc);
	nodelist.push_back(node(xc, yc, 0));
      }
    }
#ifdef PNF_FLOW_DEBUG
    if(nodelist.empty())
      PDEBUG("goal center not in grid\n");
#endif // PNF_FLOW_DEBUG
    
    const double x0((_x - _rad) / scale);
    size_t ix0(0);
    if(x0 > 0) ix0 = minval(static_cast<size_t>(rint(x0)), xmax);
    
    const double y0((_y - _rad) / scale);
    size_t iy0(0);
    if(y0 > 0) iy0 = minval(static_cast<size_t>(rint(y0)), ymax);
    
    const double x1((_x + _rad) / scale);
    size_t ix1(0);
    if(x1 > 0) ix1 = minval(static_cast<size_t>(rint(x1)), xmax);
    
    const double y1((_y + _rad) / scale);
    size_t iy1(0);
    if(y1 > 0) iy1 = minval(static_cast<size_t>(rint(y1)), ymax);
    
    PVDEBUG("bound indices:   %zd   %zd   %zd   %zd\n", ix0, iy0, ix1, iy1);
    
    for(size_t ix(ix0); ix <= ix1; ++ix)
      for(size_t iy(iy0); iy <= iy1; ++iy){
	const double dr2(square(ix * scale - _x) + square(iy * scale - _y));
	if(dr2 <= r2)
	  nodelist.push_back(node(ix, iy, sqrt(dr2)));
      }
  }


  void Flow::
  DoAddGoal(estar::Facade & facade, const region & goal)
  {
    const double obstacle(facade.GetObstacleMeta());
    for(region::nodelist_t::const_iterator in(goal.nodelist.begin());
	in != goal.nodelist.end(); ++in)
      if(facade.GetMeta(in->ix, in->iy) != obstacle)
	facade.AddGoal(in->ix, in->iy, in->val);
  }
  
  
  void Flow::
  DoRemoveGoal(estar::Facade & facade, const region & goal)
  {
    for(region::nodelist_t::const_iterator in(goal.nodelist.begin());
	in != goal.nodelist.end(); ++in)
      facade.RemoveGoal(in->ix, in->iy);
  }

  
  Flow::baseobject::
  baseobject(region * _footprint, double _speed, estar::Facade * _dist)
    : footprint(_footprint), speed(_speed), dist(_dist)
  {
  }
  
  
  Flow::object::
  object(region * footprint, double speed, estar::Facade * dist,
	 const size_t _id, estar::array<double> * _cooc)
    : baseobject(footprint, speed, dist), id(_id), cooc(_cooc), max_cooc(-1)
  {
  }
  
  
  Flow::robot::
  robot(region * footprint, double speed, estar::Facade * dist,
	RobotShape * _mask)
    : baseobject(footprint, speed, dist), mask(_mask)
  {
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
    hint = "m_goal only valid after pnf::Flow::SetGoal()";
  fprintf(stderr,
	  "\n%s: %ld: assertion \"%s\" failed\n"
	  "  function: %s\n"
	  "  hint: %s\n",
	  file, line, expr, function, hint);
  reinterpret_cast<void (*)()>(0)(); // this might be a bit harsh...
}
