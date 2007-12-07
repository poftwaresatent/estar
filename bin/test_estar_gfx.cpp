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


#include "Getopt.hpp"
#include <estar/util.hpp>
#include <estar/RiskMap.hpp>
#include <estar/numeric.hpp>
#include <estar/Algorithm.hpp>
#include <estar/LSMKernel.hpp>
#include <estar/AlphaKernel.hpp>
#include <estar/Grid.hpp>
#include <estar/graphics.hpp>
#include <estar/dump.hpp>
#include <estar/Facade.hpp>
#include <gfx/Viewport.hpp>
#include <gfx/Mousehandler.hpp>
#include <gfx/wrap_glut.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>


#undef USE_DEPTH_BUFFER


using namespace estar;
using namespace gfx;
using namespace boost;
using namespace std;

namespace local {
  
  /** Lazy typing. */
  typedef Subwindow::logical_bbox_t bb_t;

  /** Testing E* for "Navigation in Human Presence". Work done in the
      context of Cogniron and Akin's thesis at LAAS-CNRS. */
  class NHPRiskmap: public RiskMap {
  public:
    const double m_scale;
    
    NHPRiskmap(double maxcost, double maxrisk)
      : m_scale(maxrisk / maxcost) { }
    
    virtual double RiskToMeta(double risk) const
    { return boundval(0.0, 1 - risk, 1.0); }
    
    virtual double MetaToRisk(double meta) const
    { return boundval(0.0, 1 - meta, 1.0); }
    
    double CostToRisk(double cost) const
    { return cost < 0 ? 1 : cost * m_scale; }
    
    double CostToMeta(double cost) const
    { return RiskToMeta(CostToRisk(cost)); }
  };

}

using namespace local;


static void * run_glthread(void * nothing_at_all);
static void parse_options(int argc, char ** argv);
static void parse_grid(istream & is);
static void init_glut(int * argc, char ** argv, int width, int height);
static void reshape(int width, int height);
static void draw();
static void keyboard(unsigned char key, int x, int y);
static void timer(int handle);
static void mouse(int button, int state, int x, int y);
static void motion(int x, int y);
static void cleanup(void);


static const unsigned int timer_delay(100);

static bool text_only(false);
static bool step(true);
static bool continuous(false);
static bool finish(false);
static shared_ptr<Viewport> m_value_view;
static shared_ptr<Viewport> m_risk_view;
static shared_ptr<Grid> m_grid;
static shared_ptr<Algorithm> m_algo;
static shared_ptr<Kernel> m_kernel;
static shared_ptr<Facade> hmmmm_facade;
static shared_ptr<Mousehandler> m_mouse_meta;
static ssize_t m_goal_ix, m_goal_iy;
static map<vertex_t, double> m_goal;
static ssize_t m_robot_ix, m_robot_iy;
static vertex_t m_robot;
static shared_ptr<ostream> result_os;
static shared_ptr<NHPRiskmap> m_riskmap;


int main(int argc, char ** argv)
{
  parse_options(argc, argv);
  set_cleanup(cleanup);
  
  if(text_only){
    cout << "graphics disabled!\n";
    if( ! result_os)
      cout << "hint: enable writing to an output file to see results.\n";
    while(m_algo->HaveWork()){
      m_algo->ComputeOne(*m_kernel, 0.5);
      if(m_algo->GetStep() % 50 == 0){
	cerr << ".";
	if(m_algo->GetStep() % 2500 == 0)
	  cerr << "\n";
      }
    }
    cerr << "\n";
  }
  else{
    double x0, y0, x1, y1;
    get_grid_bbox(*m_grid->GetCSpace(), x0, y0, x1, y1);
    static const bb_t realbbox(x0, y0, x1, y1);
    m_value_view.reset(new Viewport("value", realbbox, bb_t(0, 0, 0.5, 1)));
    m_value_view->SetMousehandler(Viewport::LEFT, m_mouse_meta);
    m_value_view->Enable();
    m_risk_view.reset(new Viewport("risk", realbbox, bb_t(0.5, 0, 1, 1)));
    m_risk_view->SetMousehandler(Viewport::LEFT, m_mouse_meta);
    m_risk_view->Enable();
    run_glthread(0);
  }
}


void init_glut(int * argc, char ** argv,
	       int width, int height)
{
  glutInit(argc, argv);
#ifdef USE_DEPTH_BUFFER
  glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);
  glEnable(GL_DEPTH_TEST);
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
  glClearColor(0.5, 0.5, 0.5, 0);
#ifdef USE_DEPTH_BUFFER
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
  glClear(GL_COLOR_BUFFER_BIT);
#endif
  
  bool have_path(false);
  if(get(m_algo->GetValueMap(), m_robot) <= m_algo->GetLastComputedValue())
    have_path = true;
  
  m_value_view->PushProjection();
  draw_grid_value(*m_grid->GetCSpace(), *m_algo,
		  ColorScheme::Get(BLUE_GREEN_RED), true);
  draw_grid_connect(*m_grid->GetCSpace(), *m_algo, 0.5, 0.5, 0.5, 1);
  draw_grid_queue(*m_grid->GetCSpace(), *m_algo);
  draw_grid_upwind(*m_grid->GetCSpace(), *m_algo, 1, 1, 1, 5);
  if(m_grid->connect != HEX_GRID){
    if (have_path) {
      if ( ! hmmmm_facade)
	hmmmm_facade.reset(new Facade(m_algo, m_grid, m_kernel));
      draw_trace(*hmmmm_facade,
		 m_robot_ix * hmmmm_facade->scale,
		 m_robot_iy * hmmmm_facade->scale,
		 ColorScheme::Get(GREEN_PINK_BLUE),
		 1, 0.5, 0);
    }
    glPointSize(5);
    glBegin(GL_POINTS);
    glColor3d(1, 0.5, 0.5);
    glVertex2d(m_robot_ix + 0.5, m_robot_iy + 0.5);
    glColor3d(0.5, 1.0, 0.5);
    glVertex2d(m_goal_ix + 0.5, m_goal_iy + 0.5);
    glEnd();
  }
  m_value_view->PopProjection();
  
  m_risk_view->PushProjection();
  draw_grid_risk(*m_grid->GetCSpace(), *m_algo, *m_riskmap,
		 ColorScheme::Get(GREEN_PINK_BLUE));
  if(m_grid->connect != HEX_GRID){
    if(have_path) {
      if ( ! hmmmm_facade)
	hmmmm_facade.reset(new Facade(m_algo, m_grid, m_kernel));
      draw_trace(*hmmmm_facade,
		 m_robot_ix * hmmmm_facade->scale,
		 m_robot_iy * hmmmm_facade->scale,
		 ColorScheme::Get(GREY_WITH_SPECIAL),
		 1, 0.5, 0);
    }
    glBegin(GL_POINTS);
    glColor3d(1, 0.5, 0.5);
    glVertex2d(m_robot_ix + 0.5, m_robot_iy + 0.5);
    glColor3d(0.5, 1.0, 0.5);
    glVertex2d(m_goal_ix + 0.5, m_goal_iy + 0.5);
    glEnd();
  }
  m_risk_view->PopProjection();
  
  glFlush();
  glutSwapBuffers();
}


void * run_glthread(void * nothing_at_all)
{
  static char * fake_argv[] = {"ESTAR"};
  static int fake_argc = 1;
  init_glut(& fake_argc, fake_argv, 400, 400);
  glutMainLoop();
  return 0;
}


void cleanup()
{
  if(result_os){
    cout << "cleanup(): writing result\n";
    const value_map_t & value(m_algo->GetValueMap());
    for(ssize_t grid_iy(m_grid->GetYSize() - 1); grid_iy >= 0; --grid_iy){
      for(ssize_t grid_ix(0); grid_ix < m_grid->GetXSize(); ++grid_ix)
	(*result_os) << get(value, m_grid->Index2Vertex(grid_ix, grid_iy))
		     << "\t";
      (*result_os) << "\n";
    }
  }
}


void reshape(int width, int height)
{
  Subwindow::DispatchResize(Subwindow::screen_point_t(width, height));
}


void keyboard(unsigned char key, int x, int y)
{
  switch(key){
  case ' ':
    step = true;
    continuous = false;
    finish = false;
    break;
  case 'c':
    step = false;
    continuous = true;
    finish = false;
    break;
  case 'f':
    step = false;
    continuous = true;
    finish = true;
    break;
  case 'q':
    exit(EXIT_SUCCESS);
    break;
  }
}


void timer(int handle)
{
  if(step || continuous || finish){
    if(step)
      step = false;
    
    if( ! finish){
      if(m_algo->HaveWork()){
	m_algo->ComputeOne(*m_kernel, 0.5);
#ifdef ESTAR_DEBUG
	dump_queue(*m_algo, m_grid.get(), 0, stdout);
	dump_grid(*m_grid, stdout);
#endif // ESTAR_DEBUG
      }
    }
    else{
      while(m_algo->HaveWork()){
	m_algo->ComputeOne(*m_kernel, 0.5);
#ifdef ESTAR_DEBUG
	dump_queue(*m_algo, m_grid.get(), 0, stdout);
	dump_grid(*m_grid, stdout);
#endif // ESTAR_DEBUG
      }
      finish = false;
      continuous = false;
    }
    
    m_value_view->Update();
    m_risk_view->Update();
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
  util::Parser parser;
  string result_fname;
  parser.Add(new util::Callback<bool>(text_only, 't', "text-only",
				      "disable Open GL output"));
  parser.Add(new util::Callback<string>(result_fname, 'o', "outfile",
					"write results to specified file"));
  cout << argv[0] << "\n";
  parser.UsageMessage(cout);
  const int res(parser.Do(argc, argv, cerr));
  if(res < 0){
    cerr << "ERROR in parse_options()\n";
    exit(EXIT_FAILURE);
  }
  if(res >= argc){
    cerr << "ERROR: config file name expected\n";
    exit(EXIT_FAILURE);
  }
  
  ifstream is(argv[res]);
  if( ! is){
    cerr << "ERROR in parse_options(): Couldn't open \""
	 << argv[res] << "\"\n";
    exit(EXIT_FAILURE);
  }
  parse_grid(is);
  
  for(map<vertex_t, double>::const_iterator ig(m_goal.begin());
      ig != m_goal.end(); ++ig)
    m_algo->AddGoal(ig->first, ig->second);
  
  if( ! text_only) {
    shared_ptr<Facade> facade(new Facade(m_algo, m_grid, m_kernel));
    m_mouse_meta.reset(new ObstacleMousehandler(facade, facade));
  }
  
  if(result_fname != ""){
    result_os.reset(new ofstream(result_fname.c_str()));
    if( ! (*result_os)){
      cerr << "ERROR in parse_options(): Couldn't open \""
	   << result_fname << "\"\n";
      exit(EXIT_FAILURE);
    }
  }
}


void parse_grid(istream & is)
{
  vector<vector<double> > cost;
  ssize_t grid_xsize(0);
  double maxcost(0);
  bool hex(false);
  m_goal_ix = 0;
  m_goal_iy = 0;
  m_robot_ix = 0;
  m_robot_iy = 0;
  
  string textline;
  while(getline(is, textline)){
    istringstream tls(textline);
    if(textline[0] == '#'){
      tls.ignore(1, '\n');
      string token;
      if(tls >> token){
	if(token == "goal"){
	  tls >> m_goal_ix >> m_goal_iy;
	  if( ! tls){
	    cerr << "ERROR in parse_grid(): Couldn't parse goal from \""
		 << tls.str() << "\"\n";
	    exit(EXIT_FAILURE);
	  }
	}
	else if(token == "robot"){
	  tls >> m_robot_ix >> m_robot_iy;
	  if( ! tls){
	    cerr << "ERROR in parse_grid(): Couldn't parse robot from \""
		 << tls.str() << "\"\n";
	    exit(EXIT_FAILURE);
	  }
	}
	else if(token == "hexgrid")
	  hex = true;
      }
      continue;
    }
    
    cost.push_back(vector<double>());
    vector<double> & line(*cost.rbegin());
    double cost;
    while(tls >> cost){
      line.push_back(cost);
      if(cost > maxcost)
	maxcost = cost;
    }
    if(static_cast<ssize_t>(line.size()) > grid_xsize)
      grid_xsize = line.size();
  }
  const ssize_t grid_ysize(cost.size());
  
  if(grid_xsize < 1){
    cerr << "ERROR in parse_grid(): grid_xsize == " << grid_xsize << "\n";
    exit(EXIT_FAILURE);
  }
  if(grid_ysize < 1){
    cerr << "ERROR in parse_grid(): grid_ysize == " << grid_ysize << "\n";
    exit(EXIT_FAILURE);
  }
  if((grid_xsize <= m_goal_ix) || (grid_ysize <= m_goal_iy)){
    cerr << "ERROR in parse_grid(): invalid goal (" << m_goal_ix << ","
	 << m_goal_iy << " ) in grid (" << grid_xsize << "," << grid_ysize
	 << " )\n";
    exit(EXIT_FAILURE);
  }
  if((grid_xsize <= m_robot_ix) || (grid_ysize <= m_robot_iy)){
    cerr << "ERROR in parse_grid(): invalid robot (" << m_robot_ix << ","
	 << m_robot_iy << " ) in grid (" << grid_xsize << "," << grid_ysize
	 << " )\n";
    exit(EXIT_FAILURE);
  }
  
  if(hex){
    m_grid.reset(new Grid(grid_xsize, grid_ysize, HEX_GRID));
    m_kernel.reset(new AlphaKernel(1));
  }
  else{
    m_grid.reset(new Grid(grid_xsize, grid_ysize, FOUR_CONNECTED));
    m_kernel.reset(new LSMKernel(m_grid->GetCSpace(), 1));
  }
  m_algo.reset(new Algorithm(m_grid->GetCSpace(), false, false,
			     false, false, false));
  
  static const bool fake_goal_radius(false);
  if( ! fake_goal_radius)
    m_goal.insert(make_pair(m_grid->Index2Vertex(m_goal_ix, m_goal_iy), 0));
  else{
    const ssize_t ix0(0 == m_goal_ix ? 0 : m_goal_ix - 1);
    const ssize_t ix1(grid_xsize - 1 == m_goal_ix
		     ? grid_xsize - 1 : m_goal_ix + 1);
    const ssize_t iy0(0 == m_goal_iy ? 0 : m_goal_iy - 1);
    const ssize_t iy1(grid_ysize - 1 == m_goal_iy
		     ? grid_ysize - 1 : m_goal_iy + 1);
    for(ssize_t ix(ix0); ix <= ix1; ++ix)
      for(ssize_t iy(iy0); iy <= iy1; ++iy){
	double dist(sqrt(square(static_cast<double>(ix) - m_goal_ix)
			 + square(static_cast<double>(iy) - m_goal_iy)));
	m_goal.insert(make_pair(m_grid->Index2Vertex(ix, iy), dist));
      }
  }
  
  m_robot = m_grid->Index2Vertex(m_robot_ix, m_robot_iy);
  
  m_riskmap.reset(new NHPRiskmap(maxcost, 1));
  const double norisk_meta(m_riskmap->RiskToMeta(0));
  for(ssize_t grid_iy(grid_ysize - 1), iline(0);
      iline < static_cast<ssize_t>(cost.size());
      --grid_iy, ++iline){
    for(ssize_t grid_ix(0), icol(0);
	grid_ix < grid_xsize;
	++grid_ix, ++icol){
#ifdef ESTAR_DEBUG
      printf("  (%ld,%ld):", grid_ix, grid_iy);
#endif // ESTAR_DEBUG
      if(icol >= static_cast<ssize_t>(cost[iline].size())){
#ifdef ESTAR_DEBUG
	printf("xoxo:%4.2f", norisk_meta);
#endif // ESTAR_DEBUG
	m_algo->InitMeta(m_grid->Index2Vertex(grid_ix, grid_iy), norisk_meta);
      }
      else{
	const double meta(m_riskmap->CostToMeta(cost[iline][icol]));
#ifdef ESTAR_DEBUG
	printf("%4.2f:%4.2f", cost[iline][icol], meta);
#endif // ESTAR_DEBUG
	m_algo->InitMeta(m_grid->Index2Vertex(grid_ix, grid_iy), meta);
      }
    }
#ifdef ESTAR_DEBUG
    printf("\n");
#endif // ESTAR_DEBUG
  }
  
#ifdef ESTAR_DEBUG
  printf("DONE parsing grid\n");
#endif // ESTAR_DEBUG
}
