/* 
 * Copyright (C) 2004
 * Swiss Federal Institute of Technology, Lausanne. All rights reserved.
 * 
 * Author: Roland Philippsen <roland dot philippsen at gmx dot net>
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


#ifndef GETOPT_HPP
#define GETOPT_HPP


#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <iosfwd>
#include <vector>
#include <map>
#include <string>
#include <sstream>


struct option;


namespace util {
  
  
  class BaseCallback
  {
  public:
    BaseCallback(char shortopt,
		 const char * longopt_name,
		 bool requires_arg,
		 const char * description);
    virtual ~BaseCallback();
    virtual bool Do(const char * arg, std::ostream & os) = 0;
    
    const char                m_shortopt;
    const std::string         m_longopt_name;
    const bool                m_requires_arg;
    const std::string         m_description;
    boost::scoped_ptr<option> m_longopt;
  };
  
  
  class Parser
  {
  public:
    Parser()
      : m_longest_longopt(0)
    { }
    
    /** \return true on success, false if there already is a callback
	for this (short) option. */
    bool Add(boost::shared_ptr<BaseCallback> callback);
    
    /** \note just for convenience, do not use if callback is already
	in a boost::shared_ptr<>! */
    bool Add(BaseCallback * callback)
    { return Add(boost::shared_ptr<BaseCallback>(callback)); }
    
    /**
       \return Index into argv of first non-option argument. If no
       arguments remain after parsing, then argc is returned. If
       there's an error, -1 is returned.
    */
    int Do(int argc, char ** argv, std::ostream & os);
    
    void UsageMessage(std::ostream & os);
    
    
    typedef std::map<char, unsigned int> index_map_t;
    
    index_map_t                                   m_index_map;
    std::vector<boost::shared_ptr<BaseCallback> > m_callback;
    std::vector<bool>                             m_present;
    std::vector<const char *>                     m_argument;
    unsigned int                                  m_longest_longopt;
  };
  
  
  template<typename T>
  class Callback
    : public BaseCallback
  {
  public:
    typedef T option_t;
    
    Callback(option_t & option,
	     char shortopt,
	     const char * longopt_name,
	     const char * description)
      : BaseCallback(shortopt, longopt_name, true, description),
	m_option(option)
    { }
    
    virtual bool Do(const char * arg, std::ostream & os) {
      if(arg == 0) {
	os << m_longopt_name << ": argument expected\n";
	return false;
      }
      std::istringstream is(arg);
      is >> m_option;
      if( ! is) {
	os << m_longopt_name << ": invalid argument \"" << arg << "\"\n";
	return false;
      }
      return true;
    }
    
    option_t & m_option;
  };
  
  
  template<>
  class Callback<bool>
    : public BaseCallback
  {
  public:
    typedef bool option_t;
    
    Callback(option_t & option,
	     char shortopt,
	     const char * longopt_name,
	     const char * description)
      : BaseCallback(shortopt, longopt_name, false, description),
	m_option(option)
    { }
    
    virtual bool Do(const char * arg, std::ostream & os) {
      m_option = true;
      return true;
    }
    
    option_t & m_option;
  };
  
}

#endif // GETOPT_H
