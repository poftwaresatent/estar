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

#ifdef WIN32
# include <estar/win32.hpp>
#endif // WIN32


namespace gfx {


  template<typename T>
  struct point {
    point(T _x, T _y): x(_x), y(_y) {}
    T x, y;
  };
  
  template<typename T>
  struct bbox {
    bbox(T _x0, T _y0, T _x1, T _y1):	x0(_x0), y0(_y0), x1(_x1), y1(_y1) {}
    
    friend std::ostream & operator << (std::ostream & os, const bbox & b) {
      return os << "(" << b.x0 << ", " << b.y0
		<< ", " << b.x1 << ", " << b.y1 << ")";
    }
    
    T x0, y0, x1, y1;
  };
  
  typedef point<double> logical_point_t;
  typedef point<int> screen_point_t;
  typedef bbox<double> logical_bbox_t;
  typedef bbox<int> screen_bbox_t;
  
  
  class Basewindow
  {
  public:
    explicit Basewindow(logical_bbox_t lbbox);
    
    virtual ~Basewindow() {}
    
    /**
       Inform the Basewindow that the application window size has
       changed. Note that the winsize parameter is the new size of the
       WHOLE window, not just the portion occupied by the Basewindow.

       At the end of the resize operation, the virtual PostResize()
       method is called to allow subclasses to intercept resizing
       events.
    */
    void Resize(screen_point_t winsize);
    
    /**
       Reposition the window within the application window. The
       bounding box of the Basewindow is given in logical coordinates,
       where (0, 0) is the lower-left and (1, 1) the upper-right
       corner of the WHOLE application window.
    */
    void Reposition(logical_bbox_t lbbox);
    
    bool ContainsLogicalPoint(logical_point_t lpoint) const;
    bool ContainsScreenPoint(screen_point_t spoint) const;
    
    logical_point_t Screen2Logical(screen_point_t pixel) const;
    logical_bbox_t Screen2Logical(screen_bbox_t sbbox) const;
    
    screen_point_t Logical2Screen(logical_point_t lpoint) const;
    screen_bbox_t Logical2Screen(logical_bbox_t lbbox) const;
    
    const logical_bbox_t & Lbbox() const { return m_lbbox; }
    const logical_point_t & Lsize() const { return m_lsize; }
    const screen_bbox_t & Sbbox() const { return m_sbbox; }
    const screen_point_t & Ssize() const { return m_ssize; }
    const screen_point_t & Winsize() const { return m_winsize; }
    
  protected:
    virtual void PostResize() = 0;
    
  private:
    logical_bbox_t m_lbbox;
    logical_point_t m_lsize;
    screen_bbox_t m_sbbox;
    screen_point_t m_ssize;
    screen_point_t m_winsize;
  };
  
  
  class Rootwindow;
  
  
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
    : public Basewindow
  {
  public:
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
    
    /** Called via DispatchClick() afer inversion of the Y axis. */
    virtual void Click(int button, int state, screen_point_t mouse) = 0;
    
    /** Called via DispatchDrag() afer inversion of the Y axis. */
    virtual void Drag(screen_point_t mouse) = 0;
    
    void Enable();
    void Disable();

    inline const std::string & Name() const;
    inline bool Enabled() const;
    
    
  private:
    bool m_enabled;
    bool const m_clickable;
    std::string m_name;
  };
  
  
  class Rootwindow
    : public Basewindow
  {
  public:
    Rootwindow();
    
    void Attach(Subwindow * win);
    bool Detach(Subwindow * win);
    
    Subwindow * GetSubwindow(logical_point_t lpoint);
    Subwindow * GetSubwindow(screen_point_t spoint);

    void DispatchUpdate();
    void DispatchResize(screen_point_t winsize);
    void DispatchDrag(screen_point_t mouse);
    void DispatchClick(int button, int state, screen_point_t mouse);

    void InvertYaxis(screen_point_t & spoint) const;
    void InvertYaxis(screen_bbox_t & sbbox) const;
    void InvertYaxis(logical_point_t & lpoint) const;
    void InvertYaxis(logical_bbox_t & lbbox) const;
    
  protected:
    virtual void PostResize() {}
    
  private:
    typedef std::set<Subwindow *> registry_t;
    
    registry_t m_registry;
    Subwindow * m_lastclicked;
  };
  
  
  const std::string & Subwindow::
  Name()
    const 
  {
    return m_name;
  }
  
  
  bool Subwindow::
  Enabled()
    const 
  {
    return m_enabled;
  }

}

#endif // GFX_SUBWINDOW_HPP
