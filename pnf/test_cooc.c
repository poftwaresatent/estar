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


#include "pnf_cooc.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


static void dump_double(FILE *stream,
			const char * comment,
			double * posvector, double * dataarray,
			int isize, int jsize);
static void dump_int(FILE *stream,
		     const char * comment,
		     double * posvector, int * dataarray,
		     int isize, int jsize);
static void dump_plot(FILE *stream,
		      const char * name,
		      int dataset, int jsize);
static void dump_plot_fig(FILE *stream,
			  const char * name,
			  int dataset, int jsize);
static void lwpp(double r,
		 double *d, int d_len,
		 double vr,
		 double vm,
		 double s1, double s2, double s,
		 double ** cooc,
		 double ** pos, int * pos_len,
		 double ** pleft,
		 double ** pbothleft,
		 double ** pmiddle,
		 double ** pbothright,
		 double ** pright,
		 int ** left,
		 int ** bothleft,
		 int ** middle,
		 int ** bothright,
		 int ** right);


int main(int argc, char ** argv)
{
  double robot_position = 0;
  double object_position[] = {-5, -10, -15, -20, -25, -30};
  int object_position_len = 6;
  double robot_speed = 10;
  double object_speed = 20;
  double start_pos = -40;
  double end_pos = 40;
  double resolution = 0.5;
  
  double * co_occurrence;
  double * position;
  int position_len;
  
  double * pleft;
  double * pbothleft;
  double * pmiddle;
  double * pbothright;
  double * pright;
  int * left;
  int * bothleft;
  int * middle;
  int * bothright;
  int * right;
  
  FILE *datastream = fopen("test_cooc.data", "w");
  if(NULL == datastream){
    perror("test_cooc.data");
    exit(EXIT_FAILURE);
  }
  
  FILE *plotstream = fopen("test_cooc.plot", "w");
  if(NULL == plotstream){
    perror("test_cooc.plot");
    fclose(datastream);
    exit(EXIT_FAILURE);
  }
  
  FILE *plotallstream = fopen("test_cooc.plotall", "w");
  if(NULL == plotallstream){
    perror("test_cooc.plotall");
    fclose(datastream);
    fclose(plotstream);
    exit(EXIT_FAILURE);
  }
  
  FILE *plotfigstream = fopen("test_cooc.plotfig", "w");
  if(NULL == plotfigstream){
    perror("test_cooc.plotfig");
    fclose(datastream);
    fclose(plotstream);
    fclose(plotallstream);
    exit(EXIT_FAILURE);
  }
  
  lwpp(robot_position,
       object_position, object_position_len,
       robot_speed,
       object_speed,
       start_pos, end_pos, resolution,
       &co_occurrence,
       &position, &position_len,
       &pleft,
       &pbothleft,
       &pmiddle,
       &pbothright,
       &pright,
       &left,
       &bothleft,
       &middle,
       &bothright,
       &right);

  fprintf(datastream,
	  "### IN\n"
	  "# robot_position == %f\n"
	  "# object_position == (list)\n"
	  "# object_position_len == %d\n"
	  "# robot_speed == %f\n"
	  "# object_speed == %f\n"
	  "# start_pos == %f\n"
	  "# end_pos == %f\n"
	  "# resolution == %f\n"
	  "### OUT\n"
	  "# position_len == %d\n",
	  robot_position, object_position_len, robot_speed, object_speed,
	  start_pos, end_pos, resolution, position_len);
  
  dump_double(datastream,
	      "\n\n# pos\tco_occurrence[ipos][iobj]:\n",
	      position, co_occurrence, position_len, object_position_len);
  dump_double(datastream,
	      "\n\n# pos\tpleft[ipos][iobj]:\n",
	      position, pleft, position_len, object_position_len);
  dump_double(datastream,
	      "\n\n# pos\tpbothleft[ipos][iobj]:\n",
	      position, pbothleft, position_len, object_position_len);
  dump_double(datastream,
	      "\n\n# pos\tpmiddle[ipos][iobj]:\n",
	      position, pmiddle, position_len, object_position_len);
  dump_double(datastream,
	      "\n\n# pos\tpbothright[ipos][iobj]:\n",
	      position, pbothright, position_len, object_position_len);
  dump_double(datastream,
	      "\n\n# pos\tpright[ipos][iobj]:\n",
	      position, pright, position_len, object_position_len);
  
  dump_int(datastream,
	   "\n\n# pos\tleft[ipos][iobj]:\n",
	   position, left, position_len, object_position_len);
  dump_int(datastream,
	   "\n\n# pos\tbothleft[ipos][iobj]:\n",
	   position, bothleft, position_len, object_position_len);
  dump_int(datastream,
	   "\n\n# pos\tmiddle[ipos][iobj]:\n",
	   position, middle, position_len, object_position_len);
  dump_int(datastream,
	   "\n\n# pos\tbothright[ipos][iobj]:\n",
	   position, bothright, position_len, object_position_len);
  dump_int(datastream,
	   "\n\n# pos\tright[ipos][iobj]:\n",
	   position, right, position_len, object_position_len);
  
  dump_plot(plotstream, "co_occurrence", 0, object_position_len);
  dump_plot_fig(plotfigstream, "co_occurrence", 0, object_position_len);
  
  dump_plot(plotallstream, "co_occurrence", 0, object_position_len);
  dump_plot(plotallstream, "pleft",         1, object_position_len);
  dump_plot(plotallstream, "pbothleft",     2, object_position_len);
  dump_plot(plotallstream, "pmiddle",       3, object_position_len);
  dump_plot(plotallstream, "pbothright",    4, object_position_len);
  dump_plot(plotallstream, "pright",        5, object_position_len);
  //   dump_plot(plotallstream, "left",          6, object_position_len);
  //   dump_plot(plotallstream, "bothleft",      7, object_position_len);
  //   dump_plot(plotallstream, "middle",        8, object_position_len);
  //   dump_plot(plotallstream, "bothright",     9, object_position_len);
  //   dump_plot(plotallstream, "right",        10, object_position_len);
  
  free(co_occurrence);
  free(position);
  free(pleft);
  free(pbothleft);
  free(pmiddle);
  free(pbothright);
  free(pright);
  free(left);
  free(bothleft);
  free(middle);
  free(bothright);
  free(right);
  
  fclose(datastream);
  fclose(plotstream);
  fclose(plotallstream);
  fclose(plotfigstream);
  
  printf("sh test_cooc.plot\n");
  printf("sh test_cooc.plotall\n");
  printf("sh test_cooc.plotfig\n");
  
  return 0;
}


/** legacy API wrapper */
void lwpp(double r,
	  double * d, int d_len,
	  double vr,
	  double vm,
	  double s1, double s2, double s,
	  double ** cooc,
	  double ** pos, int * pos_len,
	  double ** pleft,
	  double ** pbothleft,
	  double ** pmiddle,
	  double ** pbothright,
	  double ** pright,
	  int ** left,
	  int ** bothleft,
	  int ** middle,
	  int ** bothright,
	  int ** right)
{
  //   30/03/03, btj, EPFL-ASL
  
  // define plot range
  int i;
  (*pos_len) = (int) ceil((s2 - s1) / s);
  (*pos) = calloc((*pos_len), sizeof(double));
  for(i = 0; i < (*pos_len); i++)
    (*pos)[i] = s1 + i * s + s/2;
  
  // allocate return arrays
  int array_len = d_len * (*pos_len);
  *cooc = calloc(array_len, sizeof(double));
  *pleft = calloc(array_len, sizeof(double));
  *pbothleft = calloc(array_len, sizeof(double));
  *pmiddle = calloc(array_len, sizeof(double));
  *pbothright = calloc(array_len, sizeof(double));
  *pright = calloc(array_len, sizeof(double));
  *left = calloc(array_len, sizeof(int));
  *bothleft = calloc(array_len, sizeof(int));
  *middle = calloc(array_len, sizeof(int));
  *bothright = calloc(array_len, sizeof(int));
  *right = calloc(array_len, sizeof(int));
  
  int foo;
  
  // do the computations
  for(i = 0; i < d_len; i++){
    int j;
    for(j = 0; j < (*pos_len); ++j){
      double lambda_r = (*pos)[j] - r;
      if(lambda_r < 0)
	lambda_r = - lambda_r;
      int k = i + j * d_len;
      double lambda_i = (*pos)[j] - d[i];
      (*cooc)[k] = pnf_cooc_detail(lambda_i,
				   lambda_r,
				   vm,
				   vr,
				   s,
				   &((*pleft)[k]),
				   &((*pbothleft)[k]),
				   &((*pmiddle)[k]),
				   &((*pbothright)[k]),
				   &((*pright)[k]),
				   &((*left)[k]),
				   &((*bothleft)[k]),
				   &((*middle)[k]),
				   &((*bothright)[k]),
				   &((*right)[k]),
				   &foo);
    }
  }
}


void dump_double(FILE *stream,
		 const char * comment,
		 double * posvector, double * dataarray,
		 int isize, int jsize)
{
  int i, j;
  fprintf(stream, comment);
  for(i = 0; i < isize; i++){
    fprintf(stream, "%f", posvector[i]);
    for(j = 0; j < jsize; j++)
      fprintf(stream, "\t%f", dataarray[j + i * jsize]);
    fprintf(stream, "\n");
  }
}


void dump_int(FILE *stream,
	      const char * comment,
	      double * posvector, int * dataarray,
	      int isize, int jsize)
{
  int i, j;
  fprintf(stream, comment);
  for(i = 0; i < isize; i++){
    fprintf(stream, "%f", posvector[i]);
    for(j = 0; j < jsize; j++)
      fprintf(stream, "\t%d", dataarray[j + i * jsize]);
    fprintf(stream, "\n");
  }
}


void dump_plot(FILE *stream,
	       const char * name,
	       int dataset, int jsize)
{
  int j;
  fprintf(stream,
	  "# %s:\n"
	  "gnuplot -persist <<EOF\n"
	  "plot 'test_cooc.data' i %d u 1:2 w l t '%s 0'",
	  name, dataset, name);
  for(j = 1; j < jsize; j++)
    fprintf(stream, ", 'test_cooc.data' i %d u 1:%d w l t '%s %d'",
	    dataset, j + 2, name, j);
  fprintf(stream, "\nEOF\n");
}


void dump_plot_fig(FILE *stream,
		   const char * name,
		   int dataset, int jsize)
{
  int j;
  fprintf(stream,
	  "# %s:\n"
	  "gnuplot <<EOF\n"
	  "set terminal fig\n"
	  "set output 'test_cooc.fig'\n"
	  "plot 'test_cooc.data' i %d u 1:2 w l t '0'",
	  name, dataset);
  for(j = 1; j < jsize; j++)
    fprintf(stream, ", 'test_cooc.data' i %d u 1:%d w l t '%d'",
	    dataset, j + 2, j);
  fprintf(stream, "\nEOF\n");
}
