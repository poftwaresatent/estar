/* 
 * Copyright (C) 2006
 * Roland Philippsen <roland dot philippsen at gmx dot net>
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


#include "PNFRiskMap.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>


using namespace std;
using namespace pnf;


static void dump_double(FILE *stream, const string * desc,
			double * risk, double ** meta,
			size_t npoints, size_t ncurves);
static void dump_plot(FILE *stream, const string * desc, size_t ncurves);
static void dump_plot_fig(FILE *stream, const string * desc, size_t ncurves);


int main(int argc, char ** argv)
{  
  FILE *datastream = fopen("test_riskmap.data", "w");
  if(NULL == datastream){
    perror("test_riskmap.data");
    exit(EXIT_FAILURE);
  }
  
  FILE *plotstream = fopen("test_riskmap.plot", "w");
  if(NULL == plotstream){
    perror("test_riskmap.plot");
    fclose(datastream);
    exit(EXIT_FAILURE);
  }
  
  FILE *plotfigstream = fopen("test_riskmap.plotfig", "w");
  if(NULL == plotfigstream){
    perror("test_riskmap.plotfig");
    fclose(datastream);
    fclose(plotstream);
    exit(EXIT_FAILURE);
  }
  
  const size_t npoints = 500;
  const size_t ncurves = 3;
  double * risk = new double[npoints];
  double ** meta = new double*[ncurves];
  for(size_t icurve(0); icurve < ncurves; ++icurve)
    meta[icurve] = new double[npoints];
  
  static const char * name[] = {"sigma", "sigma", "sigma"};
  static const double cutoff[] = { 0.8, 0.95, 0.95 };
  static const double degree[] = { 2.0, 1.5, 5.0 };
  
  PNFRiskMap * riskmap[ncurves];
  string desc[ncurves];
  for(size_t icurve(0); icurve < ncurves; ++icurve){
    riskmap[icurve] =
      PNFRiskMap::Create(name[icurve], cutoff[icurve], degree[icurve]);
    if( ! riskmap[icurve]){
      cerr << "Oops, PNFRiskMap::Create(" << name[icurve]
	   << ") returned null!\n";
      exit(EXIT_FAILURE);
    }
    ostringstream foo;
    foo << "\"" << name[icurve] << "\" r0=" << cutoff[icurve]
	<< " n=" << degree[icurve];
    desc[icurve] = foo.str();
  }
  
  for(size_t ipoint(0); ipoint < npoints; ++ipoint){
    risk[ipoint] = ipoint / (static_cast<double>(npoints) - 1.0);
    for(size_t icurve(0); icurve < ncurves; ++icurve)
      meta[icurve][ipoint] = riskmap[icurve]->RiskToMeta(risk[ipoint]);
  }
  
  dump_double(datastream, desc, risk, meta, npoints, ncurves);
  dump_plot(plotstream, desc, ncurves);
  dump_plot_fig(plotfigstream, desc, ncurves);
  
  delete[] risk;
  for(size_t icurve(0); icurve < ncurves; ++icurve){
    delete[] meta[icurve];
    delete riskmap[icurve];
  }
  delete[] meta;
  
  fclose(datastream);
  fclose(plotstream);
  fclose(plotfigstream);
  
  printf("sh test_riskmap.plot\n");
  printf("sh test_riskmap.plotfig\n");
  
  return 0;
}


void dump_double(FILE *stream, const string * desc,
		 double * risk, double ** meta,
		 size_t npoints, size_t ncurves)
{
  for(size_t icurve(0); icurve < ncurves; ++icurve)
    fprintf(stream, "# %s\n", desc[icurve].c_str());
  for(size_t ipoint(0); ipoint < npoints; ++ipoint){
    fprintf(stream, "%g", risk[ipoint]);
    for(size_t icurve(0); icurve < ncurves; ++icurve)
      fprintf(stream, "\t%g", meta[icurve][ipoint]);
    fprintf(stream, "\n");
  }
}


void dump_plot(FILE *stream, const string * desc, size_t ncurves)
{
  fprintf(stream, "gnuplot -persist <<EOF\n");
  for(size_t icurve(0); icurve < ncurves; ++icurve)
    fprintf(stream, "%s 'test_riskmap.data' u 1:%lu w l t '%s'",
	    icurve == 0 ? "plot" : ",", icurve + 2, desc[icurve].c_str());
  fprintf(stream, "\nEOF\n");
}


void dump_plot_fig(FILE *stream, const string * desc, size_t ncurves)
{
  fprintf(stream,
	  "gnuplot -persist <<EOF\n"
	  "set terminal fig\n"
	  "set output 'test_riskmap.fig'\n");
  for(size_t icurve(0); icurve < ncurves; ++icurve)
    fprintf(stream, "%s 'test_riskmap.data' u 1:%lu w l t '%s'",
	    icurve == 0 ? "plot" : ",", icurve + 2, desc[icurve].c_str());
  fprintf(stream, "\nEOF\n");
}
