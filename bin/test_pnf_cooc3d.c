/* 
 * Copyright (C) 2004
 * Swiss Federal Institute of Technology, Lausanne. All rights reserved.
 * 
 * Author: Roland Philippsen <roland dot philippsen at gmx dot net>
 *         Autonomous Systems Lab <http://asl.epfl.ch/>
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


#include <pnf/pnf_cooc.h>
#include <stdio.h>


static const double lambda_i_0  =  -2;
static const double lambda_i_1  =   2;
static const double lambda_r_0  =  -2;
static const double lambda_r_1  =   2;
static const double lambda_step =   0.1;
static const double v_i = 3;
static const double v_r = 1;
static const double delta = 10e-2;

static const char * dfname = "cooc3d.data";
static const char * pfname = "cooc3d.plot";


int main(int argc, char ** argv)
{
  FILE * df = fopen(dfname, "w");
  if(0 == df){
    perror(dfname);
    return 42;
  }

  FILE * pf = fopen(pfname, "w");
  if(0 == pf){
    fclose(df);
    perror(pfname);
    return 42;
  }
  
  fprintf(df,
	  "# lambda_i: %f ... %f\n"
	  "# lambda_r: %f ... %f\n"
	  "# lambda_step: %f\n"
	  "# v_i: %f\n"
	  "# v_r: %f\n"
	  "# delta: %f\n",
	  lambda_i_0, lambda_i_1, lambda_r_0, lambda_r_1, lambda_step,
	  v_i, v_r, delta);
  
  double l_r, l_i;
  for(l_r = lambda_r_0; l_r <= lambda_r_1; l_r += lambda_step){
    for(l_i = lambda_i_0; l_i <= lambda_i_1; l_i += lambda_step)
      fprintf(df, "%f   %f   %f\n", l_r, l_i,
	      pnf_cooc(l_i, l_r, v_i, v_r, delta));
    fprintf(df, "\n");
  }
  
  fprintf(pf,
	  "gnuplot -persist <<EOF\n"
	  "set contour surface\n"
	  "splot '%s' w l\n"
	  "EOF\n",
	  dfname);
  
  fclose(df);
  fclose(pf);
  
  printf("sh %s\n", pfname);
  return 0;
}
