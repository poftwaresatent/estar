/* 
 * Copyright (C) 2007 Roland Philippsen <roland dot philippsen at gmx net>
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
#include <estar/ComparisonFacade.hpp>
#include <estar/Facade.hpp>
#include <estar/Grid.hpp>
#include <estar/graphics.hpp>
#include <gfx/Viewport.hpp>
#include <gfx/Mousehandler.hpp>
#include <gfx/wrap_glut.hpp>
#include <iostream>
#include <fstream>
#include <sstream>


using namespace estar;
using namespace gfx;
using namespace boost;
using namespace std;

namespace local {
  
  /** lazy typing */
  typedef Subwindow::logical_bbox_t bb_t;
  
  /** convenient string-based lookup of viewports */
  typedef map<string, shared_ptr<Viewport> > viewport_map_t;
  
  class CycleColorScheme: public gfx::ColorScheme {
  public:
    CycleColorScheme(double period, double width)
      : m_cc(new gfx::ColorCycle(gfx::ColorScheme::Get(gfx::INVERTED_GREY),
				 period, width))
    {}
    
    virtual void Set(double value) const {
      if (value >= estar::infinity)
	glColor3d(0.4, 0, 0);
      else
	m_cc->Set(value);
    }
    
  private:
    shared_ptr<gfx::ColorCycle> m_cc;
  };
}

using namespace local;


static void parse_options(int argc, char ** argv);
static void create_viewports();
static void create_mousehandlers();
static void init_glut(int * argc, char ** argv, int width, int height);
static void reshape(int width, int height);
static void draw();
static void keyboard(unsigned char key, int x, int y);
static void timer(int handle);
static void mouse(int button, int state, int x, int y);
static void motion(int x, int y);


static const unsigned int timer_delay(100);

static bool step(true);
static bool continuous(false);
static bool finish(false);
static viewport_map_t viewport;
static shared_ptr<ComparisonFacade> comparison;
static shared_ptr<CycleColorScheme> cycle_color;


int main(int argc, char ** argv)
{
  parse_options(argc, argv);
  create_viewports();
  create_mousehandlers();
  
  init_glut(& argc, argv, 400, 400);
  glutMainLoop();
}


void init_glut(int * argc, char ** argv, int width, int height)
{
  glutInit(argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
  glutInitWindowPosition(0, 0);
  glutInitWindowSize(width, height);
  
  int handle(glutCreateWindow("estar_compare_gfx"));
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
  glClear(GL_COLOR_BUFFER_BIT);
  
  viewport["master"]->PushProjection();
  shared_ptr<FacadeReadInterface const> master(comparison->GetMaster());
  draw_grid_value(*master, cycle_color.get(), false);
  draw_grid_queue(*master);
  ////  draw_grid_upwind(*master, 1, 1, 1, 5);
  viewport["master"]->PopProjection();
  
  viewport["sample"]->PushProjection();
  shared_ptr<FacadeReadInterface const> sample(comparison->GetSample());
  draw_grid_value(*sample, cycle_color.get(), false);
  draw_grid_queue(*sample);
  ////  draw_grid_upwind(*sample, 1, 1, 1, 5);
  viewport["sample"]->PopProjection();
  
  viewport["obstacles"]->PushProjection();
  draw_grid_status(*sample);
  draw_grid_obstacles(*sample, 0.8, 0.8, 0.8, false);
  viewport["obstacles"]->PopProjection();
  
  glFlush();
  glutSwapBuffers();
}


void reshape(int width, int height)
{
  Subwindow::DispatchResize(Subwindow::screen_point_t(width, height));
}


/**
   The following keys are active:
   - SPACE: compute one step
   - c: continuously compute and draw after each step
   - f: finish propagating without drawing
   - g: stop computing and clear all goals
   - q: quit
*/
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
  case 'g':
    comparison->RemoveAllGoals();
    step = false;
    continuous = false;
    finish = false;
    break;
  case 'q':
    exit(EXIT_SUCCESS);
    break;
  default:
    cout << "INFO: no action assigned to key '" << key << "'\n";
  }
}


void timer(int handle)
{
  if (step || continuous || finish) {
    
    shared_ptr<FacadeReadInterface const> sample(comparison->GetSample());
    if ( ! finish)
      comparison->ComputeOne();
    else
      while (sample->HaveWork())
	comparison->ComputeOne();
    
    if ( ! sample->HaveWork() ){
      finish = false;
      continuous = false;
    }
    step = false;
  }
  
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


struct goal_s {
  goal_s(ssize_t _x, ssize_t _y, double _v)
    : x(_x), y(_y), v(_v) {}
  ssize_t x, y;
  double v;
};


void parse_options(int argc, char ** argv)
{
  typedef util::Callback<string> string_cb;
  typedef util::Callback<bool> bool_cb;
  util::Parser parser;
  string cfg_fname("");
  bool help(false);
  parser.Add(new string_cb(cfg_fname,
			   'c', "config", "name of config file"));
  parser.Add(new bool_cb(help,
			 'h', "help", "print help message"));
  for (int ii(0); ii < argc; ++ii)
    cout << argv[ii] << " ";
  cout << "\n";
  const int res(parser.Do(argc, argv, cerr));
  if (res < 0) {
    cerr << "ERROR in parse_options()\n";
    parser.UsageMessage(cerr);
    exit(EXIT_FAILURE);
  }
  
  if (help) {
    parser.UsageMessage(cout);
    exit(EXIT_FAILURE);
  }
  
  typedef vector<goal_s> goals_t;
  goals_t goals;
  int xsize(20);
  int ysize(20);
  double scale(1);
  string master_kernel("lsm");
  string sample_kernel("lsm");
  FacadeOptions master_options;
  master_options.auto_flush = true;
  master_options.auto_reset = true;
  FacadeOptions sample_options;
  if (cfg_fname.empty())
    goals.push_back(goal_s(0, 0, 0));
  
  else {
    // parse the config file
    
    ifstream is(cfg_fname.c_str());
    if ( ! is) {
      cerr << "ERROR: could not open config file \""
	   << cfg_fname << "\"\n";
      exit(EXIT_FAILURE);
    }
    
    string textline;
    while (getline(is, textline)) {
      istringstream tls(textline);
      if (textline[0] == '#')
	continue;
      
      string token;
      if ( ! (tls >> token))
	continue;
      
      if (token == "goal") {
	int xx, yy;
	double vv;
	tls >> xx >> yy >> vv;
	if ( ! tls) {
	  cerr << "ERROR: could not parse goal from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
	if ((xx < 0) || (yy < 0) || (vv < 0)) {
	  cerr << "ERROR: negative X, Y, or value in goal \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
	goals.push_back(goal_s(xx, yy, vv));
      }
      else if (token == "size") {
	tls >> xsize >> ysize;
	if ( ! tls) {
	  cerr << "ERROR: could not parse size from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
      }
      else if (token == "scale") {
	tls >> scale;
	if ( ! tls) {
	  cerr << "ERROR: could not parse scale from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
      }
      
      else if (token == "master_kernel") {
	tls >> master_kernel;
	if (( ! tls) || master_kernel.empty()) {
	  cerr << "ERROR: could not parse master_kernel from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
      }
      else if (token == "master_connect_diagonal") {
	string foo;
	tls >> foo;
	if (( ! tls) || foo.empty()) {
	  cerr << "ERROR: could not parse master_connect_diagonal from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
	if (foo == "true")
	  master_options.connect_diagonal = true;
	else if (foo == "false")
	  master_options.connect_diagonal = false;
	else {
	  cerr << "ERROR: invalid master_connect_diagonal \""
	       << tls.str() << "\" should be true or false\n";
	  exit(EXIT_FAILURE);
	}
      }
      else if (token == "master_check_upwind") {
	string foo;
	tls >> foo;
	if (( ! tls) || foo.empty()) {
	  cerr << "ERROR: could not parse master_check_upwind from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
	if (foo == "true")
	  master_options.check_upwind = true;
	else if (foo == "false")
	  master_options.check_upwind = false;
	else {
	  cerr << "ERROR: invalid master_check_upwind \""
	       << tls.str() << "\" should be true or false\n";
	  exit(EXIT_FAILURE);
	}
      }
      else if (token == "master_check_local_consistency") {
	string foo;
	tls >> foo;
	if (( ! tls) || foo.empty()) {
	  cerr << "ERROR: could not parse master_check_local_consistency from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
	if (foo == "true")
	  master_options.check_local_consistency = true;
	else if (foo == "false")
	  master_options.check_local_consistency = false;
	else {
	  cerr << "ERROR: invalid master_check_local_consistency \""
	       << tls.str() << "\" should be true or false\n";
	  exit(EXIT_FAILURE);
	}
      }
      else if (token == "master_check_queue_key") {
	string foo;
	tls >> foo;
	if (( ! tls) || foo.empty()) {
	  cerr << "ERROR: could not parse master_check_queue_key from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
	if (foo == "true")
	  master_options.check_queue_key = true;
	else if (foo == "false")
	  master_options.check_queue_key = false;
	else {
	  cerr << "ERROR: invalid master_check_queue_key \""
	       << tls.str() << "\" should be true or false\n";
	  exit(EXIT_FAILURE);
	}
      }
      else if (token == "master_auto_flush") {
	string foo;
	tls >> foo;
	if (( ! tls) || foo.empty()) {
	  cerr << "ERROR: could not parse master_auto_flush from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
	if (foo == "true")
	  master_options.auto_flush = true;
	else if (foo == "false")
	  master_options.auto_flush = false;
	else {
	  cerr << "ERROR: invalid master_auto_flush \""
	       << tls.str() << "\" should be true or false\n";
	  exit(EXIT_FAILURE);
	}
      }
      else if (token == "master_auto_reset") {
	string foo;
	tls >> foo;
	if (( ! tls) || foo.empty()) {
	  cerr << "ERROR: could not parse master_auto_reset from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
	if (foo == "true")
	  master_options.auto_reset = true;
	else if (foo == "false")
	  master_options.auto_reset = false;
	else {
	  cerr << "ERROR: invalid master_auto_reset \""
	       << tls.str() << "\" should be true or false\n";
	  exit(EXIT_FAILURE);
	}
      }
      
      else if (token == "sample_kernel") {
	tls >> sample_kernel;
	if (( ! tls) || sample_kernel.empty()) {
	  cerr << "ERROR: could not parse sample_kernel from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
      }
      else if (token == "sample_connect_diagonal") {
	string foo;
	tls >> foo;
	if (( ! tls) || foo.empty()) {
	  cerr << "ERROR: could not parse sample_connect_diagonal from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
	if (foo == "true")
	  sample_options.connect_diagonal = true;
	else if (foo == "false")
	  sample_options.connect_diagonal = false;
	else {
	  cerr << "ERROR: invalid sample_connect_diagonal \""
	       << tls.str() << "\" should be true or false\n";
	  exit(EXIT_FAILURE);
	}
      }
      else if (token == "sample_check_upwind") {
	string foo;
	tls >> foo;
	if (( ! tls) || foo.empty()) {
	  cerr << "ERROR: could not parse sample_check_upwind from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
	if (foo == "true")
	  sample_options.check_upwind = true;
	else if (foo == "false")
	  sample_options.check_upwind = false;
	else {
	  cerr << "ERROR: invalid sample_check_upwind \""
	       << tls.str() << "\" should be true or false\n";
	  exit(EXIT_FAILURE);
	}
      }
      else if (token == "sample_check_local_consistency") {
	string foo;
	tls >> foo;
	if (( ! tls) || foo.empty()) {
	  cerr << "ERROR: could not parse sample_check_local_consistency from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
	if (foo == "true")
	  sample_options.check_local_consistency = true;
	else if (foo == "false")
	  sample_options.check_local_consistency = false;
	else {
	  cerr << "ERROR: invalid sample_check_local_consistency \""
	       << tls.str() << "\" should be true or false\n";
	  exit(EXIT_FAILURE);
	}
      }
      else if (token == "sample_check_queue_key") {
	string foo;
	tls >> foo;
	if (( ! tls) || foo.empty()) {
	  cerr << "ERROR: could not parse sample_check_queue_key from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
	if (foo == "true")
	  sample_options.check_queue_key = true;
	else if (foo == "false")
	  sample_options.check_queue_key = false;
	else {
	  cerr << "ERROR: invalid sample_check_queue_key \""
	       << tls.str() << "\" should be true or false\n";
	  exit(EXIT_FAILURE);
	}
      }
      else if (token == "sample_auto_flush") {
	string foo;
	tls >> foo;
	if (( ! tls) || foo.empty()) {
	  cerr << "ERROR: could not parse sample_auto_flush from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
	if (foo == "true")
	  sample_options.auto_flush = true;
	else if (foo == "false")
	  sample_options.auto_flush = false;
	else {
	  cerr << "ERROR: invalid sample_auto_flush \""
	       << tls.str() << "\" should be true or false\n";
	  exit(EXIT_FAILURE);
	}
      }
      else if (token == "sample_auto_reset") {
	string foo;
	tls >> foo;
	if (( ! tls) || foo.empty()) {
	  cerr << "ERROR: could not parse sample_auto_reset from \""
	       << tls.str() << "\"\n";
	  exit(EXIT_FAILURE);
	}
	if (foo == "true")
	  sample_options.auto_reset = true;
	else if (foo == "false")
	  sample_options.auto_reset = false;
	else {
	  cerr << "ERROR: invalid sample_auto_reset \""
	       << tls.str() << "\" should be true or false\n";
	  exit(EXIT_FAILURE);
	}
      }
      
      else {
	cerr << "ERROR: unknown token \"" << token << "\"\n";
	exit(EXIT_FAILURE);
      }
    }
  }
  
  comparison = ComparisonFacade::Create(master_kernel, master_options,
					sample_kernel, sample_options,
					xsize, ysize, scale, stderr);
  if ( ! comparison)
    exit(EXIT_FAILURE);
  
  for(goals_t::const_iterator ig(goals.begin());
      ig != goals.end(); ++ig)
    comparison->AddGoal(ig->x, ig->y, ig->v);
  
  cycle_color.reset(new CycleColorScheme(10 * scale, 1.5 * scale));
}


void create_viewports()
{
  if ( ! comparison) {
    cerr << "ERROR in create_viewports(): no comparison\n";
    exit(EXIT_FAILURE);
  }
  double x0, y0, x1, y1;
  get_grid_bbox(*comparison->GetMaster()->GetCSpace(), x0, y0, x1, y1);
  static bb_t const realbb(x0, y0, x1, y1);
  viewport["master"].
    reset(new Viewport("master",     realbb, bb_t(0.0, 0.5, 0.5, 1.0)));
  viewport["sample"].
    reset(new Viewport("sample",     realbb, bb_t(0.0, 0.0, 0.5, 0.5)));
  viewport["obstacles"].
    reset(new Viewport("obstacles",  realbb, bb_t(0.5, 0.5, 1.0, 1.0)));
  viewport["delta"].
    reset(new Viewport("delta",      realbb, bb_t(0.5, 0.0, 1.0, 0.5)));
  for (viewport_map_t::iterator iv(viewport.begin());
       iv != viewport.end(); ++iv)
    iv->second->Enable();
}


void create_mousehandlers()
{
  if ( ! comparison) {
    cerr << "ERROR in create_viewports(): no comparison\n";
    exit(EXIT_FAILURE);
  }
  shared_ptr<Mousehandler>
    obst(new ObstacleMousehandler(comparison->GetMaster(), comparison));
  shared_ptr<Mousehandler>
    goal(new GoalMousehandler(comparison->GetMaster(), comparison));
  for (viewport_map_t::iterator iv(viewport.begin());
       iv != viewport.end(); ++iv) {
    iv->second->SetMousehandler(Viewport::LEFT, obst);
    iv->second->SetMousehandler(Viewport::RIGHT, goal);
  }
}
