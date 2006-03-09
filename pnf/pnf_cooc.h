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


#ifndef PNF_COOC_H
#define PNF_COOC_H


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
   Co-occurrence probability model based on "Line World Path Planning"
   developed by Bjoern Jensen in 2003 for joint work with Roland
   Philippsen on Probabilistic Navigation Functions at the Autonomous
   Systems Lab, EPFL. Use the pnf_cooc() function if you're not
   interested in the intermediate values
   <code>pleft</code>...<code>pright</code> and
   <code>left</code>...<code>right</code>.
   
   \param lambda_i   - IN:  distance to the moving object
   \param lambda_r   - IN:  distance to the robot
   \param v_i        - IN:  (maximum) speed of the moving object
   \param v_r        - IN:  (maximum) speed of the robot
   \param delta      - IN:  step size (discrete "grid")
   
   \param pleft      - OUT: partial intermediate sum
   \param pbothleft  - OUT: partial intermediate sum
   \param pmiddle    - OUT: partial intermediate sum
   \param pbothright - OUT: partial intermediate sum
   \param pright     - OUT: partial intermediate sum
   
   \param left       - OUT: flag for activation of partial sum
   \param bothleft   - OUT: flag for activation of partial sum
   \param middle     - OUT: flag for activation of partial sum
   \param bothright  - OUT: flag for activation of partial sum
   \param right      - OUT: flag for activation of partial sum

   \param boundguard - OUT: numerical guard activated (workaround)
   
   The co-occurrence is a probabilistic measure of how likely a
   collision can occur, at a point of interest, between the robot and
   a moving object. The point of interest is at distance
   <code>lambda_i</code> from the object and distance
   <code>lambda_r</code>from the robot. The object is assumed to move
   with maximum speed <code>v_i</code>, and the robot with maximum
   speed <code>v_r</code>. The step size <code>delta</code> defines
   the granularity distance with which the moving object can "choose"
   to go either towards or away from the robot, or remain stationary.
   
   \return A value between 0 and 1 (inclusive).
   
   \note Bjoern Jensen, Roland Philippsen, and Roland
   Siegwart. "Motion Detection and Path Planning in Dynamic
   Environments". Workshop Proceedings Reasoning with Uncertainty in
   Robotics, International Joint Conference on Artificial Intelligence
   (IJCAI), 2003.
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
		       int * boundguard);


/**
   The same as pnf_cooc_detail(), but without exposing the internal
   partial probabilities.
   
   \param lambda_i   - IN:  distance to the moving object
   \param lambda_r   - IN:  distance to the robot
   \param v_i        - IN:  (maximum) speed of the moving object
   \param v_r        - IN:  (maximum) speed of the robot
   \param delta      - IN:  step size (discrete "grid")

   \return A value between 0 and 1 (inclusive).
*/
double pnf_cooc(double lambda_i,
		double lambda_r,
		double v_i,
		double v_r,
		double delta);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // PNF_COOC_H
