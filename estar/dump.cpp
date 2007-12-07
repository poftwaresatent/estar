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


#include "dump.hpp"
#include "FacadeReadInterface.hpp"
#include "Algorithm.hpp"
#include "Grid.hpp"
#include "util.hpp"
#include "pdebug.hpp"


using namespace boost;


namespace estar {
  
  
  void dump_probabilities(const array<double> &prob,
			  size_t x0, size_t y0, size_t x1, size_t y1,
			  FILE * stream)
  {
    for(size_t iy(y1); iy >= y0; --iy){
      for(size_t ix(x0); ix <= x1; ++ix)
	fprintf(stream, "+-----");
      fprintf(stream, "+\n");
      for(size_t ix(x0); ix <= x1; ++ix){
	const double value(prob[ix][iy]);
	if(value < 0)           fprintf(stream, "| < 0 ");
	else if(value > 1)      fprintf(stream, "| > 1 ");
	else if(value < 1e-9)   fprintf(stream, "|tiny ");
	else if(value > 1-1e-9) fprintf(stream, "| one ");
	else                    fprintf(stream, "|%5.2f", value);
      }
      fprintf(stream, "|\n");
    }
    for(size_t ix(x0); ix <= x1; ++ix)
      fprintf(stream, "+-----");
    fprintf(stream, "+\n");
  }
  
  
  void dump_gnuplottable(const array<double> &data,
			 size_t x0, size_t y0, size_t x1, size_t y1,
			 FILE * stream)
  {
    fprintf(stream, "# x: %zd...%zd\n# y: %zd...%zd\n", x0, x1, y0, y1);
    for(size_t x(x0); x <= x1; x++){
      for(size_t y(y0); y <= y1; y++)
	fprintf(stream, "%zd   %zd   %f\n", x, y, data[x][y]);
      fprintf(stream, "\n");
    }
  }
  
  
  void dump_raw(const array<double> &data,
		double replace_infinity,
		size_t x0, size_t y0, size_t x1, size_t y1,
		FILE * stream)
  {
    fprintf(stream, "# x: %zd...%zd\n# y: %zd...%zd\n", x0, x1, y0, y1);
    for(size_t x(x0); x <= x1; x++){
      for(size_t y(y0); y <= y1; y++)
	if(infinity != data[x][y])
	  fprintf(stream, "%zd   %zd   %f\n", x, y, data[x][y]);
	else
	  fprintf(stream, "%zd   %zd   %f\n", x, y, replace_infinity);	  
      fprintf(stream, "\n");
    }
  }
  
  
  void dump_raw_value(const Grid & grid,
		      const Algorithm & algo,
		      ssize_t x0, ssize_t y0, ssize_t x1, ssize_t y1,
		      double infinity_replacement,
		      FILE * stream)
  {
    fprintf(stream, "# x: %zd...%zd\n# y: %zd...%zd\n", x0, x1, y0, y1);
    const value_map_t & value(algo.GetValueMap());
    for(ssize_t x(x0); x <= x1; x++){
      for(ssize_t y(y0); y <= y1; y++){
	const double vv(get(value, grid.Index2Vertex(x, y)));
	if(vv != infinity)
	  fprintf(stream, "%zd   %zd   %f\n", x, y, vv);
	else
	  fprintf(stream, "%zd   %zd   %f\n", x, y, infinity_replacement);
      }
      fprintf(stream, "\n");
    }
  }
  
  
  void dump_raw_meta(const Grid & grid,
		     const Algorithm & algo,
		     ssize_t x0, ssize_t y0, ssize_t x1, ssize_t y1,
		     FILE * stream)
  {
    fprintf(stream, "# x: %zd...%zd\n# y: %zd...%zd\n", x0, x1, y0, y1);
    const meta_map_t & meta(algo.GetMetaMap());
    for(ssize_t x(x0); x <= x1; x++){
      for(ssize_t y(y0); y <= y1; y++)
	fprintf(stream, "%zd   %zd   %f\n", x, y,
		get(meta, grid.Index2Vertex(x, y)));
      fprintf(stream, "\n");
    }
  }
  
  
  void dump_raw(const FacadeReadInterface & facade,
		FILE * value_stream,
		FILE * meta_stream)
  {
    ////    shared_ptr<Grid const> grid(facade.GetGrid());
    Grid const & grid(facade.GetGrid());
    ////     if ( ! grid)
    ////       return;
    ssize_t const xsize(grid.GetXSize());
    if (xsize < 1)
      return;
    ssize_t const ysize(grid.GetYSize());
    if (ysize < 1)
      return;
    Algorithm const & algo(facade.GetAlgorithm());
    if(value_stream)
      dump_raw_value(grid, algo, 0, 0, xsize - 1, ysize - 1, -1, value_stream);
    if(meta_stream)
      dump_raw_meta(grid, algo, 0, 0, xsize - 1, ysize - 1, meta_stream);  
  }
  
  
  void dump_queue(const Algorithm & algo, const Grid * grid, size_t limit,
		  FILE * stream)
  {
    const queue_t & queue(algo.GetQueue().Get());
    if(queue.empty()){
      fprintf(stream, "queue: empty\n");
      return;
    }
    const flag_map_t & flag(algo.GetFlagMap());
    const value_map_t & value(algo.GetValueMap());
    const rhs_map_t & rhs(algo.GetRhsMap());
    size_t count(0);
    const_queue_it iq(queue.begin());
    const double firstkey(iq->first);
    fprintf(stream, "queue:\n");
    for(/**/; iq != queue.end(); ++iq, ++count){
      const vertex_t vertex(iq->second);
      fprintf(stream, "  %s f: %s %s i: %zu",
	      iq->first == firstkey ? "*" : " ",
	      flag_name(get(flag, vertex)),
	      get(rhs, vertex) < get(value, vertex)
	      ? "lower" : "raise", vertex);
      if(0 != grid){
	const GridNode & gn(grid->Vertex2Node(vertex));
	fprintf(stream, " (%zu, %zu)", gn.ix, gn.iy);
      }
      const double vv(get(value, vertex));
      if(vv == infinity) fprintf(stream, " k: %g v: inf", iq->first);
      else               fprintf(stream, " k: %g v: %g", iq->first, vv);
      const double rr(get(rhs, vertex));
      if(rr == infinity) fprintf(stream, " rhs: inf\n");
      else               fprintf(stream, " rhs: %g\n", rr);
      if((limit > 0) && (count > limit) && (iq->first != firstkey))
	break;
    }
  }
  
  
  void dump_queue(const FacadeReadInterface & facade, size_t limit,
		  FILE * stream)
  {
    dump_queue(facade.GetAlgorithm(), &facade.GetGrid(), limit, stream);
  }
  
  
  static void linesep(const Grid & grid, FILE * stream,
		      ssize_t ix0, ssize_t ix1,
		      const char * prefix, const char * high)
  {
    char * line;
    if(grid.connect == HEX_GRID) line = "+-----+-----";
    else                         line = "+-----------";
    if(0 != high)   fprintf(stream, high);
    if(0 != prefix) fprintf(stream, prefix);
    for(ssize_t ix(ix0); ix <= ix1; ++ix)
      fprintf(stream, line);
    if(0 != high) fprintf(stream, "+%s\n", high);
    else          fprintf(stream, "+\n");
  }
  
  
  static const double huge(500);
  
  
  static void line1(const Grid & grid, FILE * stream,
		    ssize_t iy, ssize_t ix0, ssize_t ix1,
		    const char * prefix, const char * high)
  {
    if(0 != high) fprintf(stream, high);
    if(0 != prefix) fprintf(stream, prefix);
    shared_ptr<GridCSpace const> const cspace(grid.GetCSpace());
    for(ssize_t ix(ix0); ix <= ix1; ++ix){
      const double meta(cspace->GetMeta(grid.Index2Vertex(ix, iy)));
      if(infinity == meta)
	fprintf(stream, "|infty ");
      else if(huge <= meta)
	fprintf(stream, "|huge  ");
      else
	fprintf(stream, "|%5.2f ", meta);
      const double value(cspace->GetValue(grid.Index2Vertex(ix, iy)));
      if(infinity == value)
	fprintf(stream, "infty");
      else if(huge <= value)
	fprintf(stream, "huge ");
      else
	fprintf(stream, "%5.2f", value);
    }
    if(0 != high) fprintf(stream, "|%s\n", high);
    else          fprintf(stream, "|\n");
  }
  
  
  static void line2(const Grid & grid, FILE * stream,
		    ssize_t iy, ssize_t ix0, ssize_t ix1,
		    const char * prefix, const char * high)
  {
    if(0 != high) fprintf(stream, high);
    if(0 != prefix) fprintf(stream, prefix);
    shared_ptr<GridCSpace const> const cspace(grid.GetCSpace());
    for(ssize_t ix(ix0); ix <= ix1; ++ix){
      const flag_t flag(cspace->GetFlag(grid.Index2Vertex(ix, iy)));
      if(NONE == flag)
	fprintf(stream, "|      ");
      else
	fprintf(stream, "|%5s ", flag_name(flag));
      const double rhs(cspace->GetRhs(grid.Index2Vertex(ix, iy)));
      if(infinity == rhs)
	fprintf(stream, "infty");
      else if(huge <= rhs)
	fprintf(stream, "huge ");
      else
	fprintf(stream, "%5.2f", rhs);
    }
    if(0 != high) fprintf(stream, "|%s\n", high);
    else          fprintf(stream, "|\n");
  }
  
  
  static void line3(const Grid & grid, FILE * stream,
		    ssize_t iy, ssize_t ix0, ssize_t ix1,
		    const char * prefix, const char * high)
  {
    if(0 != high) fprintf(stream, high);
    if(0 != prefix) fprintf(stream, prefix);
    shared_ptr<GridCSpace const> const cspace(grid.GetCSpace());
    for(ssize_t ix(ix0); ix <= ix1; ++ix){
      const vertex_t vertex(grid.Index2Vertex(ix, iy));
      if( ! (OPEN & cspace->GetFlag(vertex)))
	fprintf(stream, "|      ");
      else if(cspace->GetRhs(vertex) < cspace->GetValue(vertex))
	fprintf(stream, "|lower ");
      else if(cspace->GetRhs(vertex) > cspace->GetValue(vertex))
	fprintf(stream, "|raise ");
      else
	fprintf(stream, "|r==v? ");
      fprintf(stream, "%5zu", grid.Index2Vertex(ix, iy));
    }
    if(0 != high) fprintf(stream, "|%s\n", high);
    else          fprintf(stream, "|\n");
  }
  
  
  static void line4(const Grid & grid, FILE * stream,
		    ssize_t iy, ssize_t ix0, ssize_t ix1,
		    const char * prefix, const char * high)
  {
    if(0 != high) fprintf(stream, high);
    if(0 != prefix) fprintf(stream, prefix);
    for(ssize_t ix(ix0); ix <= ix1; ++ix){
      const GridNode & node(grid.Index2Node(ix, iy));
      fprintf(stream, "| (%3zu, %3zu)", node.ix, node.iy);
    }
    if(0 != high) fprintf(stream, "|%s\n", high);
    else          fprintf(stream, "|\n");
  }


  void dump_grid(const Grid & grid, FILE * stream)
  {
    dump_grid_range(grid, 0, 0, grid.xsize - 1, grid.ysize - 1, stream);
  }
  
  
  //  2  +-----+-----+-----+-----+
  //  2  |12345 abcde|12345 abcde|
  //  1  +-----+-----+-----+-----+-----+
  //  1        |12345 abcde|12345 abcde|
  //  0  +-----+-----+-----+-----+-----+
  //  0  |12345 abcde|12345 abcde|
  // x0  +-----+-----+-----+-----+
  
  //  3        +-----+-----+-----+-----+
  //  3        |12345 abcde|12345 abcde|
  //  2  +-----+-----+-----+-----+-----+
  //  2  |12345 abcde|12345 abcde|
  //  1  +-----+-----+-----+-----+-----+
  //  1        |12345 abcde|12345 abcde|
  // x1        +-----+-----+-----+-----+
  
  void dump_grid_range(const Grid & grid,
		       ssize_t ix0, ssize_t iy0, ssize_t ix1, ssize_t iy1,
		       FILE * stream)
  {
    PVDEBUG("%zu   %zu   %zu   %zu\n", ix0, iy0, ix1, iy1);
    const char * even("");
    const char * oddsep(grid.connect == HEX_GRID ? "+-----" : even);
    const char * oddpre(grid.connect == HEX_GRID ? "      " : even);
    ssize_t iy(iy1);
    const char * prefix;
    for(/**/; iy != iy0; --iy){
      if(iy % 2){
	linesep(grid, stream, ix0, ix1, oddsep, 0);
	prefix = oddpre;
      }
      else{
	linesep(grid, stream, ix0, ix1, even, 0);
	prefix = even;
      }
      line1(grid, stream, iy, ix0, ix1, prefix, 0);
      line2(grid, stream, iy, ix0, ix1, prefix, 0);
      line3(grid, stream, iy, ix0, ix1, prefix, 0);
      line4(grid, stream, iy, ix0, ix1, prefix, 0);
    }
    if(iy % 2){
      linesep(grid, stream, ix0, ix1, oddsep, 0);
      prefix = oddpre;
    }
    else{
      linesep(grid, stream, ix0, ix1, even, 0);
      prefix = even;
    }
    line1(grid, stream, iy, ix0, ix1, prefix, 0);
    line2(grid, stream, iy, ix0, ix1, prefix, 0);
    line3(grid, stream, iy, ix0, ix1, prefix, 0);
    line4(grid, stream, iy, ix0, ix1, prefix, 0);
    if(iy % 2)
      linesep(grid, stream, ix0, ix1, even, 0);
    else
      linesep(grid, stream, ix0, ix1, oddsep, 0);
  }
  
  
  void dump_facade_range_highlight(const FacadeReadInterface & facade,
				   ssize_t ix0, ssize_t iy0,
				   ssize_t ix1, ssize_t iy1,
				   ssize_t ixhigh, ssize_t iyhigh,
				   FILE * stream)
  {
    dump_grid_range_highlight(facade.GetGrid(), ix0, iy0, ix1, iy1,
			      ixhigh, iyhigh, stream);
  }
  
  
  void dump_grid_range_highlight(const Grid & grid,
				 ssize_t ix0, ssize_t iy0,
				 ssize_t ix1, ssize_t iy1,
				 ssize_t ixhigh, ssize_t iyhigh,
				 FILE * stream)
  {
    PVDEBUG("%zu   %zu   %zu   %zu   %zu   %zu\n",
	    ix0, iy0, ix1, iy1, ixhigh, iyhigh);
    fprintf(stream, " ");
    for(ssize_t ix(ix0); ix <= ix1; ++ix)
      if(ix == ixhigh) fprintf(stream, " ***********");
      else             fprintf(stream, "            ");
    fprintf(stream, " \n");
    ssize_t iy(iy1);
    for(/**/; iy != iy0; --iy){
      const char * high(iy == iyhigh ? "*" : " ");
      linesep(grid, stream, ix0, ix1, 0, " ");
      line1(grid, stream, iy, ix0, ix1, 0, high);
      line2(grid, stream, iy, ix0, ix1, 0, high);
      line3(grid, stream, iy, ix0, ix1, 0, high);
      line4(grid, stream, iy, ix0, ix1, 0, high);
    }
    const char * high(iy == iyhigh ? "*" : " ");
    linesep(grid, stream, ix0, ix1, 0, " ");
    line1(grid, stream, iy, ix0, ix1, 0, high);
    line2(grid, stream, iy, ix0, ix1, 0, high);
    line3(grid, stream, iy, ix0, ix1, 0, high);
    line4(grid, stream, iy, ix0, ix1, 0, high);
    linesep(grid, stream, ix0, ix1, 0, " ");
    fprintf(stream, " ");
    for(ssize_t ix(ix0); ix <= ix1; ++ix)
      if(ix == ixhigh) fprintf(stream, " ***********");
      else             fprintf(stream, "            ");
    fprintf(stream, " \n");
  }
  

  void dump_upwind(const Algorithm & algo, const Grid * grid, FILE * stream)
  {
    const Upwind & upwind(algo.GetUpwind());
    const GridNode * gfrom(0);
    vertex_it iv, vend;
    tie(iv, vend) = vertices(algo.GetCSpaceGraph());
    if(iv == vend)
      return;
    fprintf(stream, "upwind edges (from, to):\n");
    for(/**/; iv != vend; ++iv){
      if(0 != grid) gfrom = &(grid->Vertex2Node(*iv));
      const Upwind::set_t & downwind(upwind.GetDownwind(*iv));
      for(Upwind::set_t::const_iterator id(downwind.begin());
	  id != downwind.end(); ++id){
	fprintf(stream, "  (%zu, %zu)", *iv, *id);
	if(0 != grid){
	  const GridNode & gto(grid->Vertex2Node(*id));
	  fprintf(stream, " [(%zu, %zu) -> (%zu, %zu)]",
		  gfrom->ix, gfrom->iy, gto.ix, gto.iy);
	}
	fprintf(stream, "\n");
      }
    }
  }
  
}
