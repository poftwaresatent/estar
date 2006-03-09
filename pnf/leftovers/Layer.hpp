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


#ifndef LAYER_HPP
#define LAYER_HPP


#include <estar/base.hpp>
#include <boost/scoped_ptr.hpp>
#include <map>


namespace estar {
  class Grid;
  class Algorithm;
  class LSMKernel;
}


namespace pnf {
  
  
  class Layer
  {
  public:
    Layer(size_t xsize, size_t ysize);
    
    
    void SetMeta(size_t ix, size_t iy, double meta);
    double GetMeta(size_t ix, size_t iy) const;
    
    double GetValue(size_t ix, size_t iy) const;
    
    void AddGoal(size_t ix, size_t iy);
    void RemoveGoal(size_t ix, size_t iy);
    bool IsGoal(size_t ix, size_t iy) const;
    
    bool HaveWork() const;
    void ComputeOne();
    
    
  private:
    boost::scoped_ptr<estar::Algorithm> m_algo;
    boost::scoped_ptr<estar::Grid> m_grid;
    boost::scoped_ptr<estar::LSMKernel> m_kernel;
    const estar::value_map_t & m_value_map;
    const estar::meta_map_t & m_meta_map;
    
  public:			// initialized after m_kernel
    const double freespace_meta;
    const double obstacle_meta;
  };
  
}

#endif // LAYER_HPP
