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
// #include <estar/util.hpp>
// #include <estar/numeric.hpp>
// #include <estar/Algorithm.hpp>
// #include <estar/LSMKernel.hpp>
// #include <estar/AlphaKernel.hpp>
#include <estar/Grid.hpp>
#include <estar/graphics.hpp>
// #include <estar/dump.hpp>
#include <gfx/Viewport.hpp>
#include <gfx/Mousehandler.hpp>
#include <gfx/wrap_glut.hpp>
// #include <boost/shared_ptr.hpp>
#include <iostream>
// #include <fstream>
// #include <sstream>


using namespace estar;
using namespace gfx;
using namespace boost;
using namespace std;

namespace local {
  
  /** lazy typing */
  typedef Subwindow::logical_bbox_t bb_t;
  
  /** convenient string-based lookup of viewports */
  typedef map<string, shared_ptr<Viewport> > viewport_map_t;
  
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
  
  int handle(glutCreateWindow("test_estar_replan_gfx"));
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
  draw_grid_value(*master, ColorScheme::Get(BLUE_GREEN_RED), true);
  draw_grid_queue(*master);
  ////  draw_grid_upwind(*master, 1, 1, 1, 5);
  viewport["master"]->PopProjection();
  
  viewport["sample"]->PushProjection();
  shared_ptr<FacadeReadInterface const> sample(comparison->GetSample());
  draw_grid_value(*sample, ColorScheme::Get(BLUE_GREEN_RED), true);
  draw_grid_queue(*sample);
  ////  draw_grid_upwind(*sample, 1, 1, 1, 5);
  viewport["sample"]->PopProjection();
  
  viewport["comparison"]->PushProjection();
  draw_grid_meta(*sample, ColorScheme::Get(BLUE_GREEN_RED));
  draw_grid_status(*sample);
  viewport["comparison"]->PopProjection();
  
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


void parse_options(int argc, char ** argv)
{
  util::Parser parser;
  int xsize(20);
  int ysize(20);
  int goalx(0);
  int goaly(0);
  string kernel("lsm");
  double scale(1);
  bool cdiag(false);
  
  //   parser.Add(new util::Callback<string>(result_fname, 'o', "outfile",
  // 					"write results to specified file"));
  //   cout << argv[0] << "\n";
  //   parser.UsageMessage(cout);
  //   const int res(parser.Do(argc, argv, cerr));
  //   if(res < 0){
  //     cerr << "ERROR in parse_options()\n";
  //     exit(EXIT_FAILURE);
  //   }
  
  comparison =
    ComparisonFacade::Create(kernel, xsize, ysize, scale, cdiag, stderr);
  if ( ! comparison)
    exit(EXIT_FAILURE);
  comparison->AddGoal(goalx, goaly, 0);
  
  //   for(map<vertex_t, double>::const_iterator ig(m_goal.begin());
  //       ig != m_goal.end(); ++ig)
  //     m_algo->AddGoal(ig->first, ig->second);
}


void create_viewports()
{
  if ( ! comparison) {
    cerr << "ERROR in create_viewports(): no comparison\n";
    exit(EXIT_FAILURE);
  }
  Grid const & grid(comparison->GetMaster()->GetGrid());
  double x0, y0, x1, y1;
  get_grid_bbox(grid, x0, y0, x1, y1);
  static bb_t const realbb(x0, y0, x1, y1);
  viewport["master"].
    reset(new Viewport("master",     realbb, bb_t(0.0, 0.5, 0.4, 1.0)));
  viewport["sample"].
    reset(new Viewport("sample",     realbb, bb_t(0.0, 0.0, 0.4, 0.5)));
  viewport["comparison"].
    reset(new Viewport("comparison", realbb, bb_t(0.4, 0.0, 1.0, 1.0)));
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
