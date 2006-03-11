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


#include "graphics.hpp"
#include <gfx/wrap_gl.hpp>
#include <estar/RiskMap.hpp>
#include <estar/Grid.hpp>
#include <estar/numeric.hpp>
#include <estar/Algorithm.hpp>
#include <estar/Kernel.hpp>
#include <estar/Facade.hpp>
#include <estar/Upwind.hpp>


#ifdef DEBUG
# define ESTAR_GRAPHICS_DEBUG
#else
# undef ESTAR_GRAPHICS_DEBUG
#endif

#ifdef ESTAR_GRAPHICS_DEBUG
# define PDEBUG PDEBUG_OUT
#else
# define PDEBUG PDEBUG_OFF
#endif

#define PVDEBUG PDEBUG_OFF


using namespace estar;
using boost::tie;
using std::pair;
using std::make_pair;


namespace local {
  
  /** Utility for tracing the gradient from robot to goal. */
  struct trace_s {
    trace_s(size_t _ix, size_t _iy, double _val,
	    bool _ok, bool _heur, double _x, double _y)
      : ix(_ix), iy(_iy), val(_val), ok(_ok), heur(_heur), x(_x), y(_y) { }
    size_t ix, iy;
    double val;
    bool ok, heur;
    double x, y;
  };
  
  /** Utility for translating a value into a color. */
  typedef void (*csfunc_t)(double);
  
  /** Utility for translating an index into coordinates. */
  typedef pair<double, double> (*pfunc_t)(double ix, double iy);
  
}

using namespace local;


namespace gfx {
  
  
  static const double hex_yscale(0.866025403784); // sqrt(3)/2
  
  
  static void cs_green_pink_blue(double value)
  {
    glColor3d(1 - 2 * absval(0.5 - value), 1 - value, value);
  }  
  
  static void cs_blue_green_red(double value)
  {
    glColor3d(value, 1 - 2 * absval(0.5 - value), 1 - value);
  }
  
  static void cs_grey_with_special(double value)
  {
    if(value < 0)
      glColor3d(0, 0, 1);
    else if(value < epsilon)
      glColor3d(0, 1, 0);
    else if(value == infinity)
      glColor3d(1, 0, 0);
    else
      glColor3d(value, value, value);
  }
  
  static void cs_inverted_grey(double value)
  {
    value = 1 - value;
    if(value < 0)
      glColor3d(0, 0, 0);
    else if(value > 1)
      glColor3d(1, 1, 1);
    else
      glColor3d(value, value, value);
  }
  
  static void cs_red(double value)
  {
    glColor3d(1, 0, 0);
  }
  
  static csfunc_t csfunc_map[] = {
    cs_green_pink_blue,
    cs_grey_with_special,
    cs_blue_green_red,
    cs_inverted_grey,
    cs_red
  };
  
  
  static pair<double, double> p_cartesian(double ix, double iy)
  {
    return make_pair(ix + 0.5, iy + 0.5);
  }
  
  static pair<double, double> p_hexgrid(double ix, double iy)
  {
    if(absval(fmod(iy, 2) - 1) > 0.5)
      return make_pair(ix + 0.5, hex_yscale * iy + 0.5);
    return make_pair(ix + 1.0, hex_yscale * iy + 0.5);
  }
  
  static pfunc_t get_pfunc(const Grid & grid)
  {
    switch(grid.connect){
    case FOUR_CONNECTED:
    case EIGHT_CONNECTED:
      return p_cartesian;
    case HEX_GRID:
      return p_hexgrid;
    default:
      PDEBUG_ERR("ERROR: Invalid grid.connect %d\n", grid.connect);
      exit(EXIT_FAILURE);
    }
    return 0;
  }
  
  
  void draw_grid_value(const Grid & grid, const Algorithm & algo,
		       colorscheme_t colorscheme)
  {
    csfunc_t csfunc(csfunc_map[colorscheme]);
    pfunc_t pfunc(get_pfunc(grid));
    
    const double delta(algo.GetLastComputedValue());
    if(0 >= delta){
      PDEBUG("0 >= delta == %g\n", delta);
      return;
    }
    
    const size_t xsize(grid.GetXSize());
    const size_t ysize(grid.GetYSize());
    const value_map_t & value(algo.GetValueMap());
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for(size_t ix(0); ix < xsize; ++ix)
      for(size_t iy(0); iy < ysize; ++iy){
	const double value(minval(get(value, grid.GetVertex(ix, iy)), delta));
	csfunc(value / delta);
	double xc, yc;
	tie(xc, yc) = pfunc(ix, iy);
	glRectd(xc - 0.5, yc - 0.5, xc + 0.5, yc + 0.5);
      }
  }
  
  
  void draw_grid_risk(const Grid & grid,
 		      const Algorithm & algo,
 		      const RiskMap & riskmap,
		      colorscheme_t colorscheme)
  {
    csfunc_t csfunc(csfunc_map[colorscheme]);
    pfunc_t pfunc(get_pfunc(grid));
    
    const size_t xsize(grid.GetXSize());
    const size_t ysize(grid.GetYSize());
    const meta_map_t & meta(algo.GetMetaMap());
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for(size_t ix(0); ix < xsize; ++ix)
      for(size_t iy(0); iy < ysize; ++iy){
	csfunc(riskmap.MetaToRisk(get(meta, grid.GetVertex(ix, iy))));
 	double xc, yc;
	tie(xc, yc) = pfunc(ix, iy);
	glRectd(xc - 0.5, yc - 0.5, xc + 0.5, yc + 0.5);
      }
  }
  
  
  void draw_grid_meta(const Grid & grid,
 		      const Algorithm & algo,
		      const Kernel & kernel,
		      colorscheme_t colorscheme)
  {
    csfunc_t csfunc(csfunc_map[colorscheme]);
    pfunc_t pfunc(get_pfunc(grid));
    
    const size_t xsize(grid.GetXSize());
    const size_t ysize(grid.GetYSize());
    const meta_map_t & meta(algo.GetMetaMap());
    const double min_meta(minval(kernel.freespace_meta, kernel.obstacle_meta));
    const double max_meta(maxval(kernel.freespace_meta, kernel.obstacle_meta));
    const double delta(max_meta - min_meta);
    
    if(delta < epsilon)
      return;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for(size_t ix(0); ix < xsize; ++ix)
      for(size_t iy(0); iy < ysize; ++iy){
	csfunc(get(meta, grid.GetVertex(ix, iy)) / delta - min_meta);
 	double xc, yc;
	tie(xc, yc) = pfunc(ix, iy);
	glRectd(xc - 0.5, yc - 0.5, xc + 0.5, yc + 0.5);
      }
  }
  
  
  void draw_trace(const Facade & facade,
		  double robot_x, double robot_y, double goalradius,
		  colorscheme_t colorscheme,
		  double fail_r, double fail_g, double fail_b)
  {
    if((robot_x < 0) || (robot_y < 0))
      return;
    const size_t ix(static_cast<size_t>(rint(robot_x / facade.scale)));
    const size_t iy(static_cast<size_t>(rint(robot_y / facade.scale)));
    if((ix >= facade.xsize) || (iy >= facade.ysize))
      return;
    draw_trace(facade.GetGrid(), facade.GetAlgorithm(),
	       ix, iy,
	       facade.scale / 2, goalradius, colorscheme,
	       fail_r, fail_g, fail_b);
  }
  
  
  /** \todo Implement tracing for hexgrids. Only recompute gradient if
      robot index changed. */
  void draw_trace(const Grid & grid,
		  const Algorithm & algo,
		  size_t robot_ix, size_t robot_iy,
		  double stepsize, double goalradius,
		  colorscheme_t colorscheme,
		  double fail_r, double fail_g, double fail_b)
  {
    if((grid.connect != FOUR_CONNECTED)
       && (grid.connect != EIGHT_CONNECTED)){
      PDEBUG_OUT("TODO: Implement tracing for hexgrids!\n");
      return;
    }
    
    csfunc_t csfunc(csfunc_map[colorscheme]);
    
    const value_map_t & value(algo.GetValueMap());
    const size_t max_ix(grid.GetXSize() - 1);
    const size_t max_iy(grid.GetYSize() - 1);
    const double startval(get(value, grid.GetVertex(robot_ix, robot_iy)));
    
    PDEBUG("\n"
	   "  robot: (%lu, %lu)\n"
	   "  initial val: %2e\n"
	   "  stepsize: %f\n"
	   "  goalradius: %f\n",
	   robot_ix, robot_iy, startval, stepsize,
	   goalradius);
    
    typedef std::vector<struct trace_s> trace_t;
    trace_t trace;
    
    {
      size_t rix(robot_ix);
      size_t riy(robot_iy);
      double rx(rix);
      double ry(riy);
      double val(startval);
      for(int i(0); (i < 100000) && (val > goalradius); ++i){
	double gx, gy;
	const bool ok(grid.ComputeGradient(rix, riy, gx, gy));
	bool heur(false);
	if(ok){
	  const double scale(stepsize / (sqrt(square(gx) + square(gy))));
	  if(scale < epsilon)
	    heur = true;
	  else{
	    rx -= gx * scale;
	    ry -= gy * scale;
	  }
	}
	if(heur || ( ! ok)){
	  if(gx > 0)      rx -= stepsize / 2;
	  else if(gx < 0) rx += stepsize / 2;
	  if(gy > 0)      ry -= stepsize / 2;
	  else if(gy < 0) ry += stepsize / 2;
	}
	rix = boundval<size_t>(0, static_cast<size_t>(rint(rx)), max_ix);
	riy = boundval<size_t>(0, static_cast<size_t>(rint(ry)), max_iy);
	val = get(value, grid.GetVertex(rix, riy));
	trace.push_back(trace_s(rix, riy, val, ok, heur, rx, ry));
	
	PVDEBUG(" -(%5.2f, %5.2f) ==> (%5.2f, %5.2f) [%lu, %lu] @ %2e\n",
		gx, gy, rx, ry, rix, riy, val);
      }
    }
    
    {
      pfunc_t pfunc(get_pfunc(grid));
      glBegin(GL_POINTS);
      csfunc(0);
      glPointSize(1);
      double rx, ry;
      tie(rx, ry) = pfunc(robot_ix, robot_iy);
      glVertex2d(rx, ry);
      for(trace_t::const_iterator it(trace.begin()); it != trace.end(); ++it){
	if(it->ok && ( ! it->heur))
	  csfunc(it->val / startval);
	else
	  glColor3d(fail_r, fail_g, fail_b);
	tie(rx, ry) = pfunc(it->x, it->y);
	glVertex2d(rx, ry);
      }
      glEnd();
    }
  }
  
  
  void draw_grid_meta(const Facade & facade,
		      colorscheme_t colorscheme)
  {
    draw_grid_meta(facade.GetGrid(), facade.GetAlgorithm(), facade.GetKernel(),
		   colorscheme);
  }
  
  
  void draw_grid_value(const Facade & facade,
		       colorscheme_t colorscheme)
  {
    draw_grid_value(facade.GetGrid(), facade.GetAlgorithm(), colorscheme);
  }
  

  void draw_array(const array<double> & grid,
		  size_t x0, size_t y0, size_t x1, size_t y1,
		  double lower, double upper,
		  colorscheme_t colorscheme)
  {
    csfunc_t csfunc(csfunc_map[colorscheme]);
    
    const double delta(upper - lower);
    if(0 >= delta)
      return;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for(size_t ix(x0); ix <= x1; ++ix)
      for(size_t iy(y0); iy <= y1; ++iy){
	csfunc(boundval(0.0, (grid[ix][iy] - lower) / delta, 1.0));
	glRectd(ix, iy, ix + 1, iy + 1);
      }
  }


  void draw_grid_queue(const Grid & grid,
		       const Algorithm & algo)
  {
    const flag_map_t & flag(algo.GetFlagMap());
    const value_map_t & value(algo.GetValueMap());
    const rhs_map_t & rhs(algo.GetRhsMap());
    const queue_t & queue(algo.GetQueue().Get());
    pfunc_t pfunc(get_pfunc(grid));
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1);
    for(const_queue_it iq(queue.begin()); iq != queue.end(); ++iq){
      const vertex_t vertex(iq->second);
      double delta(0);
      switch(get(flag, vertex)){
      case OPEN:
	if(get(rhs, vertex) > get(value, vertex))
	  glColor3d(0, 1, 1);
	else{
	  delta = 0.1;
	  glColor3d(1, 0, 1);
	}
	break;
      case OPNG:
	if(get(rhs, vertex) > get(value, vertex))
	  glColor3d(0.5, 1, 1);
	else{
	  delta = 0.1;
	  glColor3d(1, 0.5, 1);
	}
	break;
      default:
	PDEBUG("i: %d   f: %s\n", vertex, flag_name(get(flag, vertex)));
	continue;
      }
      const GridNode & gn(grid.Vertex2Node(vertex));
      double xc, yc;
      tie(xc, yc) = pfunc(gn.ix, gn.iy);
      glRectd(xc - 0.5 + delta, yc - 0.5 + delta,
	      xc + 0.5 - delta, yc + 0.5 - delta);
    }
  }
  
  
  void draw_grid_upwind(const Grid & grid,
			const Algorithm & algo,
			double red, double green, double blue,
			double linewidth)
  {
    pfunc_t pfunc(get_pfunc(grid));
    const Upwind::map_t & uwm(algo.GetUpwind().GetMap());
    glBegin(GL_LINES);
    glColor3d(red, green, blue);
    glLineWidth(linewidth);
    for(Upwind::map_t::const_iterator iu(uwm.begin()); iu != uwm.end(); ++iu){
      const GridNode & from(grid.Vertex2Node(iu->first));
      const Upwind::set_t & ts(iu->second);
      double xf, yf;
      tie(xf, yf) = pfunc(from.ix, from.iy);
      for(Upwind::set_t::const_iterator it(ts.begin()); it != ts.end(); ++it){
	const GridNode & to(grid.Vertex2Node(*it));
	double xt, yt;
	tie(xt, yt) = pfunc(to.ix, to.iy);
	glVertex2d(0.5 * (xf + xt), 0.5 * (yf + yt));
	glVertex2d(xt, yt);
      }
    }
    glEnd();
  }
  
  
  void draw_grid_connect(const Grid & grid,
			 const Algorithm & algo,
			 double red, double green, double blue,
			 double linewidth)
  {
    pfunc_t pfunc(get_pfunc(grid));
    const cspace_t & cspace(algo.GetCSpace());
    glBegin(GL_LINES);
    glColor3d(red, green, blue);
    glLineWidth(linewidth);
    vertex_it iv, vend;
    tie(iv, vend) = vertices(cspace);
    for(/**/; iv != vend; ++iv){
      const GridNode & from(grid.Vertex2Node(*iv));
      double xf, yf;
      tie(xf, yf) = pfunc(from.ix, from.iy);
      boost::graph_traits<cspace_t>::adjacency_iterator ia, aend;
      tie(ia, aend) = adjacent_vertices(*iv, cspace);
      for(/**/; ia != aend; ++ia){
	const GridNode & to(grid.Vertex2Node(*ia));
	double xt, yt;
	tie(xt, yt) = pfunc(to.ix, to.iy);
	glVertex2d(xf, yf);
	glVertex2d(xt, yt);
      }
    }
    glEnd();
  }
  
  
  void get_grid_bbox(const Grid & grid,
		     double & x0, double & y0, double & x1, double & y1)
  {
    switch(grid.connect){
    case FOUR_CONNECTED:
    case EIGHT_CONNECTED:
      x0 = 0;
      y0 = 0;
      x1 = grid.xsize;
      y1 = grid.ysize;
      break;
    case HEX_GRID:
      x0 = 0;
      y0 = 0;
      x1 = grid.xsize + 0.5;
      y1 = hex_yscale * (grid.ysize - 1) + 1;
      break;
    default:
      PDEBUG_ERR("ERROR: Invalid grid.connect %d\n", grid.connect);
      exit(EXIT_FAILURE);
    }
    PDEBUG("%s size (%lu, %lu) bbox (%g, %g, %g, %g)\n",
	   grid.connect == FOUR_CONNECTED
	   ? "FOUR" : (grid.connect == EIGHT_CONNECTED ? "EIGHT" : "HEX"),
	   grid.xsize, grid.ysize, x0, y0, x1, y1);
  }
  
}