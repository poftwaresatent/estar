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


#ifndef ESTAR_RISKMAP_HPP
#define ESTAR_RISKMAP_HPP


namespace estar {
  
  
  /** Translate risk to meta information. Depends on the Kernel
      subtype and heuristics that influence how "dangerous" a given
      risk value is interpreted to be. */
  class RiskMap
  {
  public:
    virtual ~ RiskMap() { }
    virtual double RiskToMeta(double risk) const = 0;
    virtual double MetaToRisk(double meta) const = 0;
  };
  
  
  /** Simplistic RiskMap subtype. Always return a fixed risk and meta,
      no matter what. */
  class DummyRiskMap
    : public RiskMap
  {
  public:
    DummyRiskMap(double risk, double meta)
      : dummy_risk(risk), dummy_meta(meta) { }
    virtual double RiskToMeta(double risk) const { return dummy_meta; };
    virtual double MetaToRisk(double meta) const { return dummy_risk; };
    const double dummy_risk, dummy_meta;
  };
  
  
  /** Simplel RiskMap subtype. "Inverts" risk and meta information:
      <code>risk=1-meta <=> meta=1-risk</code> */
  class InvertRiskMap
    : public RiskMap
  {
  public:
    virtual double RiskToMeta(double risk) const { return 1 - risk; };
    virtual double MetaToRisk(double meta) const { return 1 - meta; };
  };
  
}

#endif // ESTAR_RISKMAP_HPP
