/* 
 * Copyright (C) 2004
 * Swiss Federal Institute of Technology, Lausanne. All rights reserved.
 * 
 * Author: Roland Philippsen <roland dot philippsen at gmx net>
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


#include "PNFRiskMap.hpp"
#include <cmath>


namespace pnf {
  
  
  PNFRiskMap::
  PNFRiskMap(const std::string & _name,
	     double _cutoff, double _degree)
    : name(_name),
      cutoff(_cutoff),
      degree(_degree)
  {
  }
  
  
  PNFRiskMap * PNFRiskMap::
  Create(const std::string & name,
	 double cutoff, double degree)
  {
    if(name == "spike")
      return new Spike(cutoff, degree);
    if(name == "blunt")
      return new Blunt(cutoff, degree);
    if(name == "sigma")
      return new Sigma(cutoff, degree);
    return 0;
  }
  
  
  Spike::
  Spike(double cutoff, double degree)
    : PNFRiskMap("spike", cutoff, degree)
  {
  }
  
  
  double Spike::
  RiskToMeta(double risk) const
  {
    if(risk >= cutoff)
      return 0;
    return pow(1 - risk / cutoff, degree);
  }
  
  
  double Spike::
  MetaToRisk(double meta) const
  {
    if(meta <= 0)
      return cutoff;
    return cutoff * (1 - pow(meta, 1 / degree));
  }
  
  
  Blunt::
  Blunt(double cutoff, double degree)
    : PNFRiskMap("blunt", cutoff, degree)
  {
  }
  
  
  double Blunt::
  RiskToMeta(double risk) const
  {
    if(risk >= cutoff)
      return 0;
    return 1 - pow(risk / cutoff, degree);
  }
  
  
  double Blunt::
  MetaToRisk(double meta) const
  {
    if(meta <= 0)
      return cutoff;
    return cutoff * pow(1 - meta, 1 / degree);
  }
  
  
  Sigma::
  Sigma(double cutoff, double degree)
    : PNFRiskMap("sigma", cutoff, degree)
  {
  }
  
  
  double Sigma::
  RiskToMeta(double risk) const
  {
    if(risk >= cutoff)
      return 0;
    if(risk <= (1 - cutoff))
      return 1;
    const double rhs(0.5 * pow(1 - fabs(risk - 0.5) / (cutoff - 0.5), degree));
    if(risk >= 0.5)
      return rhs;
    return 1 - rhs;
  }
  
  
  double Sigma::
  MetaToRisk(double meta) const
  {
    if(meta <= 0)
      return cutoff;
    if(meta >= 1)
      return 1 - cutoff;
    if(meta >= 0.5)
      return 0.5 + (cutoff - 0.5) * ( 1 - pow(2 * meta, 1 / degree) );
    meta = 1 - meta;
    return 0.5 + (cutoff - 0.5) * ( pow(2 * meta, 1 / degree) - 1 );
  }
  
}
