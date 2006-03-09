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


#ifndef GFX_MOUSEHANDLER_HPP
#define GFX_MOUSEHANDLER_HPP


namespace gfx {
  
  
  /** Generic mouse click handling (per Subwindow). */
  class Mousehandler
  {
  public:
    virtual ~Mousehandler() {}
    virtual void HandleClick(double x, double y) = 0;
  };
  
}

#endif // GFX_MOUSEHANDLER_HPP
