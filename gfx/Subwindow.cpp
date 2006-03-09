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
#include <iostream>


using namespace std;


namespace gfx {


  Subwindow::registry_t Subwindow::m_registry;
  Subwindow * Subwindow::m_lastclicked(0);


  Subwindow::
  Subwindow(const string & name,
	    logical_bbox_t lbbox,
	    bool enable):
    m_lbbox(lbbox),
    m_lsize(lbbox.x1 - lbbox.x0, lbbox.y1 - lbbox.y0),
    m_sbbox(0, 0, 1, 1),
    m_ssize(1, 1),
    m_winsize(1, 1),
    m_enabled(enable),
    m_name(name)
  {
    m_registry.insert(this);
  }


  Subwindow::
  ~Subwindow()
  {
    m_registry.erase(this);
  }


  Subwindow * Subwindow::
  GetSubwindow(logical_point_t lpoint)
  {
    for(registry_t::iterator is(m_registry.begin());
	is != m_registry.end();
	++is)
      if((*is)->m_enabled && (*is)->ContainsLogicalPoint(lpoint))
	return *is;
    return 0;
  }


  Subwindow * Subwindow::
  GetSubwindow(screen_point_t spoint)
  {
    for(registry_t::iterator is(m_registry.begin());
	is != m_registry.end();
	++is)
      if((*is)->m_enabled && (*is)->ContainsScreenPoint(spoint))
	return *is;
    return 0;
  }


  void Subwindow::
  DispatchUpdate()
  {
    for(registry_t::iterator is(m_registry.begin());
	is != m_registry.end();
	++is)
      if((*is)->m_enabled)
	(*is)->Update();
  }


  void Subwindow::
  DispatchResize(screen_point_t winsize)
  {
    for(registry_t::iterator is(m_registry.begin());
	is != m_registry.end();
	++is)
      if((*is)->m_enabled)
	(*is)->Resize(winsize);
    Root()->Resize(winsize);
  }


  void Subwindow::
  DispatchClick(int button,
		int state,
		screen_point_t mouse)
  {
    Root()->InvertYaxis(mouse);
    Subwindow * sw(GetSubwindow(mouse));
    if(sw == 0){
      m_lastclicked = 0;
      return;
    }
  
    sw->Click(button, state, mouse);
  
    if(state == GLUT_DOWN)
      m_lastclicked = sw;
  }


  void Subwindow::
  DispatchDrag(screen_point_t mouse)
  {
    if(m_lastclicked == 0){
      return;
    }

    Root()->InvertYaxis(mouse);
    m_lastclicked->Drag(mouse);
  }


  void Subwindow::
  Update()
  {
  }


  void Subwindow::
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


  void Subwindow::
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


  Subwindow * Subwindow::
  Root()
  {
    // NOTE: Takes up some space and time due to m_registry, but stays
    // disabled forever
    class Rootwin: public Subwindow {
    public:
      Rootwin() throw(): Subwindow("__root", logical_bbox_t(0, 0, 1, 1)) {}
      virtual void PushProjection() const throw(){}
      virtual void PopProjection() const throw(){}
    protected:
      virtual void PostResize() throw(){}
      virtual void Click(int button, int state, screen_point_t mouse) throw(){}
      virtual void Drag(screen_point_t mouse) throw(){}
    };

    static Rootwin root;
    return & root;
  }


  void Subwindow::
  Enable()
  {
    m_enabled = true;
    Resize(Root()->m_winsize);
  }


  void Subwindow::
  Disable()
  {
    m_enabled = false;
  }

}
