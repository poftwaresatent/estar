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
#include "RiskMap.hpp"
#include "GridNode.hpp"
#include "numeric.hpp"
#include "Algorithm.hpp"
#include "Kernel.hpp"
#include "FacadeReadInterface.hpp"
#include "Upwind.hpp"
#include "Region.hpp"
#include "pdebug.hpp"
#include "../gfx/wrap_gl.hpp"
#include <boost/shared_ptr.hpp>


using namespace estar;
using boost::tie;
using boost::shared_ptr;
using std::pair;
using std::make_pair;


namespace gfx {
  
  
  static const double hex_yscale(0.866025403784); // sqrt(3)/2
  
  
  class cs_green_pink_blue: public ColorScheme {
  public:
    void Set(double value) const
    { glColor3d(1 - 2 * absval(0.5 - value), 1 - value, value); }
  };
  
  class cs_blue_green_red: public ColorScheme {
  public:
    void Set(double value) const
    { glColor3d(value, 1 - 2 * absval(0.5 - value), 1 - value); }
  };
  
  class cs_grey_with_special: public ColorScheme {
  public:
    void Set(double value) const {
      if(value < 0)              glColor3d(0, 0, 1);
      else if(value < epsilon)   glColor3d(0, 1, 0);
      else if(value == infinity) glColor3d(1, 0, 0);
      else                       glColor3d(value, value, value); }
  };
  
  class cs_inverted_grey: public ColorScheme {
  public:
    void Set(double value) const {
      value = 1 - value;
      if(value < 0)      glColor3d(0, 0, 0);
      else if(value > 1) glColor3d(1, 1, 1);
      else               glColor3d(value, value, value); }
  };
  
  class cs_red: public ColorScheme {
  public:
    void Set(double value) const { glColor3d(1, 0, 0); }
  };
  
  
  const ColorScheme * ColorScheme::
  Get(colorscheme_enum_t number)
  {
    static shared_ptr<ColorScheme> green_pink_blue(new cs_green_pink_blue);
    static shared_ptr<ColorScheme> grey_with_special(new cs_grey_with_special);
    static shared_ptr<ColorScheme> blue_green_red(new cs_blue_green_red);
    static shared_ptr<ColorScheme> inverted_grey(new cs_inverted_grey);
    static shared_ptr<ColorScheme> red(new cs_red);
    switch(number){
      case GREEN_PINK_BLUE:   return green_pink_blue.get();
      case GREY_WITH_SPECIAL: return grey_with_special.get();
      case BLUE_GREEN_RED:    return blue_green_red.get();
      case INVERTED_GREY:     return inverted_grey.get();
      case RED:               return red.get();
    }
    return 0;
  }
  
  
  ColorCycle::
  ColorCycle(const ColorScheme * scheme,
	     double period,
	     double width)
    : m_scheme(scheme),
      m_period(period),
      m_width(width)
  {
    if (m_period < epsilon)
      m_period = epsilon;
    if (m_width > m_period / 3)
      m_width = m_period / 3;
    m_half_period = m_period / 2;
    m_scaled_width = m_width / m_period;
    m_scale = 1.0 / (1 - 2 * m_scaled_width);
  }
  
  
  void ColorCycle::
  Set(double value) const
  {
    m_scheme->Set(ComputeMapping(value));
  }
  
  
  double ColorCycle::
  ComputeMapping(double value) const
  {
    value = fmod(value, m_period);
    if (value > m_half_period)
      value = m_period - value;
    value *= 2;
    // here, value is rescaled to [0..1] mirrored around period/2,
    // i.e. it's a periodic sawtooth with zeros at N*period and ones
    // at (2N+1)*period/2
    
    value = (value - m_scaled_width) * m_scale;
    if (value <= 0)
      return 0;
    if (value >= 1)
      return 1;
    return value;
  }
  
  
  void draw_grid_value(const GridCSpace & cspace,
		       const Algorithm & algo,
		       const ColorScheme * colorscheme,
 		       bool auto_scale_value)
  {
    if( ! colorscheme)
      return;
    
    if (auto_scale_value) {
      const double delta(algo.GetLastComputedValue());
      if (0 >= delta) {
	PVDEBUG("0 >= delta == %g\n", delta);
	return;
      }
      
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      for (vertex_read_iteration viter(cspace.begin());
	   viter.not_at_end(); ++viter) {
	const double vv(minval(cspace.GetValue(*viter), delta));
	colorscheme->Set(vv / delta);
	double xc, yc;
	tie(xc, yc) = cspace.ComputePosition(*viter);
	glRectd(xc - 0.5, yc - 0.5, xc + 0.5, yc + 0.5);
      }
    }
    else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      for (vertex_read_iteration viter(cspace.begin());
	   viter.not_at_end(); ++viter) {
	colorscheme->Set(cspace.GetValue(*viter));
	double xc, yc;
	tie(xc, yc) = cspace.ComputePosition(*viter);
	glRectd(xc - 0.5, yc - 0.5, xc + 0.5, yc + 0.5);
      }
    }
  }
  
  
  void draw_grid_rhs(const GridCSpace & cspace, const Algorithm & algo,
		     const ColorScheme * colorscheme)
  {
    if( ! colorscheme)
      return;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for (vertex_read_iteration viter(cspace.begin());
	 viter.not_at_end(); ++viter) {
      colorscheme->Set(cspace.GetRhs(*viter));
      double xc, yc;
      tie(xc, yc) = cspace.ComputePosition(*viter);
      glRectd(xc - 0.5, yc - 0.5, xc + 0.5, yc + 0.5);
    }
  }
  
  
  void draw_grid_risk(const GridCSpace & cspace,
 		      const Algorithm & algo,
 		      const RiskMap & riskmap,
		      const ColorScheme * colorscheme)
  {
    if( ! colorscheme)
      return;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for (vertex_read_iteration viter(cspace.begin());
	 viter.not_at_end(); ++viter) {
      colorscheme->Set(riskmap.MetaToRisk(cspace.GetMeta(*viter)));
      double xc, yc;
      tie(xc, yc) = cspace.ComputePosition(*viter);
      glRectd(xc - 0.5, yc - 0.5, xc + 0.5, yc + 0.5);
    }
  }
  
  
  void draw_grid_meta(const GridCSpace & cspace,
 		      const Algorithm & algo,
		      const Kernel & kernel,
		      const ColorScheme * colorscheme)
  {
    if( ! colorscheme)
      return;
    
    double const min_meta(minval(kernel.freespace_meta, kernel.obstacle_meta));
    double const max_meta(maxval(kernel.freespace_meta, kernel.obstacle_meta));
    double const delta(max_meta - min_meta);
    
    if(delta < epsilon)
      return;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for (vertex_read_iteration viter(cspace.begin());
	 viter.not_at_end(); ++viter) {
      colorscheme->Set(cspace.GetMeta(*viter) / delta - min_meta);
      double xc, yc;
      tie(xc, yc) = cspace.ComputePosition(*viter);
      glRectd(xc - 0.5, yc - 0.5, xc + 0.5, yc + 0.5);
    }
  }
  
  
  void draw_trace(const FacadeReadInterface & facade,
		  double robot_x, double robot_y,
		  const ColorScheme * colorscheme,
		  double fail_r, double fail_g, double fail_b)
  {
    if( ! colorscheme)
      return;
    if((robot_x < 0) || (robot_y < 0))
      return;
    double const distance(estar::infinity);
    double const stepsize(0.5 * facade.GetScale());
    ssize_t const maxsteps(1000);
    carrot_trace trace;
    int const status(facade.TraceCarrot(robot_x, robot_y,
					distance, stepsize, maxsteps,
					trace));
    if (status >= 0)
      draw_trace(trace, colorscheme, fail_r, fail_g, fail_b);
  }
  
  
  void draw_trace(carrot_trace const & trace,
		  ColorScheme const * colorscheme,
		  double fail_r, double fail_g, double fail_b)
  {
    if ( ! colorscheme)
      return;
    if (trace.size() < 2)
      return;
    
    carrot_trace::const_iterator icarrot(trace.begin());
    double const startval(icarrot->value);
    
    glBegin(GL_POINTS);
    glPointSize(1);
    for (/**/; icarrot != trace.end(); ++icarrot) {
      colorscheme->Set(icarrot->value / startval);
      if (icarrot->degenerate)
	glColor3d(fail_r, fail_g, fail_b);
      else
	colorscheme->Set(icarrot->value / startval);
      glVertex2d(icarrot->cx, icarrot->cy);
    }
    glEnd();
  }
  
  
  void draw_grid_meta(const FacadeReadInterface & facade,
		      const ColorScheme * colorscheme)
  {
    if( ! colorscheme)
      return;
    draw_grid_meta(*facade.GetCSpace(), facade.GetAlgorithm(),
		   facade.GetKernel(), colorscheme);
  }
  
  
  void draw_grid_obstacles(const FacadeReadInterface & facade,
			   double red, double green, double blue,
			   bool fill_cells)
  {
    GridCSpace const & cspace(*facade.GetCSpace());
    double const obstacle_meta(facade.GetKernel().obstacle_meta);
    
    glColor3d(red, green, blue);
    if (fill_cells)
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    for (vertex_read_iteration viter(cspace.begin());
	 viter.not_at_end(); ++viter) {
      if (obstacle_meta == cspace.GetMeta(*viter)) {
	double xc, yc;
	tie(xc, yc) = cspace.ComputePosition(*viter);
	glRectd(xc - 0.5, yc - 0.5, xc + 0.5, yc + 0.5);
      }
    }
  }
  
  
  void draw_grid_value(const FacadeReadInterface & facade,
		       const ColorScheme * colorscheme,
 		       bool auto_scale_value)
  {
    if( ! colorscheme)
      return;
    draw_grid_value(*facade.GetCSpace(), facade.GetAlgorithm(),
		    colorscheme, auto_scale_value);
  }
  
  
  void draw_grid_rhs(const FacadeReadInterface & facade,
		     const ColorScheme * colorscheme)
  {
    if( ! colorscheme)
      return;
    draw_grid_rhs(*facade.GetCSpace(), facade.GetAlgorithm(), colorscheme);
  }
  
  
  void draw_array(const array<double> & grid,
		  ssize_t x0, ssize_t y0, ssize_t x1, ssize_t y1,
		  double lower, double upper,
		  const ColorScheme * colorscheme)
  {
    if( ! colorscheme)
      return;
    
    const double delta(upper - lower);
    if(0 >= delta)
      return;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for(ssize_t ix(x0); ix <= x1; ++ix)
      for(ssize_t iy(y0); iy <= y1; ++iy){
	colorscheme->Set(boundval(0.0, (grid[ix][iy] - lower) / delta, 1.0));
	glRectd(ix, iy, ix + 1, iy + 1);
      }
  }


  void draw_grid_queue(const GridCSpace & cspace,
		       const Algorithm & algo)
  {
    queue_t const & queue(algo.GetQueue().Get());
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1);
    for(const_queue_it iq(queue.begin()); iq != queue.end(); ++iq){
      vertex_t const vertex(iq->second);
      double const delta(cspace.GetRhs(vertex) - cspace.GetValue(vertex));
      double rim(0);
      switch (cspace.GetFlag(vertex)) {
      case OPEN:
	if (absval(delta) < epsilon) { // same, shouldn't be queued
	  rim = 0.2;
	  glColor3d(1, 1, 0);
	}
	else if(delta > 0)	// rhs > value: "raise event"
	  glColor3d(0, 1, 1);
	else {			// rhs < value: "lower event"
	  rim = 0.1;
	  glColor3d(1, 0, 1);
	}
	break;
      case OPNG:
	if (absval(delta) < epsilon) {
	  rim = 0.2;
	  glColor3d(1, 1, 0.5);
	}
	else if (cspace.GetRhs(vertex) > cspace.GetValue(vertex))
	  glColor3d(0.5, 1, 1);
	else {
	  rim = 0.1;
	  glColor3d(1, 0.5, 1);
	}
	break;
      default:
	PVDEBUG("i: %d   f: %s\n", vertex, flag_name(cspace.GetFlag(vertex)));
	continue;
      }
      double xc, yc;
      tie(xc, yc) = cspace.ComputePosition(vertex);
      glRectd(xc - 0.5 + rim, yc - 0.5 + rim,
	      xc + 0.5 - rim, yc + 0.5 - rim);
    }
  }
  
  
  void draw_grid_queue(const estar::FacadeReadInterface & facade)
  {
    draw_grid_queue(*facade.GetCSpace(), facade.GetAlgorithm());
  }
  
  
  void draw_grid_upwind(const estar::FacadeReadInterface & facade,
			double red, double green, double blue,
			double linewidth)
  {
    draw_grid_upwind(*facade.GetCSpace(), facade.GetAlgorithm(),
		     red, green, blue, linewidth);
  }
  
  
  void draw_grid_upwind(const GridCSpace & cspace,
			const Algorithm & algo,
			double red, double green, double blue,
			double linewidth)
  {
    Upwind::map_t const & uwm(algo.GetUpwind().GetMap());

    glBegin(GL_LINES);
    glColor3d(red, green, blue);
    glLineWidth(linewidth);

    for(Upwind::map_t::const_iterator iu(uwm.begin()); iu != uwm.end(); ++iu){
      double xf, yf;
      tie(xf, yf) = cspace.ComputePosition(iu->first);
      const Upwind::set_t & ts(iu->second);
      for(Upwind::set_t::const_iterator it(ts.begin()); it != ts.end(); ++it){
	double xt, yt;
	tie(xt, yt) = cspace.ComputePosition(*it);
	glVertex2d(0.5 * (xf + xt), 0.5 * (yf + yt));
	glVertex2d(xt, yt);
      }
    }
    glEnd();
  }
  
  
  void draw_grid_connect(const GridCSpace & cspace,
			 const Algorithm & algo,
			 double red, double green, double blue,
			 double linewidth)
  {
    glBegin(GL_LINES);
    glColor3d(red, green, blue);
    glLineWidth(linewidth);
    for (vertex_read_iteration viter(cspace.begin());
	 viter.not_at_end(); ++viter) {
      double xf, yf;
      tie(xf, yf) = cspace.ComputePosition(*viter);
      for (edge_read_iteration eiter(cspace.begin(*viter));
	   eiter.not_at_end(); ++eiter) {
	double xt, yt;
	tie(xt, yt) = cspace.ComputePosition(*eiter);
	glVertex2d(xf, yf);
	glVertex2d(xt, yt);
      }
    }
    glEnd();
  }
  
  
  void get_grid_bbox(const estar::FacadeReadInterface & facade,
		     double & x0, double & y0, double & x1, double & y1)
  {
    get_grid_bbox(*facade.GetCSpace() , x0, y0, x1, y1);
  }
  
  
  void get_grid_bbox(const GridCSpace & cspace,
		     double & x0, double & y0, double & x1, double & y1)
  {
    cspace.ComputeBBox(x0, y0, x1, y1);
  }
  
  
  void draw_region(const estar::Region & region,
		   double red, double green, double blue)
  {
    glColor3d(red, green, blue);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1);
    const Region::indexlist_t & area(region.GetArea());
    for(size_t ia(0); ia < area.size(); ++ia)
      glRectd(area[ia].x, area[ia].y, area[ia].x + 1, area[ia].y + 1);
  }


  void draw_grid_status(const estar::FacadeReadInterface & facade)
  {
    GridCSpace const & cspace(*facade.GetCSpace());
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for (vertex_read_iteration viter(cspace.begin());
	 viter.not_at_end(); ++viter) {
      const FacadeReadInterface::node_status_t nstat(facade.GetStatus(*viter));
      switch (nstat) {
      case FacadeReadInterface::UPWIND:    glColor3d(0, 0,   1); break;
      case FacadeReadInterface::DOWNWIND:  glColor3d(1, 0.5, 0); break;
      case FacadeReadInterface::WAVEFRONT: glColor3d(1, 0,   0); break;
      case FacadeReadInterface::GOAL:      glColor3d(0, 1,   0); break;
      case FacadeReadInterface::OBSTACLE:  glColor3d(1, 0,   1); break;
      case FacadeReadInterface::OUT_OF_GRID:
      default:
	continue;
      }
      double xc, yc;
      tie(xc, yc) = cspace.ComputePosition(*viter);
      glRectd(xc - 0.5, yc - 0.5, xc + 0.5, yc + 0.5);
    }
  }
  
}
