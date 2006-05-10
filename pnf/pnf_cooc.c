/* 
 * Copyright (C) 2003
 * Swiss Federal Institute of Technology, Lausanne. All rights reserved.
 * 
 * Authors: Bjoern Jensen <bjoern dot jensen at singleton-technology dot com>
 *          Roland Philippsen <roland dot philippsen at gmx dot net>
 *          Autonomous Systems Lab <http://asl.epfl.ch/>
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
#include <math.h>
#include <float.h>

// under gcc (GCC) 3.4.4 20050721 (Red Hat 3.4.4-2) and maybe
// elsewhere it happens that INFINITY is not defined after the
// inclusion of math.h, so we try something else here...
#ifndef INFINITY
# define INFINITY DBL_MAX
#endif // INFINITY


/**
   \todo <code>(N-1) / N</code> can be cached. <code>N</code> should
   use <code>ceil()</code> instead of the flakey <code>+0.5</code>. 
*/
double pnf_cooc_detail(double lambda_i,
		       double lambda_r,
		       double v_i,
		       double v_r,
		       double delta,
		       double * pleft,
		       double * pbothleft,
		       double * pmiddle,
		       double * pbothright,
		       double * pright,
		       int * left,
		       int * bothleft,
		       int * middle,
		       int * bothright,
		       int * right,
		       int * boundguard)
{
  double t, N, v1, v2, cooc;
  
  if(lambda_r < 0) lambda_r = - lambda_r;
  if(v_i < 0) v_i = - v_i;
  if(v_r < 0) v_r = - v_r;
  if(delta < 0) delta = - delta;
  
  if((lambda_r <= DBL_EPSILON) || (lambda_r >= INFINITY) || isnan(lambda_r)
     || (v_i   <= DBL_EPSILON) || (v_i      >= INFINITY) || isnan(v_i)
     || (v_r   <= DBL_EPSILON) || (v_r      >= INFINITY) || isnan(v_r)
     || (delta <= DBL_EPSILON) || (delta    >= INFINITY) || isnan(delta)
     || ((lambda_i >= 0)
	 && ((lambda_i <=   DBL_EPSILON) || (lambda_i >=   INFINITY)))
     || ((lambda_i < 0)
	 && ((lambda_i >= - DBL_EPSILON) || (lambda_i <= - INFINITY)))
     || isnan(lambda_i)){
    (*boundguard) = 1;
    return 0;
  }
  
  t = lambda_r / v_r;
  N = t / (delta / v_r) + 0.5;
  v1 = (lambda_i - delta / 2) / t;
  v2 = (lambda_i + delta / 2) / t;
  
  (*pleft) =
    (v1 + v_i) / v_i
    + (N-1) / N * (v1 * v1 - v_i * v_i) / (2*v_i * v_i);
  (*pbothleft) =
    (v2 - v1) / v_i
    + (N-1) / N * (v2 * v2 - v1 * v1) / (2*v_i * v_i);
  (*pmiddle) =
    (v2 - v1) / v_i
    - (N-1) / N * (v2 * v2 + v1 * v1) / (2*v_i * v_i);
  (*pbothright) =
    (v2 - v1) / v_i
    - (N-1) / N * (v2 * v2 - v1 * v1) / (2*v_i * v_i);
  (*pright) =
    (v_i - v2) / v_i
    - (N-1) / N * (v_i * v_i - v2 * v2) / (2*v_i * v_i);
  
  if((v1 < -v_i) && (-v_i < v2) && (v2 < 0))
    (*left) = 1;
  else
    (*left) = 0;
  
  if((-v_i <= v1) && (v1 < 0) && (-v_i <= v2) && (v2 < 0))
    (*bothleft) = 1;
  else
    (*bothleft) = 0;

  if((-v_i <= v1) && (v1 < 0) && (0 <= v2) && (v2 < v_i))
    (*middle) = 1;
  else
    (*middle) = 0;

  if((0 <= v1) && (v1 < v_i) && (0 <= v2) && (v2 < v_i))
    (*bothright) = 1;
  else
    (*bothright) = 0;

  if((0 <= v1) && (v1 < v_i) && (v_i <= v2))
    (*right) = 1;
  else
    (*right) = 0;
  
  cooc
    = (*left) * (*pleft)
    + (*bothleft) * (*pbothleft)
    + (*middle) * (*pmiddle)
    + (*bothright) * (*pbothright)
    + (*right) * (*pright);
  
  if(cooc < 0){		    // this has actually been observed (cooc=-1e-5)
    (*boundguard) = 1;
    return 0;
  }
  if(cooc > 1){
    (*boundguard) = 1;
    return 1;
  }
  if(isnan(cooc)){ // this has actually been observed (*pleft=inf)
    (*boundguard) = 1;
    return 0;
  }
  (*boundguard) = 0;
  return cooc;
}


double pnf_cooc(double lambda_i,
		double lambda_r,
		double v_i,
		double v_r,
		double delta)
{
  double pleft, pbothleft, pmiddle, pbothright, pright;
  int left, bothleft, middle, bothright, right, boundguard;
  return
    pnf_cooc_detail(lambda_i, lambda_r, v_i, v_r, delta,
		    &pleft, &pbothleft, &pmiddle, &pbothright, &pright,
		    &left, &bothleft, &middle, &bothright, &right,
		    &boundguard);
}
