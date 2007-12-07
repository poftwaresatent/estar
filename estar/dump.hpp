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


#ifndef ESTAR_DUMP_HPP
#define ESTAR_DUMP_HPP


#include <estar/util.hpp>
#include <stdio.h>


namespace estar {
  
  
  class FacadeReadInterface;
  class Algorithm;
  class Grid;
  
  
  /**
     Write an array out to a stream, with formatting appropriate for
     values that lie in the range 0...1 (such as probabilities). A
     sub-array can be selected for dumping by choosing appropriate
     values for x0, y0, x1, y1 (all indices interpreted as inclusive).
     \note BEWARE x1 and y1 are *inclusive*
  */
  void dump_probabilities(const array<double> &prob,
			  size_t x0, size_t y0, size_t x1, size_t y1,
			  FILE * stream);
  
  
  /**
     Similar to dump_probabilities(), but uses a raw format useful eg
     for gnuplot.
     \note BEWARE x1 and y1 are *inclusive*
  */
  void dump_raw(const array<double> &data,
		/** use this instead of infinite values */
		double replace_infinity,
		size_t x0, size_t y0, size_t x1, size_t y1,
		FILE * stream);
  
  
  /**
     Similar to dump_raw(), but specifically for a Grid's values.
     \note BEWARE x1 and y1 are *inclusive*
  */
  void dump_raw_value(const Grid & grid,
		      const Algorithm & algo,
		      ssize_t x0, ssize_t y0, ssize_t x1, ssize_t y1,
		      double infinity_replacement,
		      FILE * stream);
  
  
  /**
     Similar to dump_raw(), but specifically for a Grid's meta information.
     \note BEWARE x1 and y1 are *inclusive*
  */
  void dump_raw_meta(const Grid & grid,
		     const Algorithm & algo,
		     ssize_t x0, ssize_t y0, ssize_t x1, ssize_t y1,
		     FILE * stream);
  
  /** Similar to eg dump_raw_value(), but writes all values and meta
      data through a facade instance to two streams (writes
      sequentially, so things will not get mixed up if you provide the
      same stream twice). Set a stream to NULL if you're not interested
      in that part of the dump. Uses -1 as infinity_replacement for
      dump_raw_value(). */
  void dump_raw(const FacadeReadInterface & facade,
		FILE * value_stream,
		FILE * meta_stream);
  
  
  /** if grid is null it is ignored, if limit is > 0 it isn't */
  void dump_queue(const Algorithm & algo, const Grid * grid, size_t limit,
		  FILE * stream);
  
  void dump_queue(const FacadeReadInterface & facade, size_t limit,
		  FILE * stream);
  
  
  void dump_grid(const Grid & grid, FILE * stream);
  void dump_grid_range(const Grid & grid,
		       ssize_t ix0, ssize_t iy0, ssize_t ix1, ssize_t iy1,
		       FILE * stream);
  void dump_grid_range_highlight(const Grid & grid,
				 ssize_t ix0, ssize_t iy0,
				 ssize_t ix1, ssize_t iy1,
				 ssize_t ixhigh, ssize_t iyhigh,
				 FILE * stream);
  void dump_facade_range_highlight(const FacadeReadInterface & facade,
				   ssize_t ix0, ssize_t iy0,
				   ssize_t ix1, ssize_t iy1,
				   ssize_t ixhigh, ssize_t iyhigh,
				   FILE * stream);

  void dump_upwind(const Algorithm & algo, const Grid * grid, FILE * stream);
    
}

#endif // ESTAR_DUMP_HPP
