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


#ifndef GFX_SUBWINDOW_HPP
#define GFX_SUBWINDOW_HPP


#include <string>
#include <set>
#include <iostream>
#include <cmath>


namespace gfx {


  /**
     A sub-window of the main GLUT window. Maps logical coordinates in
     the range 0...1 to pixels and vice-versa. Mainly for dispatcher
     functionality, see Viewport for more directly useful subclass.
     
     \note (MAYBE ONLY FOR MOUSE?) Screen coordinates (pixels, window
     sizes) are measured with the y-axis "hanging down", but logical
     coordinates (subwindow bounding boxes, logical points) have a
     conventional y-axis. Subclasses need to be aware of this
     discrepancy.
  */
  class Subwindow
  {
  public:
    /** A bounding box, four coordinates. */
    template<typename T>
    class bbox {
    public:
      bbox(T _x0, T _y0, T _x1, T _y1):
	x0(_x0), y0(_y0), x1(_x1), y1(_y1) { }
    
      friend std::ostream & operator << (std::ostream & os, const bbox & b) {
	return os << "(" << b.x0 << ", " << b.y0
		  << ", " << b.x1 << ", " << b.y1 << ")";
      }
    
      T x0, y0, x1, y1;
    };

    /** A point, two coordinates. */
    template<typename T>
    class point {
    public:
      point(T _x, T _y):
	x(_x), y(_y) { }
    
      T x, y;
    };

    typedef bbox<double> logical_bbox_t;
    typedef bbox<int> screen_bbox_t;
    typedef point<double> logical_point_t;
    typedef point<int> screen_point_t;

  
    Subwindow(const std::string & name,
	      logical_bbox_t lbbox,
	      bool enable = true,
	      bool clickable = true);
    virtual ~Subwindow();
  
    static Subwindow * GetSubwindow(logical_point_t lpoint);
    static Subwindow * GetSubwindow(screen_point_t spoint);
    static void DispatchUpdate();
    static void DispatchResize(screen_point_t winsize);
    static void DispatchDrag(screen_point_t mouse);
    static void DispatchClick(int button, int state, screen_point_t mouse);

    virtual void PushProjection() const = 0;
    virtual void PopProjection() const = 0;
  
    /** \note Default implementation is NOP. */
    virtual void Update();

    void Resize(screen_point_t winsize);
    void Reposition(logical_bbox_t lbbox);
    void Enable();
    void Disable();
    inline bool ContainsLogicalPoint(logical_point_t lpoint) const;
    inline bool ContainsScreenPoint(screen_point_t spoint) const;
    inline const std::string & Name() const;
    inline bool Enabled() const;

    inline logical_point_t Screen2Logical(screen_point_t pixel) const;
    inline logical_bbox_t Screen2Logical(screen_bbox_t sbbox) const;
    inline screen_point_t Logical2Screen(logical_point_t lpoint) const;
    inline screen_bbox_t Logical2Screen(logical_bbox_t lbbox) const;

    inline void InvertYaxis(screen_point_t & spoint) const;
    inline void InvertYaxis(screen_bbox_t & sbbox) const;
    inline void InvertYaxis(logical_point_t & lpoint) const;
    inline void InvertYaxis(logical_bbox_t & lbbox) const;

  protected:
    virtual void PostResize() = 0;
    virtual void Click(int button, int state, screen_point_t mouse) = 0;
    virtual void Drag(screen_point_t mouse) = 0;

    inline const logical_bbox_t & Lbbox() const { return m_lbbox; }
    inline const logical_point_t & Lsize() const { return m_lsize; }
    inline const screen_bbox_t & Sbbox() const { return m_sbbox; }
    inline const screen_point_t & Ssize() const { return m_ssize; }
    inline const screen_point_t & Winsize() const { return m_winsize; }


  private:
    typedef std::set<Subwindow *> registry_t;

  
    static Subwindow * Root();


    static registry_t m_registry;
    static Subwindow * m_lastclicked;

    logical_bbox_t m_lbbox;
    logical_point_t m_lsize;
    screen_bbox_t m_sbbox;
    screen_point_t m_ssize;
    screen_point_t m_winsize;
    bool m_enabled;
    bool m_clickable;
    std::string m_name;
  };


  const std::string & Subwindow::
  Name()
    const 
  {
    return m_name;
  }


  bool Subwindow::
  ContainsLogicalPoint(logical_point_t lpoint)
    const 
  {
    return
      (lpoint.x >= m_lbbox.x0) &&
      (lpoint.y >= m_lbbox.y0) &&
      (lpoint.x <= m_lbbox.x1) &&
      (lpoint.y <= m_lbbox.y1);
  }


  bool Subwindow::
  ContainsScreenPoint(screen_point_t spoint)
    const 
  {
    return
      (spoint.x >= m_sbbox.x0) &&
      (spoint.y >= m_sbbox.y0) &&
      (spoint.x <= m_sbbox.x1) &&
      (spoint.y <= m_sbbox.y1);
  }


  bool Subwindow::
  Enabled()
    const 
  {
    return m_enabled;
  }


  Subwindow::logical_point_t Subwindow::
  Screen2Logical(screen_point_t pixel)
    const 
  {
    return logical_point_t(static_cast<double>(pixel.x) / m_winsize.x,
			   static_cast<double>(pixel.y) / m_winsize.y);
  }


  Subwindow::logical_bbox_t Subwindow::
  Screen2Logical(screen_bbox_t sbbox)
    const 
  {
    return logical_bbox_t(static_cast<double>(sbbox.x0) / m_winsize.x,
			  static_cast<double>(sbbox.y0) / m_winsize.y,
			  static_cast<double>(sbbox.x1) / m_winsize.x,
			  static_cast<double>(sbbox.y1) / m_winsize.y);
  }


  Subwindow::screen_point_t Subwindow::
  Logical2Screen(logical_point_t lpoint)
    const 
  {
    return screen_point_t(static_cast<int>(rint(lpoint.x * m_winsize.x)),
			  static_cast<int>(rint(lpoint.y * m_winsize.y)));
  }


  Subwindow::screen_bbox_t Subwindow::
  Logical2Screen(logical_bbox_t lbbox)
    const 
  {
    return screen_bbox_t(static_cast<int>(floor(lbbox.x0 * m_winsize.x)),
			 static_cast<int>(floor(lbbox.y0 * m_winsize.y)),
			 static_cast<int>(ceil( lbbox.x1 * m_winsize.x)),
			 static_cast<int>(ceil( lbbox.y1 * m_winsize.y)));
  }


  void Subwindow::
  InvertYaxis(screen_point_t & spoint)
    const 
  {
    spoint.y = m_winsize.y - spoint.y;
  }


  void Subwindow::
  InvertYaxis(screen_bbox_t & sbbox)
    const 
  {
    int y0bak(sbbox.y0);
    sbbox.y0 = m_winsize.y - sbbox.y1;
    sbbox.y1 = m_winsize.y - y0bak;
  }


  void Subwindow::
  InvertYaxis(logical_point_t & lpoint)
    const 
  {
    lpoint.y = 1 - lpoint.y;
  }


  void Subwindow::
  InvertYaxis(logical_bbox_t & lbbox)
    const
  {
    double y0bak(lbbox.y0);
    lbbox.y0 = 1 - lbbox.y1;
    lbbox.y1 = 1 - y0bak;
  }

}

#endif // GFX_SUBWINDOW_HPP
