/* 
 * Copyright (C) 2005,2007 Roland Philippsen <roland dot philippsen at gmx net>
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


#ifndef GFX_MOUSEHANDLER_HPP
#define GFX_MOUSEHANDLER_HPP


#include <boost/shared_ptr.hpp>
#include <stddef.h>


namespace estar {
  class FacadeReadInterface;
  class FacadeWriteInterface;
}


namespace gfx {
  
  
  /** Generic mouse click handling (per Subwindow). */
  class Mousehandler
  {
  public:
    virtual ~Mousehandler() {}
    virtual void HandleClick(double x, double y) = 0;
  };
  
  
  /** Translates the subwindow's (x, y) to facade grid indices. */
  class FacadeReadMousehandler
    : public Mousehandler
  {
  public:
    typedef boost::shared_ptr<estar::FacadeReadInterface const> fread_ptr;
    
    explicit
    FacadeReadMousehandler(fread_ptr facade_read);
    
    /** Translate (x, y) into (ix, iy) and delegate to DoHandleClick(). */
    virtual void HandleClick(double x, double y);
    virtual void DoHandleClick(size_t ix, size_t iy) = 0;
    
  protected:
    fread_ptr m_facade_read;
  };
  
  
  /** A FacadeMousehandler can modify a Facade instance. */
  class FacadeMousehandler
    : public FacadeReadMousehandler
  {
  public:
    typedef boost::shared_ptr<estar::FacadeWriteInterface> fwrite_ptr;
    
    FacadeMousehandler(fread_ptr facade_read, fwrite_ptr facade_write);
    
  protected:
    fwrite_ptr m_facade_write;
  };
  
  
  /** Toggles a cell from freesapce to obstacle and back. */
  class ObstacleMousehandler
    : public FacadeMousehandler
  {
  public:
    ObstacleMousehandler(fread_ptr facade_read, fwrite_ptr facade_write);
    
    virtual void DoHandleClick(size_t ix, size_t iy);
  };
  
  
  /** Adds a goal when clicked. */
  class GoalMousehandler
    : public FacadeMousehandler
  {
  public:
    GoalMousehandler(fread_ptr facade_read, fwrite_ptr facade_write);
    
    virtual void DoHandleClick(size_t ix, size_t iy);
  };
  
}

#endif // GFX_MOUSEHANDLER_HPP
