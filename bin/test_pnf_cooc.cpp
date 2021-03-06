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
#include <estar/numeric.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


using namespace std;
using estar::absval;
using estar::maxval;


struct lwpp_param_s {
  lwpp_param_s(double _rpos, double _rrad, double _opos, double _orad,
	       double _vrob, double _vobj, const string & _msg)
    : rpos(_rpos), rrad(_rrad), opos(_opos), orad(_orad),
      vrob(_vrob), vobj(_vobj), msg(_msg) {}
  double rpos, rrad, opos, orad, vrob, vobj;
  string msg;
};

typedef vector<lwpp_param_s> lwpp_param_t;


static void test_normal(double start_pos, double end_pos, double resolution,
			const lwpp_param_t & param);
static void test_alt(double start_pos, double end_pos, double resolution,
		     const lwpp_param_t & param);


static void dump_double(FILE *stream,
			const char * comment,
			double * posvector, double * dataarray,
			int isize, int jsize);
static void dump_int(FILE *stream,
		     const char * comment,
		     double * posvector, int * dataarray,
		     int isize, int jsize);
static void dump_plot(FILE *stream,
		      const string & title,
		      const lwpp_param_t & param,
		      int dataset);
static void dump_plot_fig(FILE *stream,
			  const string & title,
			  const lwpp_param_t & param,
			  int dataset);
static void lwpp(const lwpp_param_t & param,
		 double s1, double s2, double s,
		 double ** cooc,
		 double ** pos, size_t * pos_len,
		 double ** pleft, double ** pbothleft, double ** pmiddle,
		 double ** pbothright, double ** pright,
		 int ** left, int ** bothleft, int ** middle,
		 int ** bothright, int ** right);


int main(int argc, char ** argv)
{
  string setname("basic");
  if(argc > 1)
    setname = argv[1];
  
  // rpos, rrad, opos, orad, vrob, vobj;
  lwpp_param_t param;
  double start_pos(-40);
  double end_pos(40);
  double resolution(0.5);
  void (*test)(double, double, double, const lwpp_param_t &);
  test = test_normal;
  
  if(setname == "basic"){
    param.push_back(lwpp_param_s(0, 0, -5, 0, 10, 20, "fast point object"));
    param.push_back(lwpp_param_s(0, 0, -5, 0, 10, 10, "slow point object"));
    param.push_back(lwpp_param_s(0, 0, -20, 0, 10, 20, "fast point object"));
    param.push_back(lwpp_param_s(0, 0, -20, 0, 10, 10, "slow point object"));
  }
  else if(setname == "extended"){
    param.push_back(lwpp_param_s(0, 4, -20, 4, 10, 20, "lambda correction"));
    param.push_back(lwpp_param_s(0, 4, -20, 0, 10, 20, "point object"));
    param.push_back(lwpp_param_s(0, 0, -20, 4, 10, 20, "point robot"));
    param.push_back(lwpp_param_s(0, 0, -20, 0, 10, 20, "point object and robot"));
  }
  else if(setname == "wce"){
    resolution = 0.1;
    for(int i(1); i <= 20; ++i){
      ostringstream os;
      os << "\"vi=" << i << "\"";
      param.push_back(lwpp_param_s(5, 0, -5, 0, 10, i, os.str()));
    }
  }
  else if(setname == "alt"){
    param.push_back(lwpp_param_s(5, 0, -5, 0, 10, 20, "envelope for v_i=20"));
    test = test_alt;
  }
  else{
    cerr << argv[0] << ": sorry, don't know about \"" << setname << "\"\n";
    return 1;
  }
  
  test(start_pos, end_pos, resolution, param);
}


/** tweaked legacy API wrapper */
void lwpp(const lwpp_param_t & param,
	  double s1, double s2, double s,
	  double ** cooc,
	  double ** pos, size_t * pos_len,
	  double ** pleft, double ** pbothleft, double ** pmiddle,
	  double ** pbothright, double ** pright,
	  int ** left, int ** bothleft, int ** middle,
	  int ** bothright, int ** right)
{
  // define plot range
  (*pos_len) = static_cast<size_t>(ceil((s2 - s1) / s));
  (*pos) = static_cast<double*>(calloc((*pos_len), sizeof(double)));
  for(size_t ii(0); ii < (*pos_len); ii++)
    (*pos)[ii] = s1 + ii * s + s/2;
  
  // allocate return arrays
  const size_t array_len(param.size() * (*pos_len));
  *cooc = static_cast<double*>(calloc(array_len, sizeof(double)));
  *pleft = static_cast<double*>(calloc(array_len, sizeof(double)));
  *pbothleft = static_cast<double*>(calloc(array_len, sizeof(double)));
  *pmiddle = static_cast<double*>(calloc(array_len, sizeof(double)));
  *pbothright = static_cast<double*>(calloc(array_len, sizeof(double)));
  *pright = static_cast<double*>(calloc(array_len, sizeof(double)));
  *left = static_cast<int*>(calloc(array_len, sizeof(int)));
  *bothleft = static_cast<int*>(calloc(array_len, sizeof(int)));
  *middle = static_cast<int*>(calloc(array_len, sizeof(int)));
  *bothright = static_cast<int*>(calloc(array_len, sizeof(int)));
  *right = static_cast<int*>(calloc(array_len, sizeof(int)));
  
  // do the computations
  for(size_t ii(0); ii < param.size(); ++ii){
    for(size_t jj(0); jj < (*pos_len); ++jj){
      double lambda_r( (*pos)[jj] - param[ii].rpos);
      if(lambda_r < 0) lambda_r = - lambda_r;
      lambda_r -= param[ii].rrad;
      if(lambda_r < 0) lambda_r = 0;
      double lambda_i( (*pos)[jj] - param[ii].opos);
      if(lambda_i > 0){
	if(lambda_i <= param[ii].orad)
	  lambda_i = 0;
	else
	  lambda_i -= param[ii].orad;
      }
      else{
	if(lambda_i >= - param[ii].orad)
	  lambda_i = 0;
	else
	lambda_i += param[ii].orad;
      }
      int foo;
      const size_t kk(ii + jj * param.size());
      (*cooc)[kk] = pnf_cooc_detail(lambda_i, lambda_r,
				    param[ii].vobj, param[ii].vrob, s,
				    &((*pleft)[kk]), &((*pbothleft)[kk]),
				    &((*pmiddle)[kk]), &((*pbothright)[kk]),
				    &((*pright)[kk]), &((*left)[kk]),
				    &((*bothleft)[kk]), &((*middle)[kk]),
				    &((*bothright)[kk]), &((*right)[kk]),
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


static void dump_plot(FILE *stream,
		      const string & title,
		      const lwpp_param_t & param,
		      int dataset)
{
  if(param.empty())
    return;
  fprintf(stream,
	  "set title '%s'\n"
	  "plot 'test_cooc.data' i %d u 1:2 w l t '%s'",
	  title.c_str(), dataset, param[0].msg.c_str());
  for(size_t jj(1); jj < param.size(); ++jj)
    fprintf(stream, ", 'test_cooc.data' i %d u 1:%zu w l t '%s'",
	    dataset, jj + 2, param[jj].msg.c_str());
  fprintf(stream, "\nset mouse\npause -1 \"hit return to quit\"\n");
}


void dump_plot_fig(FILE *stream,
		   const string & title,
		   const lwpp_param_t & param,
		   int dataset)
{
  if(param.empty())
    return;
  fprintf(stream,
	  "gnuplot -persist <<EOF\n"
	  "set terminal fig\n"
	  "set output 'test_cooc.fig'\n"
	  "set title '%s'\n"
	  "plot 'test_cooc.data' i %d u 1:2 w l t '%s'",
	  title.c_str(), dataset, param[0].msg.c_str());
  for(size_t jj(1); jj < param.size(); ++jj)
    fprintf(stream, ", 'test_cooc.data' i %d u 1:%zu w l t '%s'",
	    dataset, jj + 2, param[jj].msg.c_str());
  fprintf(stream, "\nEOF\n");
}


void test_normal(double start_pos, double end_pos, double resolution,
		 const lwpp_param_t & param)
{
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
  
  size_t position_len;
  double * co_occurrence, * position;
  double * pleft, * pbothleft, * pmiddle, * pbothright, * pright;
  int * left, * bothleft, * middle, * bothright, * right;
  lwpp(param, start_pos, end_pos, resolution,
       &co_occurrence, &position, &position_len,
       &pleft, &pbothleft, &pmiddle, &pbothright, &pright,
       &left, &bothleft, &middle, &bothright, &right);

  fprintf(datastream,
	  "### IN\n"
	  "# rpos rrad opos orad vrob vobj\n");
  for(size_t ii(0); ii < param.size(); ++ii)
    fprintf(datastream, "# %g %g %g %g %g %g\n",
	    param[ii].rpos, param[ii].rrad, param[ii].opos,
	    param[ii].orad, param[ii].vrob, param[ii].vobj);
  fprintf(datastream,  
	  "# start_pos == %g\n"
	  "# end_pos == %g\n"
	  "# resolution == %g\n"
	  "### OUT\n"
	  "# position_len == %zd\n",
	  start_pos, end_pos, resolution, position_len);
  
  dump_double(datastream,
	      "\n\n# pos\tco_occurrence[ipos][iobj]:\n",
	      position, co_occurrence, position_len, param.size());
  dump_double(datastream,
	      "\n\n# pos\tpleft[ipos][iobj]:\n",
	      position, pleft, position_len, param.size());
  dump_double(datastream,
	      "\n\n# pos\tpbothleft[ipos][iobj]:\n",
	      position, pbothleft, position_len, param.size());
  dump_double(datastream,
	      "\n\n# pos\tpmiddle[ipos][iobj]:\n",
	      position, pmiddle, position_len, param.size());
  dump_double(datastream,
	      "\n\n# pos\tpbothright[ipos][iobj]:\n",
	      position, pbothright, position_len, param.size());
  dump_double(datastream,
	      "\n\n# pos\tpright[ipos][iobj]:\n",
	      position, pright, position_len, param.size());
  dump_int(datastream,
	   "\n\n# pos\tleft[ipos][iobj]:\n",
	   position, left, position_len, param.size());
  dump_int(datastream,
	   "\n\n# pos\tbothleft[ipos][iobj]:\n",
	   position, bothleft, position_len, param.size());
  dump_int(datastream,
	   "\n\n# pos\tmiddle[ipos][iobj]:\n",
	   position, middle, position_len, param.size());
  dump_int(datastream,
	   "\n\n# pos\tbothright[ipos][iobj]:\n",
	   position, bothright, position_len, param.size());
  dump_int(datastream,
	   "\n\n# pos\tright[ipos][iobj]:\n",
	   position, right, position_len, param.size());
  dump_plot(plotstream, "Co-occurrence Estimation", param, 0);
  dump_plot_fig(plotfigstream, "Co-occurrence Estimation", param, 0);
  dump_plot(plotallstream, "Co-occurrence Estimation", param, 0);
  dump_plot(plotallstream, "P left",         param, 1);
  dump_plot(plotallstream, "P bothleft",     param, 2);
  dump_plot(plotallstream, "P middle",       param, 3);
  dump_plot(plotallstream, "P bothright",    param, 4);
  dump_plot(plotallstream, "P right",        param, 5);
  //   dump_plot(plotallstream, "left",          param, 6);
  //   dump_plot(plotallstream, "bothleft",      param, 7);
  //   dump_plot(plotallstream, "middle",        param, 8);
  //   dump_plot(plotallstream, "bothright",     param, 9);
  //   dump_plot(plotallstream, "right",         param, 10);
  
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
  
  printf("gnuplot test_cooc.plot\n");
  printf("sh test_cooc.plotall\n");
  printf("sh test_cooc.plotfig\n");
}


void test_alt(double start_pos, double end_pos, double resolution,
	      const lwpp_param_t & param)
{
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
  FILE *plotfigstream = fopen("test_cooc.plotfig", "w");
  if(NULL == plotfigstream){
    perror("test_cooc.plotfig");
    fclose(datastream);
    fclose(plotstream);
    exit(EXIT_FAILURE);
  }
  
  const size_t
    poslen(static_cast<size_t>(ceil((end_pos-start_pos)/resolution)));
  double pos[poslen];
  for(size_t ii(0); ii < poslen; ii++)
    pos[ii] = start_pos + (ii + 0.5) * resolution;
  const size_t alen(param.size() * poslen);
  double cooc[alen];
  for(size_t ii(0); ii < param.size(); ++ii){
    for(size_t jj(0); jj < poslen; ++jj){
      const double
	lambda_r(maxval(0.0, absval(pos[jj]-param[ii].rpos) - param[ii].rrad));
      double lambda_i(pos[jj] - param[ii].opos);
      if(lambda_i > 0){
	if(lambda_i <= param[ii].orad)
	  lambda_i = 0;
	else
	  lambda_i -= param[ii].orad;
      }
      else{
	if(lambda_i >= - param[ii].orad)
	  lambda_i = 0;
	else
	  lambda_i += param[ii].orad;
      }
      const size_t kk(ii + jj * param.size());
      const unsigned int n_v_i_steps(100);
      cooc[kk] = pnf_cooc_test_alt(lambda_i, lambda_r,
				   param[ii].vobj, param[ii].vrob,
				   resolution, n_v_i_steps);
    }
  }
  
  fprintf(datastream,
	  "### IN\n"
	  "# rpos rrad opos orad vrob vobj\n");
  for(size_t ii(0); ii < param.size(); ++ii)
    fprintf(datastream, "# %g %g %g %g %g %g\n",
	    param[ii].rpos, param[ii].rrad, param[ii].opos,
	    param[ii].orad, param[ii].vrob, param[ii].vobj);
  fprintf(datastream,  
	  "# start_pos == %g\n"
	  "# end_pos == %g\n"
	  "# resolution == %g\n"
	  "### OUT\n"
	  "# position_len == %zu\n",
	  start_pos, end_pos, resolution, poslen);
  
  dump_double(datastream,
	      "\n\n# pos\tco_occurrence[ipos][iobj]:\n",
	      pos, cooc, poslen, param.size());
  dump_plot(plotstream, "Co-occurrence Estimation", param, 0);
  dump_plot_fig(plotfigstream, "Co-occurrence Estimation", param, 0);

  fclose(datastream);
  fclose(plotstream);
  fclose(plotfigstream);

  printf("gnuplot test_cooc.plot\n");
  printf("sh test_cooc.plotfig\n");
}
