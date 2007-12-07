/* 
 * Copyright (C) 2007 Roland Philippsen <roland dot philippsen at gmx net>
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


#include "Mousehandler.hpp"
#include "../estar/FacadeReadInterface.hpp"
#include "../estar/FacadeWriteInterface.hpp"
#include "../estar/numeric.hpp"
#include "../estar/pdebug.hpp"


using namespace estar;


namespace gfx {
  
  
  FacadeReadMousehandler::
  FacadeReadMousehandler(fread_ptr facade_read)
    : m_facade_read(facade_read)
  {
  }
  
  
  void FacadeReadMousehandler::
  HandleClick(double x, double y)
  {
    size_t const ix(static_cast<size_t>(rint(x)));
    size_t const iy(static_cast<size_t>(rint(y)));
    if (m_facade_read->IsValidIndex(ix, iy))
      DoHandleClick(ix, iy);
  }
  
  
  FacadeMousehandler::
  FacadeMousehandler(fread_ptr facade_read, fwrite_ptr facade_write)
    : FacadeReadMousehandler(facade_read),
      m_facade_write(facade_write)
  {
  }
  
  
  ObstacleMousehandler::
  ObstacleMousehandler(fread_ptr facade_read, fwrite_ptr facade_write)
    : FacadeMousehandler(facade_read, facade_write)
  {
  }
  
  
  void ObstacleMousehandler::
  DoHandleClick(size_t ix, size_t iy)
  {
    double const obstmeta(m_facade_read->GetObstacleMeta());
    double const meta(m_facade_read->GetMeta(ix, iy));
    if (absval(meta - obstmeta) < epsilon) // it's an obstacle, free it!
      m_facade_write->SetMeta(ix, iy, m_facade_read->GetFreespaceMeta());
    else
      m_facade_write->SetMeta(ix, iy, m_facade_read->GetObstacleMeta());
  }
  
  
  GoalMousehandler::
  GoalMousehandler(fread_ptr facade_read, fwrite_ptr facade_write)
    : FacadeMousehandler(facade_read, facade_write)
  {
  }
  
  
  void GoalMousehandler::
  DoHandleClick(size_t ix, size_t iy)
  {
    m_facade_write->AddGoal(ix, iy, 0);
  }
  
}
