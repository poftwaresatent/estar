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


#ifndef GFX_WRAP_GLU_HPP
# define GFX_WRAP_GLU_HPP

# if defined( LINUX ) | defined( OPENBSD )
#  include <GL/glu.h>
# elif defined( OSX )
#  include <OpenGL/glu.h>
# elif defined( WIN32 )
#  include <windows.h>
#  include <GL/glu.h>
# else
#  error Define LINUX, OPENBSD, OSX, or WIN32 (or extend this header).
# endif

namespace gfx {

  /** Interoperability wrapper for OS X. Using the returned (static)
      GLUquadricObj* directly can lead to "bus errors", but access
      through this function works fine. Bizarre... */
  GLUquadricObj * wrap_glu_quadric_instance();

}

#endif // GFX_WRAP_GLU_HPP
