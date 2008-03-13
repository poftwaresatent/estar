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


#include "Viewport.hpp"
#include "Mousehandler.hpp"
#include "wrap_glut.hpp"
#include <estar/numeric.hpp>
#include <cmath>


using estar::absval;
using estar::minval;
using namespace std;
using namespace boost;


namespace gfx {


  Viewport::
  Viewport(const string & name,
	   logical_bbox_t realbbox,
	   logical_bbox_t lbbox,
	   bool enable,
	   bool preserve_aspect,
	   double remap_thresh):
    Subwindow(name, lbbox, enable),
    m_remap_thresh(remap_thresh),
    m_padded_sbbox(0, 0, 1, 1),
    m_padded_ssize(1, 1),
    m_rbbox(realbbox),
    m_rsize(realbbox.x1 - realbbox.x0, realbbox.y1 - realbbox.y0),
    m_preserve_aspect(preserve_aspect)
  {
    PostResize();
  }


  Viewport * Viewport::
  Clone(const string & newname)
    const
  {
    return new Viewport(newname, m_rbbox, Lbbox(),
			m_preserve_aspect, Enabled(), m_remap_thresh);
  }


  void Viewport::
  PostResize()
  {
    m_padded_sbbox.x0 = Sbbox().x0;
    m_padded_sbbox.y0 = Sbbox().y0;
    m_padded_sbbox.x1 = Sbbox().x1;
    m_padded_sbbox.y1 = Sbbox().y1;
    m_padded_ssize.x = Ssize().x;
    m_padded_ssize.y = Ssize().y;
  
    CalculatePadding();
  }


  void Viewport::
  Remap(logical_bbox_t realbbox)
  {
    if((absval(m_rbbox.x0 - realbbox.x0) < m_remap_thresh) &&
       (absval(m_rbbox.y0 - realbbox.y0) < m_remap_thresh) &&
       (absval(m_rbbox.x1 - realbbox.x1) < m_remap_thresh) &&
       (absval(m_rbbox.y1 - realbbox.y1) < m_remap_thresh))
      return;
  
    m_rbbox = realbbox;
    m_rsize.x = realbbox.x1 - realbbox.x0;
    m_rsize.y = realbbox.y1 - realbbox.y0;
  
    PostResize();
  }


  void Viewport::
  CalculatePadding()
  {
    if( ! m_preserve_aspect)
      return;
  
    if((Ssize().x == 0) || (Ssize().y == 0) ||
       (m_rsize.x == 0) || (m_rsize.y == 0))
      return;

    double screenaspect(static_cast<double>(Ssize().x) / Ssize().y);
    double realaspect(m_rsize.x / m_rsize.y);

    // Note: We calculate the double of the padding, will be halved
    // before applying to bounding box.
    double pad_screenx(0);
    double pad_screeny(0);
    if(screenaspect > realaspect)
      pad_screenx = minval(m_padded_ssize.x - 1, // upper bound
			   Ssize().x - Ssize().y * realaspect);
    else if(screenaspect < realaspect)
      pad_screeny = minval(m_padded_ssize.y - 1, // upper bound
			   Ssize().y - Ssize().x / realaspect);

    m_padded_ssize.x -= pad_screenx;
    m_padded_ssize.y -= pad_screeny;
  
    pad_screenx /= 2;
    pad_screeny /= 2;
    m_padded_sbbox.x0 += pad_screenx;
    m_padded_sbbox.y0 += pad_screeny;
    m_padded_sbbox.x1 -= pad_screenx;
    m_padded_sbbox.y1 -= pad_screeny;
  }


  void Viewport::
  PushProjection()
    const
  {
    glViewport(static_cast<int>(floor(m_padded_sbbox.x0)),
	       static_cast<int>(floor(m_padded_sbbox.y0)),
	       static_cast<int>(floor(m_padded_ssize.x)),
	       static_cast<int>(floor(m_padded_ssize.y)));

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(m_rbbox.x0, m_rbbox.x1, m_rbbox.y0, m_rbbox.y1);
  }


  void Viewport::
  PopProjection()
    const
  {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();  
  }


  void Viewport::
  Click(int button,
	int state,
	screen_point_t mouse)
  {
    if(state == GLUT_DOWN){
      switch(button){
      case GLUT_RIGHT_BUTTON:
	m_lastdown = RIGHT;
	break;
      case GLUT_LEFT_BUTTON:
	m_lastdown = LEFT;
	break;
      case GLUT_MIDDLE_BUTTON:
	m_lastdown = MIDDLE;
	break;
      default:
	m_lastdown = NONE;
      }
      return;
    }
  
    logical_point_t rp(PaddedScreen2Real(mouse));
    if(((button == GLUT_LEFT_BUTTON) && (m_lastdown == LEFT))
       || ((button == GLUT_RIGHT_BUTTON) && (m_lastdown == RIGHT))
       || ((button == GLUT_MIDDLE_BUTTON) && (m_lastdown == MIDDLE))){
      handler_t::iterator ih(m_handler.find(m_lastdown));
      if((ih != m_handler.end()) && (ih->second != 0))
	ih->second->HandleClick(rp.x, rp.y);
    }
    m_lastdown = NONE;
  }


  void Viewport::
  Drag(screen_point_t mouse)
  {
    // do nothing
  }


  PassiveViewport::
  PassiveViewport(const string & name,
		  logical_bbox_t realbbox,
		  logical_bbox_t lbbox,
		  bool preserve_aspect,
		  double remap_thresh):
    Viewport(name,
	     realbbox,
	     lbbox,
	     preserve_aspect,
		 false,
	     remap_thresh)
  {
  }


  void PassiveViewport::
  Click(int button,
	int state,
	screen_point_t mouse)
  {
    // do nothing
  }


  Subwindow::logical_point_t Viewport::
  PaddedScreen2Real(screen_point_t pixel)
    const
  {
    return logical_point_t(m_rbbox.x0 +
			   m_rsize.x *
			   (pixel.x - m_padded_sbbox.x0) / m_padded_ssize.x,
			   m_rbbox.y0 +
			   m_rsize.y *
			   (pixel.y - m_padded_sbbox.y0) / m_padded_ssize.y);
  }


  void Viewport::
  SetMousehandler(button_t button,
		  shared_ptr<Mousehandler> mousehandler)
  {
    handler_t::iterator ih(m_handler.find(button));
    if(ih == m_handler.end())
      m_handler.insert(make_pair(button, mousehandler));
    else
      ih->second = mousehandler;
  }

}
