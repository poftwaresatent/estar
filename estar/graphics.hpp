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


#ifndef GFX_GRAPHICS_HPP
#define GFX_GRAPHICS_HPP


#include <estar/util.hpp>
#include <stddef.h>


namespace estar {

  class FacadeReadInterface;
  class Grid;
  class Algorithm;
  class RiskMap;
  class Kernel;
  class Region;
  
}


namespace gfx {
  
  
  typedef enum {
    GREEN_PINK_BLUE,
    GREY_WITH_SPECIAL,
    BLUE_GREEN_RED,
    INVERTED_GREY,
    RED
  } colorscheme_enum_t;
  
  
  /** Utility for translating a value into a color. User-extendable! */
  class ColorScheme
  {
  public:
    virtual ~ColorScheme() { }
    virtual void Set(double value) const = 0;
    static const ColorScheme * Get(colorscheme_enum_t number);
  };
  
  
  /** Maps a value from the navigation function into the range [0..1]
      and forwards the choosing of the color to its registered
      ColorScheme. The mapping is a 'flat-headed' saw-tooth function,
      where users control the period of the saw-tooth and the width of
      the flat sections. */
  class ColorCycle
    : public ColorScheme
  {
  public:
    ColorCycle(const ColorScheme * scheme,
	       /** the cycle repeats with this period (ensured to be
		   >= epsilon) */
	       double period,
	       /** the mapped value stays 0 or 1 within 'width' of
		   N*period ('width' is trunkated to period/3) */
	       double width);
    
    /** Periodically map 'value' to [0..1] using ComputeMapping() and
	forward to the registered scheme. */
    virtual void Set(double value) const;
    
    double ComputeMapping(double value) const;
    
  protected:
    const ColorScheme * m_scheme;
    double m_period, m_width, m_half_period, m_scaled_width, m_scale;
  };
  
  
  void draw_grid_value(const estar::Grid & grid,
		       const estar::Algorithm & algo,
		       const ColorScheme * colorscheme,
		       bool auto_scale_value);
  
  void draw_grid_value(const estar::FacadeReadInterface & facade,
		       const ColorScheme * colorscheme,
		       bool auto_scale_value);
  
  void draw_grid_risk(const estar::Grid & grid,
		      const estar::Algorithm & algo,
		      const estar::RiskMap & riskmap,
		      const ColorScheme * colorscheme);
  
  void draw_grid_meta(const estar::Grid & grid,
 		      const estar::Algorithm & algo,
		      const estar::Kernel & kernel,
		      const ColorScheme * colorscheme);
  
  void draw_grid_meta(const estar::FacadeReadInterface & facade,
		      const ColorScheme * colorscheme);
  
  void draw_grid_obstacles(const estar::FacadeReadInterface & facade,
			   double red, double green, double blue);
  
  void draw_array(const estar::array<double> & grid,
		  size_t x0, size_t y0, size_t x1, size_t y1,
		  double lower, double upper,
		  const ColorScheme * colorscheme);
  
  void draw_trace(const estar::Grid & grid,
		  const estar::Algorithm & algo,
		  size_t robot_ix, size_t robot_iy,
		  double stepsize, double goalradius,
		  const ColorScheme * colorscheme,
		  double fail_r, double fail_g, double fail_b);
  
  void draw_trace(const estar::FacadeReadInterface & facade,
		  double robot_x, double robot_y, double goalradius,
		  const ColorScheme * colorscheme,
		  double fail_r, double fail_g, double fail_b);
  
  void draw_grid_queue(const estar::Grid & grid,
		       const estar::Algorithm & algo);
  
  void draw_grid_queue(const estar::FacadeReadInterface & facade);
  
  void draw_grid_upwind(const estar::Grid & grid,
			const estar::Algorithm & algo,
			double red, double green, double blue,
			double linewidth);
  
  void draw_grid_upwind(const estar::FacadeReadInterface & facade,
			double red, double green, double blue,
			double linewidth);
  
  void draw_grid_connect(const estar::Grid & grid,
			 const estar::Algorithm & algo,
			 double red, double green, double blue,
			 double linewidth);
  
  void draw_grid_status(const estar::FacadeReadInterface & facade);
  
  void get_grid_bbox(const estar::Grid & grid,
		     double & x0, double & y0, double & x1, double & y1);
  
  void get_grid_bbox(const estar::FacadeReadInterface & facade,
		     double & x0, double & y0, double & x1, double & y1);
  
  void draw_region(const estar::Region & region,
		   double red, double green, double blue);
  
}

#endif // GFX_GRAPHICS_HPP
