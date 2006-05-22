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


#include <pnf/Flow.hpp>
#include <pnf/BufferZone.hpp>
#include <pnf/PNFRiskMap.hpp>
#include <pnf/RobotShape.hpp>
#include <gfx/Viewport.hpp>
#include <gfx/MetaMousehandler.hpp>
#include <gfx/wrap_glu.hpp>
#include <gfx/wrap_glut.hpp>
#include <estar/util.hpp>
#include <estar/graphics.hpp>
#include <estar/numeric.hpp>
#include <estar/Algorithm.hpp>
#include <estar/LSMKernel.hpp>
#include <estar/Grid.hpp>
#include <estar/Facade.hpp>
#include <estar/check.hpp>
#include <estar/dump.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>


#define USE_GL
#undef USE_DEPTH_BUFFER


using namespace pnf;
using namespace estar;
using namespace gfx;
using namespace boost;
using namespace std;


static void parse_options(int argc, char ** argv);
static void parse_config(istream & config_is, ostream & dbg);

static void init_glut(int * argc, char ** argv, int width, int height);
static void reshape(int width, int height);
static void draw();
static void keyboard(unsigned char key, int x, int y);
static void timer(int handle);
static void mouse(int button, int state, int x, int y);
static void motion(int x, int y);

static void add_wall(double x0, double y0, double x1, double y1, Flow & flow);
static void dump_objdist();
static void dump_lambda();
static void dump_cooc();
static void dump_risk();
static void do_dump(const string & name,
		    const array<double> & data, double max_val);
static void do_dump(const string & name, const Facade & facade);

static void draw_setup(double wall_r, double wall_g, double wall_b,
		       double goal_r, double goal_g, double goal_b,
		       double robot_r, double robot_g, double robot_b,
		       double object_r, double object_g, double object_b,
		       bool plotreg);
static void draw_setup(bool inv, bool plotreg);


namespace local {
  
  /** Utility for reading options and config file. */
  class Config
  {
  public:
    /** Placeholder for a wall, four coordinates. */
    typedef struct { double x0, y0, x1, y1; } wall_t;
    typedef vector<wall_t> statobj_t;
    
    /** Placeholder for am object: location, radius, speed. */
    typedef struct { size_t id; double x, y, r, v; } object_t;
    typedef vector<object_t> dynobj_t;
    
    size_t grid_x, grid_y;
    double grid_d;
    statobj_t statobj;
    double robot_x, robot_y, robot_r, robot_v;
    dynobj_t dynobj;
    double goal_x, goal_y, goal_r;
    bool paper;
    double robot_buffer_factor, robot_buffer_degree;
    double object_buffer_factor, object_buffer_degree;
  };
  
}

using namespace local;


static const unsigned int timer_delay(100);

static bool m_step(false);
static bool m_continuous(true);

static bool m_debug(false);
static bool m_finish(true);

static bool m_dump_envdist(true);
static bool m_dump_objdist(true);
static bool m_dump_lambda(true);
static bool m_dump_robdist(true);
static bool m_dump_cooc(true);
static bool m_dump_risk(true);
static bool m_dump_pnf(true);

static size_t m_flowstep(0);

static shared_ptr<Viewport> m_envdist_view;
static shared_ptr<Viewport> m_robdist_view;
static vector<shared_ptr<Viewport> > m_lambda_view;
static vector<shared_ptr<Viewport> > m_cooc_view;
static shared_ptr<Viewport> m_risk_view;
static shared_ptr<Viewport> m_wspace_risk_view;
static shared_ptr<Viewport> m_pnf_meta_view;
static shared_ptr<Viewport> m_pnf_value_view;

static shared_ptr<RiskMap> m_risk_map;
static shared_ptr<Flow> m_flow;
static shared_ptr<Config> m_config;


int main(int argc,
	 char ** argv)
{
#ifdef USE_GL
  init_glut(& argc, argv, 400, 400);
#endif // USE_GL
  parse_options(argc, argv);
  
#ifndef USE_GL

  m_flow->DumpEnvdist(stdout);
  m_flow->DumpRobdist(stdout);
  for(size_t io(0); io < m_config->dynobj.size(); ++io){
    m_flow->DumpObjdist(m_config->dynobj[io].id, stdout);
    m_flow->DumpObjCooc(m_config->dynobj[io].id, stdout);
  }
  m_flow->DumpCollprob(stdout);
  m_flow->DumpPNF(stdout);

#else // USE_GL

  typedef Subwindow::logical_bbox_t bb_t;
  typedef Viewport VP;
  if(m_config->paper){
    double x0, y0, x1, y1;
    get_grid_bbox(m_flow->GetEnvdist().GetGrid(), x0, y0, x1, y1);
    const bb_t rbb(x0, y0, x1, y1);
    const bb_t lbb0(0, 0, 0, 0);
    m_envdist_view.reset(new VP("envdist", rbb, lbb0));
    const double dy(1.0 / (m_config->dynobj.size() + 1.0));
    m_robdist_view.reset(new VP("robdist", rbb, bb_t(0, 1-dy, 0.5, 1)));
    for(size_t io(0); io < m_config->dynobj.size(); ++io){
      {
	ostringstream os("lambda");
	os << m_config->dynobj[io].id;
	m_lambda_view.push_back(shared_ptr<VP>(new VP(os.str(), rbb, lbb0)));
      }
      {
	ostringstream os("cooc");
	os << m_config->dynobj[io].id;
	const bb_t lbb(0, io*dy, 0.5, (io+1)*dy);
	m_cooc_view.push_back(shared_ptr<VP>(new VP(os.str(), rbb, lbb)));
      }
    }
    {
      const double dy(1.0 / 3);
      m_wspace_risk_view.reset(new VP("risk", rbb, bb_t(0.5, 2*dy, 1, 1)));
      m_risk_view.reset(new VP("risk", rbb, bb_t(0.5, dy, 1, 2*dy)));
      m_pnf_meta_view.reset(new VP("pnf_meta", rbb, bb_t(0.5, 0, 1, dy)));
    }
    m_pnf_value_view.reset(new VP("pnf_value", rbb, lbb0));
  }
  else{
    static const bb_t rbb(0, 0, m_flow->xsize, m_flow->ysize);
    const double ldx(0.25);
    const double ldy(1.0 / (m_config->dynobj.size() + 1));
    m_envdist_view.reset(new VP("envdist", rbb, bb_t(0, 1-ldy, ldx, 1)));
    m_robdist_view.reset(new VP("robdist", rbb, bb_t(ldx, 1-ldy, 2*ldx, 1)));
    for(size_t io(0); io < m_config->dynobj.size(); ++io){
      {
	ostringstream os("lambda");
	os << m_config->dynobj[io].id;
	const bb_t lbb(0, io*ldy, ldx, (io+1)*ldy);
	m_lambda_view.push_back(shared_ptr<VP>(new VP(os.str(), rbb, lbb)));
      }
      {
	ostringstream os("cooc");
	os << m_config->dynobj[io].id;
	const bb_t lbb(ldx, io*ldy, 2*ldx, (io+1)*ldy);
	m_cooc_view.push_back(shared_ptr<VP>(new VP(os.str(), rbb, lbb)));
      }
    }
    m_risk_view.reset(new VP("risk", rbb, bb_t(2*ldx, 0.5, 3*ldx, 1)));
    m_pnf_meta_view.reset(new VP("pnf_meta", rbb, bb_t(3*ldx, 0.5, 1, 1)));
    m_pnf_value_view.reset(new VP("pnf_value", rbb, bb_t(2*ldx, 0, 1, 0.5)));
  }
  
  glutMainLoop();
  
#endif // USE_GL
}


void init_glut(int * argc, char ** argv,
	       int width, int height)
{
  glutInit(argc, argv);
#ifdef USE_DEPTH_BUFFER
  glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);
#else
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
#endif
  glutInitWindowPosition(0, 0);
  glutInitWindowSize(width, height);
  
  int handle(glutCreateWindow("ESTAR"));
  if(0 == handle){
    cerr << argv[0] << ": init_glut(): couldn't create parent window\n";
    exit(EXIT_FAILURE);
  }
  
  glutDisplayFunc(draw);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutTimerFunc(timer_delay, timer, handle);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
}


void draw()
{
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.5, 0.5, 0.5, 0);
#ifdef USE_DEPTH_BUFFER
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
  glClear(GL_COLOR_BUFFER_BIT);
#endif
  
  if(m_config->paper){
    m_robdist_view->PushProjection();
    if(4 > m_flowstep){
      draw_grid_value(m_flow->GetEnvdist(), ColorScheme::Get(INVERTED_GREY));
      draw_setup(false, false);
    }
    else{
      BOOST_ASSERT( 0 != m_flow->GetRobdist() );
      draw_grid_meta(*m_flow->GetRobdist(), ColorScheme::Get(INVERTED_GREY));
      if(7 <= m_flowstep)
	draw_trace(m_flow->GetPNF(), m_config->robot_x, m_config->robot_y,
		   m_config->goal_r, ColorScheme::Get(RED), 0, 1, 1);
      draw_setup(false, false);
    }
    m_robdist_view->PopProjection();
    double max_max_cooc(0);
    if(3 <= m_flowstep)
      for(size_t io(0); io < m_config->dynobj.size(); ++io){
	m_cooc_view[io]->PushProjection();
	if(5 > m_flowstep){
	  BOOST_ASSERT( 0 != m_flow->GetObjdist(m_config->dynobj[io].id));
	  draw_grid_value(*m_flow->GetObjdist(m_config->dynobj[io].id),
			  ColorScheme::Get(INVERTED_GREY));    
	  draw_setup(false, false);
	}
	else{
	  const array<double> * cooc;
	  double max_cooc;
	  tie(cooc, max_cooc) = m_flow->GetCooc(m_config->dynobj[io].id);
	  BOOST_ASSERT( 0 != cooc);
	  if(max_cooc > max_max_cooc)
	    max_max_cooc = max_cooc;
	  draw_array(*cooc, 0, 0, m_flow->xsize - 1, m_flow->ysize - 1,
		     0, max_cooc, ColorScheme::Get(INVERTED_GREY));
	  if(7 <= m_flowstep)
	    draw_trace(m_flow->GetPNF(), m_config->robot_x, m_config->robot_y,
		       m_config->goal_r, ColorScheme::Get(RED), 0, 1, 1);
	  draw_setup(false, false);
	}
	m_cooc_view[io]->PopProjection();
      }
    if(6 <= m_flowstep){
      {
	const array<double> * wspace_risk;
	double max_wspace_risk;
	tie(wspace_risk, max_wspace_risk) = m_flow->GetWSpaceRisk();
	if(0 != wspace_risk){
	  m_wspace_risk_view->PushProjection();
	  draw_array(*wspace_risk, 0, 0, m_flow->xsize - 1, m_flow->ysize - 1,
		     0, max_max_cooc, ColorScheme::Get(INVERTED_GREY));
	  if(7 <= m_flowstep)
	    draw_trace(m_flow->GetPNF(), m_config->robot_x, m_config->robot_y,
		       m_config->goal_r, ColorScheme::Get(RED), 0, 1, 1);
	  draw_setup(false, false);
	  m_wspace_risk_view->PopProjection();
	}
      }
      {
	const array<double> * risk;
	double max_risk;
	tie(risk, max_risk) = m_flow->GetRisk();
	if(0 != risk){
	  m_risk_view->PushProjection();
	  draw_array(*risk, 0, 0, m_flow->xsize - 1, m_flow->ysize - 1,
		     0, max_risk, ColorScheme::Get(INVERTED_GREY));
	  if(7 <= m_flowstep)
	    draw_trace(m_flow->GetPNF(), m_config->robot_x, m_config->robot_y,
		       m_config->goal_r, ColorScheme::Get(RED), 0, 1, 1);
	  draw_setup(true, false);
	  m_risk_view->PopProjection();
	}
      }
      m_pnf_meta_view->PushProjection();
      draw_grid_meta(m_flow->GetPNF(), ColorScheme::Get(INVERTED_GREY));
      if(7 <= m_flowstep)
	draw_trace(m_flow->GetPNF(), m_config->robot_x, m_config->robot_y,
		   m_config->goal_r, ColorScheme::Get(RED), 0, 1, 1);
      draw_setup(false, false);
      m_pnf_meta_view->PopProjection();
    }
  }
  else{
    if(0 <= m_flowstep){
      m_envdist_view->PushProjection();
      draw_grid_value(m_flow->GetEnvdist(), ColorScheme::Get(BLUE_GREEN_RED));
      draw_grid_queue(m_flow->GetEnvdist().GetGrid(),
		      m_flow->GetEnvdist().GetAlgorithm());
      //       draw_grid_upwind(m_flow->GetEnvdist().GetGrid(),
      // 		       m_flow->GetEnvdist().GetAlgorithm(),
      // 		       1, 1, 1, 1);
      draw_setup(false, true);
      m_envdist_view->PopProjection();
    }
    if(3 <= m_flowstep){
      for(size_t io(0); io < m_config->dynobj.size(); ++io){
	const array<double> * lambda;
	double max_lambda;
	tie(lambda, max_lambda) = m_flow->GetLambda(m_config->dynobj[io].id);
	if(0 == lambda){
	  cerr << __func__ << "(): lambda " << m_config->dynobj[io].id
	       << " not in m_flow\n";
	  exit(EXIT_FAILURE);
	}
	m_lambda_view[io]->PushProjection();
	draw_array(*lambda, 0, 0, m_flow->xsize - 1, m_flow->ysize - 1,
		   0, max_lambda, ColorScheme::Get(BLUE_GREEN_RED));
	draw_setup(false, true);
	const Region * region(m_flow->GetRegion(m_config->dynobj[io].id));
	if(0 == region){
	  cerr << __func__ << "(): region " << m_config->dynobj[io].id
	       << " not in m_flow\n";
	  exit(EXIT_FAILURE);
	}
	m_lambda_view[io]->PopProjection();
      }
    }
    if(4 <= m_flowstep){
      const Facade * robdist(m_flow->GetRobdist());
      if(0 != robdist){
	m_robdist_view->PushProjection();
	draw_grid_value(*robdist, ColorScheme::Get(BLUE_GREEN_RED));
	draw_setup(false, true);
	m_robdist_view->PopProjection();
      }
      else{
	cerr << __func__ << "(): robdist not in m_flow\n";
	exit(EXIT_FAILURE);
      }
    }
    if(5 <= m_flowstep){
      for(size_t io(0); io < m_config->dynobj.size(); ++io){
	const array<double> * cooc;
	double max_cooc;
	tie(cooc, max_cooc) = m_flow->GetCooc(m_config->dynobj[io].id);
	if(0 == cooc){
	  cerr << __func__ << "(): cooc " << m_config->dynobj[io].id
	       << " not in m_flow\n";
	  exit(EXIT_FAILURE);
	}
	m_cooc_view[io]->PushProjection();
	draw_array(*cooc, 0, 0, m_flow->xsize - 1, m_flow->ysize - 1,
		   0, max_cooc, ColorScheme::Get(BLUE_GREEN_RED));
	draw_setup(false, true);
	m_cooc_view[io]->PopProjection();
      }
    }
    if(6 <= m_flowstep){
      const array<double> * risk;
      double max_risk;
      tie(risk, max_risk) = m_flow->GetRisk();
      if(0 != risk){
	m_risk_view->PushProjection();
	draw_array(*risk, 0, 0, m_flow->xsize - 1, m_flow->ysize - 1,
		   0, max_risk, ColorScheme::Get(BLUE_GREEN_RED));
	if(7 <= m_flowstep)
	  draw_trace(m_flow->GetPNF(), m_config->robot_x, m_config->robot_y,
		     m_config->goal_r, ColorScheme::Get(RED), 0, 1, 1);
	draw_setup(false, true);
	m_risk_view->PopProjection();
      }
      m_pnf_meta_view->PushProjection();
      draw_grid_meta(m_flow->GetPNF(), ColorScheme::Get(BLUE_GREEN_RED));
      if(7 <= m_flowstep)
	draw_trace(m_flow->GetPNF(), m_config->robot_x, m_config->robot_y,
		   m_config->goal_r, ColorScheme::Get(RED), 0, 1, 1);
      draw_setup(false, true);
      m_pnf_meta_view->PopProjection();
    }
    if(7 <= m_flowstep){
      m_pnf_value_view->PushProjection();
      draw_grid_value(m_flow->GetPNF(), ColorScheme::Get(BLUE_GREEN_RED));
      draw_trace(m_flow->GetPNF(), m_config->robot_x, m_config->robot_y,
		 m_config->goal_r, ColorScheme::Get(RED), 0, 1, 1);
      draw_setup(false, true);
      m_pnf_value_view->PopProjection();
    }
  }
  
  glFlush();
  glutSwapBuffers();
}


void reshape(int width, int height)
{
  Subwindow::DispatchResize(Subwindow::screen_point_t(width, height));
}


void keyboard(unsigned char key, int x, int y)
{
  switch(key){
  case ' ':
    m_step = true;
    m_continuous = false;
    break;
  case 'c':
    m_step = false;
    m_continuous = true;
    break;
  case 'd':
    m_debug = ! m_debug;
    if(m_debug)
      cout << "debug switched ON\n";
    break;
  case 'f':
    m_finish = ! m_finish;
    if(m_finish)
      cout << "finish switched ON\n";
    else
      cout << "finish switched OFF\n";
    break;
  case 's':
    ++m_flowstep;
    if(m_debug)
      cout << "skipped flowstep to " << m_flowstep << "\n";
    break;
  case 'x':
    cout << "checking environment distance queue...\n";
    if( ! check_queue(m_flow->GetEnvdist().GetAlgorithm(),
		      &(m_flow->GetEnvdist().GetGrid()), "", cout))
      cout << "INCONSISTENT QUEUE in environment distance\n"
	   << "==================================================\n";
    else
      cout << "looks fine.\n";
    break;
  case 'q':
    exit(EXIT_SUCCESS);
    break;
  }
}


void timer(int handle)
{
  static size_t prev_flowstep(m_flowstep + 1);
  
  if(m_step || m_continuous){
    m_step = false;
    
    if(prev_flowstep != m_flowstep)
      cout << __func__ << "(): flowstep " << m_flowstep << ": ";
    
    if(0 == m_flowstep){
      if(m_flow->HaveEnvdist()){
	if(prev_flowstep != m_flowstep)
	  cout << "already have envdist...\n";
	++m_flowstep;
      }
      else{
	if(prev_flowstep != m_flowstep)
	  cout << "propagate envdist\n";
	if(m_finish)
	  m_flow->PropagateEnvdist(false);
	else
	  m_flow->PropagateEnvdist(true);
	if(m_debug){
	  const Algorithm & algo(m_flow->GetEnvdist().GetAlgorithm());
	  const Grid & grid(m_flow->GetEnvdist().GetGrid());
	  dump_queue(algo, &grid, 10, stdout);
	  const vertex_t vertex(algo.GetLastComputedVertex());
	  const GridNode & gn(grid.Vertex2Node(vertex));
	  static const size_t dx(2);
	  static const size_t dy(2);
	  const size_t x0(gn.ix <= dx ? 0 : gn.ix - dx);
	  const size_t y0(gn.iy <= dy ? 0 : gn.iy - dy);
	  const size_t x1(minval(gn.ix + dx, grid.xsize - 1));
	  const size_t y1(minval(gn.iy + dy, grid.ysize - 1));
	  dump_grid_range_highlight(grid, x0, y0, x1, y1,
				    gn.ix, gn.iy, stdout);
	  //BOOST_ASSERT( ! (get(algo.GetFlagMap(), vertex) & OPEN) );
	}
	if(m_flow->HaveEnvdist()){
	  if(m_dump_envdist)
	    do_dump("envdist", m_flow->GetEnvdist());
	  ++m_flowstep;
	}
      }
    }
    
    else if(1 == m_flowstep){
      cout << "map envdist\n";
      m_flow->_MapEnvdist(m_config->robot_buffer_factor,
			 m_config->robot_buffer_degree,
			 m_config->object_buffer_degree,
			 m_config->object_buffer_degree,
			 m_risk_map.get());
      ++m_flowstep;
    }
    
    else if(2 == m_flowstep){
      if(m_flow->HaveAllObjdist()){
	cout << "already have all objdist...\n";
	++m_flowstep;
      }
      else{
	static size_t io(0);
	if(io < m_config->dynobj.size()){
	  const size_t id(m_config->dynobj[io].id);
	  if(m_flow->HaveObjdist(id))
	    cout << "already have objdist " << id << "...\n";
	  else{
	    cout << "propagate objdist " << id << "\n";
	    m_flow->PropagateObjdist(id);
	  }
	  if( ! m_flow->HaveObjdist(id))
	    cout << "  OOPS: couldn't propagate objdist " << id << "...\n";
	  else
	    ++io;
	}
	if(m_flow->HaveAllObjdist()){
	  if(m_dump_objdist)
	    dump_objdist();
	  if(m_dump_lambda)
	    dump_lambda();
	  ++m_flowstep;
	}
      }
    }
    
    else if(3 == m_flowstep){
      if(m_flow->HaveRobdist())
	cout << "already have robdist...\n";
      else{
	cout << "propagate robdist\n";
	m_flow->PropagateRobdist();
      }
      if( ! m_flow->HaveRobdist())
	cout << "  OOPS: couldn't propagate robdist...\n";
      else{
	if(m_dump_robdist){
	  const Facade * robdist(m_flow->GetRobdist());
	  if( ! robdist){
	    cout << __func__ << "(): robdist not in m_flow\n";
	    exit(EXIT_FAILURE);
	  }
	  do_dump("robdist", *robdist);
	}
	++m_flowstep;
      }
    }
    
    else if(4 == m_flowstep){
      cout << "compute co-occurrences\n";
      m_flow->ComputeAllCooc();
      if(m_dump_cooc)
	dump_cooc();
      ++m_flowstep;
    }
    
    else if(5 == m_flowstep){
      cout << "compute risk\n";
      const BufferZone buffer(0, 3 * m_flow->half_diagonal, 2);
      m_flow->ComputeRisk(*m_risk_map, buffer);
      if(m_dump_risk)
	dump_risk();
      ++m_flowstep;
    }
    
    else if(6 == m_flowstep){
      if(m_flow->HavePNF())
	cout << "already have PNF...\n";
      else{
	cout << "propagate PNF\n";
	m_flow->PropagatePNF();
      }
      if( ! m_flow->HavePNF())
	cout << "  OOPS: couldn't propagate PNF...\n";
      else{
	if(m_dump_pnf)
	  do_dump("pnf", m_flow->GetPNF());
	++m_flowstep;
      }
    }
    
    else{
      cout << "nothing left to do\n";
      m_continuous = false;
    }
    
    prev_flowstep = m_flowstep;
  }
  
  Subwindow::DispatchUpdate();
  glutSetWindow(handle);
  glutPostRedisplay();
  glutTimerFunc(timer_delay, timer, handle);
}


void mouse(int button, int state, int x, int y)
{
  Subwindow::DispatchClick(button, state,
			   Subwindow::screen_point_t(x, y));
}


void motion(int x, int y)
{
  Subwindow::DispatchDrag(Subwindow::screen_point_t(x, y));
}


void parse_options(int argc, char ** argv)
{
  m_config.reset(new Config());
  m_config->robot_buffer_factor = -1;
  m_config->robot_buffer_degree = -1;
  m_config->object_buffer_factor = -1;
  m_config->object_buffer_degree = -1;
  
  string config_name;
  if(argc > 1)
    config_name = argv[1];
  else
    config_name = "setup.pnf";
  m_config->paper = false;
  if(argc > 2)
    if(string(argv[2]) == "paper"){
      m_config->paper = true;
      m_finish = true;
    }
    else
      cout << "ignoring second argument \"" << argv[2] << "\"\n";
  
  ifstream config_is(config_name.c_str());
  if( ! config_is){
    cerr << __func__ << "(): can't read config file \""
	 << config_name << "\"\n";
    exit(EXIT_FAILURE);
  }
  parse_config(config_is, cerr);
  
  m_flow.reset(Flow::Create(m_config->grid_x,
			    m_config->grid_y,
			    m_config->grid_d));
  if( ! m_flow){
    cerr << "Flow::Create() failed in " << __FUNCTION__ << ".\n";
    exit(EXIT_FAILURE);
  }
  
  m_flowstep = 0;
  for(size_t iw(0); iw < m_config->statobj.size(); ++iw)
    add_wall(m_config->statobj[iw].x0, m_config->statobj[iw].y0,
	     m_config->statobj[iw].x1, m_config->statobj[iw].y1,
	     * m_flow);
  for(size_t io(0); io < m_config->dynobj.size(); ++io)
    if(!m_flow->SetDynamicObject(m_config->dynobj[io].id,
				 m_config->dynobj[io].x,
				 m_config->dynobj[io].y,
				 m_config->dynobj[io].r,
				 m_config->dynobj[io].v)){
      cerr << "m_flow->SetDynamicObject() failed in " << __FUNCTION__ << ".\n"
	   << "  id: " << m_config->dynobj[io].id
	   << "  x: " << m_config->dynobj[io].x
	   << "  y: " << m_config->dynobj[io].y
	   << "  r: " << m_config->dynobj[io].r
	   << "  v: " << m_config->dynobj[io].v << "\n";
      exit(EXIT_FAILURE);
    }
  if(!m_flow->SetRobot(m_config->robot_x, m_config->robot_y,
		       m_config->robot_r, m_config->robot_v)){
    cerr << "m_flow->SetRobot() failed in " << __FUNCTION__ << ".\n"
	 << "  x: " << m_config->robot_x
	 << "  y: " << m_config->robot_y
	 << "  r: " << m_config->robot_r
	 << "  v: " << m_config->robot_v << "\n";
    exit(EXIT_FAILURE);
  }
  if(!m_flow->SetGoal(m_config->goal_x, m_config->goal_y, m_config->goal_r)){
    cerr << "m_flow->SetGoal() failed in " << __FUNCTION__ << ".\n"
	 << "  x: " << m_config->goal_x
	 << "  y: " << m_config->goal_y
	 << "  r: " << m_config->goal_r << "\n";
    exit(EXIT_FAILURE);
  }
}


void parse_config(istream & config_is, ostream & dbg)
{
  bool have_goal(false);
  bool have_robot(false);
  bool have_grid(false);
  
  string textline;
  while(getline(config_is, textline)){
    istringstream tls(textline);
    string token;
    tls >> token;
    if(( ! tls) || ('#' == token[0]))
      continue;
    
    if(token == "goal"){
      tls >> m_config->goal_x >> m_config->goal_y >> m_config->goal_r;
      if( ! tls){
	dbg << "Couldn't parse goal from \"" << tls.str() << "\"\n";
	exit(EXIT_FAILURE);
      }
      have_goal = true;
    }
    
    else if(token == "robot"){
      tls >> m_config->robot_x >> m_config->robot_y
	  >> m_config->robot_r >> m_config->robot_v;
      if( ! tls){
	dbg << "Couldn't parse robot from \"" << tls.str() << "\"\n";
	exit(EXIT_FAILURE);
      }
      have_robot = true;
    }
    
    else if(token == "grid"){
      tls >> m_config->grid_x >> m_config->grid_y >> m_config->grid_d;
      if( ! tls){
	dbg << "Couldn't parse grid from \"" << tls.str() << "\"\n";
	exit(EXIT_FAILURE);
      }
      have_grid = true;
    }
    
    else if(token == "mgrid"){
      double mx, my, md;
      tls >> mx >> my >> md;
      if( ! tls){
	dbg << "Couldn't parse mgrid from \"" << tls.str() << "\"\n";
	exit(EXIT_FAILURE);
      }
      m_config->grid_x = static_cast<size_t>(ceil(mx / md));
      m_config->grid_y = static_cast<size_t>(ceil(my / md));
      m_config->grid_d = md;
      have_grid = true;
    }
    
    else if(token == "wall"){
      Config::wall_t wall;
      tls >> wall.x0 >> wall.y0 >> wall.x1 >> wall.y1;
      if( ! tls){
	dbg << "Couldn't parse wall from \"" << tls.str() << "\"\n";
	exit(EXIT_FAILURE);
      }
      m_config->statobj.push_back(wall);
    }
    
    else if(token == "riskmap"){
      string name;
      double cutoff, degree;
      tls >> name >> cutoff >> degree;
      if( ! tls){
	dbg << "Couldn't parse riskmap from \"" << tls.str() << "\"\n";
	exit(EXIT_FAILURE);
      }
      if(m_risk_map)
	dbg << "Overwriting existing riskmap with \"" << tls.str() << "\"\n";
      m_risk_map.reset(PNFRiskMap::Create(name, cutoff, degree));
      if( ! m_risk_map){
	dbg << "Oops, PNFRiskMap::Create() returned null!\n";
	exit(EXIT_FAILURE);
      }
    }
    
    else if(token == "dummyrisk"){
      double risk, meta;
      tls >> risk >> meta;
      if( ! tls){
	dbg << "Couldn't parse dummyrisk from \"" << tls.str() << "\"\n";
	exit(EXIT_FAILURE);
      }
      if(m_risk_map)
	dbg << "Overwriting existing riskmap with \"" << tls.str() << "\"\n";
      m_risk_map.reset(new DummyRiskMap(risk, meta));
    }
    
    else if(token == "invertrisk"){
      if(m_risk_map)
	dbg << "Overwriting existing riskmap with \"" << tls.str() << "\"\n";
      m_risk_map.reset(new InvertRiskMap());
    }
    
    else if(token == "object"){
      Config::object_t obj;
      tls >> obj.id >> obj.x >> obj.y >> obj.r >> obj.v;
      if( ! tls){
	dbg << "Couldn't parse object from \"" << tls.str() << "\"\n";
	exit(EXIT_FAILURE);
      }
      m_config->dynobj.push_back(obj);
    }
    
    else if(token == "robot_buffer_factor"){
      tls >> m_config->robot_buffer_factor;
      if( ! tls){
	dbg << "Couldn't parse robot_buffer_factor from \""
	    << tls.str() << "\"\n";
	exit(EXIT_FAILURE);
      }
    }
    
    else if(token == "robot_buffer_degree"){
      tls >> m_config->robot_buffer_degree;
      if( ! tls){
	dbg << "Couldn't parse robot_buffer_degree from \""
	    << tls.str() << "\"\n";
	exit(EXIT_FAILURE);
      }
    }
    
    else if(token == "object_buffer_factor"){
      tls >> m_config->object_buffer_factor;
      if( ! tls){
	dbg << "Couldn't parse object_buffer_factor from \""
	    << tls.str() << "\"\n";
	exit(EXIT_FAILURE);
      }
    }
    
    else if(token == "object_buffer_degree"){
      tls >> m_config->object_buffer_degree;
      if( ! tls){
	dbg << "Couldn't parse object_buffer_degree from \""
	    << tls.str() << "\"\n";
	exit(EXIT_FAILURE);
      }
    }
    
    else{
      dbg << "DEBUG unhandled line \"" << tls.str() << "\"\n";
    }
  }
  
  if( ! have_goal){
    dbg << "No goal specified.\n";
    exit(EXIT_FAILURE);
  }
  if( ! have_robot){
    dbg << "No robot specified.\n";
    exit(EXIT_FAILURE);
  }
  if( ! have_grid){
    dbg << "No grid specified.\n";
    exit(EXIT_FAILURE);
  }
  if( ! m_risk_map){
    dbg << "No riskmap specified.\n";
    exit(EXIT_FAILURE);
  }
  if(m_config->statobj.empty() && m_config->dynobj.empty()){
    dbg << "Neither walls nor objects specified.\n";
    exit(EXIT_FAILURE);
  }
  
#ifdef UNDEFINED
  printf("Parsed configuration:\n"
	 "  grid:   %zd   %zd   %f\n"
	 "  robot:  %f   %f   %f   %f\n"
	 "  goal:   %f   %f   %f\n"
	 "  statobj:   %zd\n",
	 m_config->grid_x, m_config->grid_y, m_config->grid_d,
	 m_config->robot_x, m_config->robot_y,
	 m_config->robot_r, m_config->robot_v,
	 m_config->goal_x, m_config->goal_y, m_config->goal_r,
	 m_config->statobj.size());
  for(size_t i(0); i < m_config->statobj.size(); ++i)
    printf("    %f   %f   %f   %f\n",
	   m_config->statobj[i].x0, m_config->statobj[i].y0,
	   m_config->statobj[i].x1, m_config->statobj[i].y1);
  printf("  dynobj:   %zd\n",
	 m_config->statobj.size());
  for(size_t i(0); i < m_config->dynobj.size(); ++i)
    printf("    %zd   %f   %f   %f   %f\n",
	   m_config->dynobj[i].id,
	   m_config->dynobj[i].x, m_config->dynobj[i].y,
	   m_config->dynobj[i].r, m_config->dynobj[i].v);
#endif // UNDEFINED
}


void add_wall(double x0, double y0, double x1, double y1, Flow & flow)
{
  double dx(x1 - x0);
  double dy(y1 - y0);
  double nsteps(ceil(sqrt(square(dx) + square(dy)) / flow.resolution));
  nsteps *= 2;
  dx /= nsteps;
  dy /= nsteps;
  nsteps += 1;
  
  double x(x0);
  double y(y0);
  for(double j(0); j < nsteps; j += 1){
    flow.AddStaticObject(x, y);
    x += dx;
    y += dy;
  }
}


void dump_cooc()
{
  for(size_t io(0); io < m_config->dynobj.size(); ++io){
    const size_t id(m_config->dynobj[io].id);
    const array<double> * cooc;
    double max_cooc;
    tie(cooc, max_cooc) = m_flow->GetCooc(id);
    if(0 == cooc){
      cout << __func__ << "(): cooc " << id << " not in m_flow\n";
      exit(EXIT_FAILURE);
    }
    ostringstream name;
    name << "cooc" << id;
    do_dump(name.str(), *cooc, max_cooc);
  }
}


void dump_lambda()
{
  for(size_t io(0); io < m_config->dynobj.size(); ++io){
    const size_t id(m_config->dynobj[io].id);
    const array<double> * lambda;
    double max_lambda;
    tie(lambda, max_lambda) = m_flow->GetLambda(id);
    if(0 == lambda){
      cout << __func__ << "(): lambda " << id << " not in m_flow\n";
      exit(EXIT_FAILURE);
    }
    ostringstream name;
    name << "lambda" << id;
    do_dump(name.str(), *lambda, max_lambda);
  }
}


void dump_risk()
{
  const array<double> * risk;
  double max_risk;
  tie(risk, max_risk) = m_flow->GetRisk();
  if(0 == risk){
    cout << __func__ << "(): risk  not in m_flow\n";
    return;
  }
  do_dump("risk", *risk, max_risk);
}


void dump_objdist()
{
  for(size_t io(0); io < m_config->dynobj.size(); ++io){
    const size_t id(m_config->dynobj[io].id);
    const Facade * objdist(m_flow->GetObjdist(id));
    if(0 == objdist){
      cout << __func__ << "(): objdist " << id << " not in m_flow\n";
      exit(EXIT_FAILURE);
    }
    ostringstream name;
    name << "objdist" << id;
    do_dump(name.str(), * objdist);
  }
}


void do_dump(const string & name, const Facade & facade)
{
  ostringstream valuefname;
  valuefname << name << "_value.data";
  FILE * valuefile(fopen(valuefname.str().c_str(), "w"));
  if(NULL == valuefile){
    perror((string(__func__) + "(): " + valuefname.str()).c_str());
    return;
  }
  ostringstream metafname;
  metafname << name << "_meta.data";
  FILE * metafile(fopen(metafname.str().c_str(), "w"));
  if(NULL == metafile){
    perror((string(__func__) + "(): " + metafname.str()).c_str());
    fclose(valuefile);
    return;
  }
  dump_raw(facade, valuefile, metafile);
  fclose(valuefile);
  fclose(metafile);
  cout << "splot '" << valuefname.str() << "' w l\n";
  cout << "splot '" << metafname.str() << "' w l\n";
}


void do_dump(const string & name, const array<double> & data, double max_val)
{
  ostringstream fname;
  fname << name << ".data";
  FILE * of(fopen(fname.str().c_str(), "w"));
  if(NULL == of){
    perror((string(__func__) + "(): " + fname.str()).c_str());
    return;
  }
  fprintf(of, "# max_%s: %f\n", name.c_str(), max_val);
  dump_raw(data, 0, 0, m_flow->xsize - 1, m_flow->ysize - 1, of);
  fclose(of);
  cout << "splot '" << fname.str() << "' w l\n";
}


void draw_setup(double wall_r, double wall_g, double wall_b,
		double goal_r, double goal_g, double goal_b,
		double robot_r, double robot_g, double robot_b,
		double object_r, double object_g, double object_b,
		bool plotreg)
{
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glMatrixMode(GL_MODELVIEW);
  glLineWidth(1);
  
  glColor3d(wall_r, wall_g, wall_b);
  glBegin(GL_LINES);
  for(size_t iw(0); iw < m_config->statobj.size(); ++iw){
    glVertex2d(m_config->statobj[iw].x0 / m_config->grid_d + 0.5,
	       m_config->statobj[iw].y0 / m_config->grid_d + 0.5);
    glVertex2d(m_config->statobj[iw].x1 / m_config->grid_d + 0.5,
	       m_config->statobj[iw].y1 / m_config->grid_d + 0.5);
  }
  glEnd();
  
  glColor3d(goal_r, goal_g, goal_b);
  glPushMatrix();
  glTranslated(m_config->goal_x / m_config->grid_d + 0.5,
	       m_config->goal_y / m_config->grid_d + 0.5,
	       0);
  {
    const double rad(m_config->goal_r / m_config->grid_d);
    gluDisk(wrap_glu_quadric_instance(), rad, rad, 36, 1);
  }
  glPopMatrix();
  if(plotreg && m_flow){
    const pnf::Region * goal(m_flow->GetGoal());
    if(goal)
      draw_region(*goal, goal_r, goal_g, goal_b);
  }
  
  glColor3d(robot_r, robot_g, robot_b);
  glPushMatrix();
  glTranslated(m_config->robot_x / m_config->grid_d + 0.5,
	       m_config->robot_y / m_config->grid_d + 0.5,
	       0);
  {
    const double rad(m_config->robot_r / m_config->grid_d);
    gluDisk(wrap_glu_quadric_instance(), rad, rad, 36, 1);
  }
  glPopMatrix();
  if(plotreg && m_flow){
    const pnf::Region * robot(m_flow->GetRobot());
    if(robot)
      draw_region(*robot, robot_r, robot_g, robot_b);
  }
  
  glColor3d(object_r, object_g, object_b);
  for(size_t io(0); io < m_config->dynobj.size(); ++io){
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslated(m_config->dynobj[io].x / m_config->grid_d + 0.5,
		 m_config->dynobj[io].y / m_config->grid_d + 0.5,
		 0);
    const double rad(m_config->dynobj[io].r / m_config->grid_d);
    gluDisk(wrap_glu_quadric_instance(), rad, rad, 36, 1);
    glPopMatrix();
    if(plotreg && m_flow){
      const pnf::Region * region(m_flow->GetRegion(m_config->dynobj[io].id));
      if(region)
	draw_region(*region, object_r, object_g, object_b);
    }
  }
}


void draw_setup(bool inv, bool plotreg)
{
  if(inv)
    draw_setup(1, 1, 1,		// wall
	       0.5, 0.5, 1,	// goal
	       0.5, 1, 0.5,	// robot
	       0, 1, 0,		// object
	       plotreg);
  else
    draw_setup(0, 0, 0,		// wall
	       0.5, 0.5, 1,	// goal
	       0.5, 1, 0.5,	// robot
	       0, 1, 0,		// object
	       plotreg);
}
