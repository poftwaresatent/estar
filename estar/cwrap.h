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


#ifndef ESTAR_CWRAP_H
#define ESTAR_CWRAP_H


#ifdef __cplusplus
extern "C" {
#else
#endif // __cplusplus


#include <stdio.h>


  /** \return
      <ul><li> -1: invalid kernel_name </li>
          <li> otherwise: handle to be used in other calls</li></ul> */
  int estar_create(const char * kernel_name,
		   unsigned int xsize,
		   unsigned int ysize,
		   double scale,
		   int connect_diagonal,
		   FILE * dbgstream);

  void estar_destroy(int handle);
  
  /** \return
      <ul><li> -1: invalid handle </li>
          <li>  0: success </li></ul> */
  int estar_get_freespace_meta(int handle,
			       double * freespace_meta);
  
  /** \return
      <ul><li> -1: invalid handle </li>
          <li>  0: success </li></ul> */
  int estar_get_obstacle_meta(int handle,
			      double * obstacle_meta);
  
  /** \return
      <ul><li> -1: invalid handle </li>
          <li>  0: success </li></ul> */
  int estar_get_value(int handle,
		      unsigned int ix,
		      unsigned int iy,
		      double * value);
  
  /** \return
      <ul><li> -1: invalid handle </li>
          <li>  0: success </li></ul> */
  int estar_get_meta(int handle,
		     unsigned int ix,
		     unsigned int iy,
		     double * meta);
  
  /** \return
      <ul><li> -1: invalid handle </li>
          <li>  0: success </li></ul> */
  int estar_set_meta(int handle,
		     unsigned int ix,
		     unsigned int iy,
		     double meta);
  
  /** \return
      <ul><li> -1: invalid handle </li>
          <li>  0: success </li></ul> */
  int estar_add_goal(int handle,
		     unsigned int ix,
		     unsigned int iy,
		     double value);
  
//   /** \return
//       <ul><li> -1: invalid handle </li>
//           <li>  0: success </li></ul> */
//   int estar_remove_goal(int handle,
// 			unsigned int ix,
// 			unsigned int iy);
  
  /** \return
      <ul><li> -1: invalid handle </li>
          <li>  0: (ix, iy) is NOT a goal </li>
          <li>  1: (ix, iy) is a goal </li></ul> */
  int estar_is_goal(int handle,
		    unsigned int ix,
		    unsigned int iy);
  
  /** \return
      <ul><li> -1: invalid handle </li>
          <li>  0: NO work pending, ie empty queue </li>
          <li>  1: non-empty queue </li></ul> */
  int estar_have_work(int handle);
  
  /** \return
      <ul><li> -1: invalid handle </li>
          <li>  0: success </li></ul> */
  int estar_compute_one(int handle, double slack);
  
  /** \return
      <ul><li> -1: invalid handle </li>
          <li>  0: success </li></ul> */
  int estar_dump_grid(int handle,
		      FILE * stream);
  
  /** \return
      <ul><li> -1: invalid handle </li>
          <li>  0: success </li></ul> */
  int estar_dump_queue(int handle,
		       FILE * stream,
		       size_t limit);
  
  
#ifdef __cplusplus
}
#endif // __cplusplus


#endif // ESTAR_CWRAP_H
