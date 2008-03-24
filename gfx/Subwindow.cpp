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


#include "Subwindow.hpp"
#include "wrap_glut.hpp"
#include "../estar/pdebug.hpp"
#include <iostream>


using namespace std;


static gfx::Rootwindow * Root() {
  static gfx::Rootwindow root;
  return & root;
}


namespace gfx {


  Basewindow::
  Basewindow(logical_bbox_t lbbox)
    : m_lbbox(lbbox),
      m_lsize(lbbox.x1 - lbbox.x0, lbbox.y1 - lbbox.y0),
      m_sbbox(0, 0, 1, 1),
      m_ssize(1, 1),
      m_winsize(1, 1)
  {
  }
  
  
  Subwindow::
  Subwindow(const string & name,
	    logical_bbox_t lbbox,
	    bool enable,
	    bool clickable)
    : Basewindow(lbbox),
      m_enabled(enable),
      m_clickable(clickable),
      m_name(name)
  {
    PVDEBUG("%p\n  n: %s  x0: %g  y0: %g  x1: %g  y1: %g  en: %s  cl: %s\n",
	    this, name.c_str(), lbbox.x0, lbbox.y0, lbbox.x1, lbbox.y1,
	    (enable ? "true" : "false"),
	    (clickable ? "true" : "false") );
    if (clickable)
      Root()->Attach(this);
  }


  Subwindow::
  ~Subwindow()
  {
    Root()->Detach(this);
  }


  Rootwindow::
  Rootwindow()
    : Basewindow(logical_bbox_t(0, 0, 1, 1)),
      m_lastclicked(0)
  {
  }
  
  
  Subwindow * Subwindow::
  GetSubwindow(logical_point_t lpoint)
  {
    return Root()->GetSubwindow(lpoint);
  }
  
  
  Subwindow * Rootwindow::
  GetSubwindow(logical_point_t lpoint)
  {
    PVDEBUG("nsubwins: %i  x: %g\ty: %g\n",
	    m_registry.size(), lpoint.x, lpoint.y);
    
    for(registry_t::iterator is(m_registry.begin());
	is != m_registry.end(); ++is)
      if((*is)->Enabled() && (*is)->ContainsLogicalPoint(lpoint))
	return *is;
    return 0;
  }


  Subwindow * Subwindow::
  GetSubwindow(screen_point_t spoint)
  {
    return Root()->GetSubwindow(spoint);
  }
  
  
  Subwindow * Rootwindow::
  GetSubwindow(screen_point_t spoint)
  {
    PVDEBUG("nsubwins: %i  x: %d\ty: %d\n",
	    m_registry.size(), spoint.x, spoint.y);
    
    for(registry_t::iterator is(m_registry.begin());
	is != m_registry.end(); ++is)
      if((*is)->Enabled() && (*is)->ContainsScreenPoint(spoint))
	return *is;
    return 0;
  }
  
  
  void Subwindow::
  DispatchUpdate()
  {
    Root()->DispatchUpdate();
  }
  
  
  void Rootwindow::
  DispatchUpdate()
  {
    for(registry_t::iterator is(m_registry.begin());
	is != m_registry.end(); ++is)
      if((*is)->Enabled())
	(*is)->Update();
  }
  
  
  void Subwindow::
  DispatchResize(screen_point_t winsize)
  {
    Root()->DispatchResize(winsize);
  }
  
  
  void Rootwindow::
  DispatchResize(screen_point_t winsize)
  {
    Resize(winsize);
    for(registry_t::iterator is(m_registry.begin());
	is != m_registry.end(); ++is)
      if((*is)->Enabled())
	(*is)->Resize(winsize);
  }
  
  
  void Subwindow::
  DispatchClick(int button,
		int state,
		screen_point_t mouse)
  {
    Root()->DispatchClick(button, state, mouse);
  }
  
  
  void Rootwindow::
  DispatchClick(int button,
		int state,
		screen_point_t mouse)
  {
    PVDEBUG("b: %i  s: %i  mx: %i  my %i\n",
	    button, state, mouse.x, mouse.y);
    
    InvertYaxis(mouse);
    Subwindow * sw(GetSubwindow(mouse));
    PVDEBUG("  subwindow: %p\n", sw);
    
    if (sw == 0) {
      m_lastclicked = 0;
      return;
    }
    
    sw->Click(button, state, mouse);
    
    if (state == GLUT_DOWN)
      m_lastclicked = sw;
  }
  
  
  void Subwindow::
  DispatchDrag(screen_point_t mouse)
  {
    Root()->DispatchDrag(mouse);
  }
  
  
  void Rootwindow::
  DispatchDrag(screen_point_t mouse)
  {
    if (m_lastclicked == 0) {
      return;
    }
    
    InvertYaxis(mouse);
    m_lastclicked->Drag(mouse);
  }
  
  
  void Subwindow::
  Update()
  {
  }


  void Basewindow::
  Resize(screen_point_t winsize)
  {
    if((winsize.x == m_winsize.x) &&
       (winsize.y == m_winsize.y))
      return;

    m_winsize = winsize;
    m_sbbox = Logical2Screen(m_lbbox);
    m_ssize.x = m_sbbox.x1 - m_sbbox.x0;
    m_ssize.y = m_sbbox.y1 - m_sbbox.y0;
  
    PostResize();
  }


  void Basewindow::
  Reposition(logical_bbox_t lbbox)
  {
    if((lbbox.x0 == m_lbbox.x0) &&
       (lbbox.y0 == m_lbbox.y0) &&
       (lbbox.x1 == m_lbbox.x1) &&
       (lbbox.y1 == m_lbbox.y1))
      return;

    m_lbbox = lbbox;
    m_lsize.x = lbbox.x1 - lbbox.x0;
    m_lsize.y = lbbox.y1 - lbbox.y0;
    m_sbbox = Logical2Screen(lbbox);
    m_ssize.x = m_sbbox.x1 - m_sbbox.x0;
    m_ssize.y = m_sbbox.y1 - m_sbbox.y0;
  
    PostResize();
  }
  
  
  void Subwindow::
  Enable()
  {
    m_enabled = true;
    Resize(Root()->Winsize());
  }


  void Subwindow::
  Disable()
  {
    m_enabled = false;
  }
  
  
  void Rootwindow::
  InvertYaxis(screen_point_t & spoint) const 
  {
    spoint.y = Winsize().y - spoint.y;
  }


  void Rootwindow::
  InvertYaxis(screen_bbox_t & sbbox) const 
  {
    int y0bak(sbbox.y0);
    sbbox.y0 = Winsize().y - sbbox.y1;
    sbbox.y1 = Winsize().y - y0bak;
  }


  void Rootwindow::
  InvertYaxis(logical_point_t & lpoint) const 
  {
    lpoint.y = 1 - lpoint.y;
  }


  void Rootwindow::
  InvertYaxis(logical_bbox_t & lbbox) const
  {
    double y0bak(lbbox.y0);
    lbbox.y0 = 1 - lbbox.y1;
    lbbox.y1 = 1 - y0bak;
  }

  
  bool Basewindow::
  ContainsLogicalPoint(logical_point_t lpoint) const 
  {
    return
      (lpoint.x >= m_lbbox.x0) &&
      (lpoint.y >= m_lbbox.y0) &&
      (lpoint.x <= m_lbbox.x1) &&
      (lpoint.y <= m_lbbox.y1);
  }


  bool Basewindow::
  ContainsScreenPoint(screen_point_t spoint) const 
  {
    return
      (spoint.x >= m_sbbox.x0) &&
      (spoint.y >= m_sbbox.y0) &&
      (spoint.x <= m_sbbox.x1) &&
      (spoint.y <= m_sbbox.y1);
  }
  
  
  logical_point_t Basewindow::
  Screen2Logical(screen_point_t pixel) const
  {
    logical_point_t pt(static_cast<double>(pixel.x) / m_winsize.x,
		       static_cast<double>(pixel.y) / m_winsize.y);
    return pt;
  }
  
  
  logical_bbox_t Basewindow::
  Screen2Logical(screen_bbox_t sbbox) const
  {
    logical_bbox_t bb(static_cast<double>(sbbox.x0) / m_winsize.x,
		      static_cast<double>(sbbox.y0) / m_winsize.y,
		      static_cast<double>(sbbox.x1) / m_winsize.x,
		      static_cast<double>(sbbox.y1) / m_winsize.y);
    return bb;
  }
  
  
  screen_point_t Basewindow::
  Logical2Screen(logical_point_t lpoint) const
  {
    screen_point_t pt(static_cast<int>(rint(lpoint.x * m_winsize.x)),
		      static_cast<int>(rint(lpoint.y * m_winsize.y)));
    return pt;
  }
  
  
  screen_bbox_t Basewindow::
  Logical2Screen(logical_bbox_t lbbox) const
  {
    screen_bbox_t bb(static_cast<int>(floor(lbbox.x0 * m_winsize.x)),
		     static_cast<int>(floor(lbbox.y0 * m_winsize.y)),
		     static_cast<int>(ceil( lbbox.x1 * m_winsize.x)),
		     static_cast<int>(ceil( lbbox.y1 * m_winsize.y)));
    return bb;
  }
  
  
  void Rootwindow::
  Attach(Subwindow * win)
  {
    m_registry.insert(win);
  }
  
  
  bool Rootwindow::
  Detach(Subwindow * win)
  {
    registry_t::iterator ir(m_registry.find(win));
    if (ir == m_registry.end())
      return false;
    m_registry.erase(ir);
    return true;
  }

}
