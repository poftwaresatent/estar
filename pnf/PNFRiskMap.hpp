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


#ifndef PNF_RISK_MAP_HPP
#define PNF_RISK_MAP_HPP


#include <estar/RiskMap.hpp>
#include <string>


namespace pnf {
  
  
  /** A estar::RiskMap for PNF, comes in various flavors.
   
      \todo Move into estar/ subdirectory and find a more generic
      name.
  */
  class PNFRiskMap
    : public estar::RiskMap
  {
  protected:
    PNFRiskMap(const std::string & name, double cutoff, double degree);
    
  public:
    const std::string name;
    const double cutoff;
    const double degree;
    
    static PNFRiskMap * Create(const std::string & name,
			       double cutoff, double degree);
    
    /**
       Compute meta value that corresponds to a given collision
       risk. Result depends on the PNFRiskMap's name.
       
       \pre risk >= 0
       \post 1 >= retval >= 0
    */
    virtual double RiskToMeta(double risk) const = 0;
    
    /** The inverse of RiskToMeta(), with ties (meta = 0) resolved to
	(risk = cutoff). This is not useful with the PNF anyways,
	which always goes from risk to meta, never the other way
	around. */
    virtual double MetaToRisk(double meta) const = 0;
  };
  
}

#endif // PNF_RISK_MAP_HPP
