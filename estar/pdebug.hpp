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


#ifndef ESTAR_PDEBUG_HPP
#define ESTAR_PDEBUG_HPP

/**
   \file pdebug.hpp Compile-time utilities for adding/removing debug messages.
   
   This file defines a couple of macros and classes/typedefs that make
   it easier to implement switchable debug messages:
   - PDEBUG_ERR() writes a prefixed message to stderr, using fprintf()
   - PDEBUG_OUT() does the same to stdout
   - PDEBUG_OFF() has the same signature, but does nothing
   - PDEBUG() is the same as PDEBUG_OUT() if ESTAR_VERBOSE_DEBUG or
     ESTAR_DEBUG are defined
   - PVDEBUG() is the same as PDEBUG_OUT() if ESTAR_VERBOSE_DEBUG is defined
   - estar::vdebugos is typedefed to std::ostringstream if
     ESTAR_VERBOSE_DEBUG is defined, otherwise it is typedefed to
     estar::fakeos which does nothing.
   - estar::debugos is typedefed to std::ostringstream if
     ESTAR_VERBOSE_DEBUG or ESTAR_DEBUG are defined, otherwise it is
     typedefed to estar::fakeos which does nothing.
     
   So, the way to use this header is simple. Include it, and use
   PDEBUG() to write debug messages and PVDEBUG() to write verbose
   debug messages. In case you want to use something resembling a
   std::ostream, use an instance of estar::debugos or estar::vdebugos,
   but then after filling them with messages you have to explicitly
   print their str().c_str() using PDEBUG() or PVDEBUG().
*/

#include <string>
#include <stdio.h>

#define PDEBUG_ERR(fmt, arg...) fprintf(stderr, "%s(): "fmt, __func__, ## arg)
#define PDEBUG_OUT(fmt, arg...) fprintf(stdout, "%s(): "fmt, __func__, ## arg)
#define PDEBUG_OFF(fmt, arg...)

#ifdef ESTAR_VERBOSE_DEBUG
# define ESTAR_DEBUG
# define PVDEBUG PDEBUG_OUT
#else
# define PVDEBUG PDEBUG_OFF
#endif

#ifdef ESTAR_DEBUG
# include <sstream>
# define PDEBUG PDEBUG_OUT
#else
# define PDEBUG PDEBUG_OFF
#endif
  
namespace estar {
  
  /** Utility for compile-time switchable debug messages. Can be used
      like a std::ostringstream, but doesn't do anything except return
      an empty string if requested to do so. Only makes sense in
      combination with the conditional typedefs debugos and
      vdebugos. */
  class fakeos {
  public:
    template<class Foo>
    fakeos & operator << (const Foo &) { return * this; }
    std::string str() const { return std::string(); }
  };
  
#ifdef ESTAR_VERBOSE_DEBUG
  typedef std::ostringstream vdebugos;
#else
  typedef fakeos vdebugos;
#endif
  
#ifdef ESTAR_DEBUG
  typedef std::ostringstream debugos;
#else
  typedef fakeos debugos;
#endif
  
}

#endif // ESTAR_PDEBUG_HPP
