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


#include "Getopt.hpp"
#include <iostream>
#include <iomanip>
#include <getopt.h>


namespace util {
  
  
  BaseCallback::
  BaseCallback(char shortopt,
	       const char * longopt_name,
	       bool requires_arg,
	       const char * description)
    : m_shortopt(shortopt),
      m_longopt_name(longopt_name),
      m_requires_arg(requires_arg),
      m_description(description),
      m_longopt(new option())
  {
    m_longopt->name = m_longopt_name.c_str();
    m_longopt->has_arg = m_requires_arg ? required_argument : no_argument;
    m_longopt->flag = NULL;
    m_longopt->val = static_cast<int>(m_shortopt);
  }
  
  
  BaseCallback::
  ~BaseCallback()
  {
  }
  
  
  bool Parser::
  Add(boost::shared_ptr<BaseCallback> callback)
  {
    if(m_index_map.find(callback->m_shortopt) != m_index_map.end())
      return false;
    m_index_map.insert(std::make_pair(callback->m_shortopt,
				      m_callback.size()));
    if(callback->m_longopt_name.size() > m_longest_longopt)
      m_longest_longopt = callback->m_longopt_name.size();  
    m_callback.push_back(callback);
    m_present.push_back(false);	// will be set in Do()
    m_argument.push_back(0);	// will be set in Do()
    return true;
  }
  
  
  int Parser::
  Do(int argc,
     char ** argv,
     std::ostream & os)
  {
    // construct longopt and shortopt
    option longopt[m_callback.size() + 1];
    std::string shortopt;
    for(unsigned int i(0); i < m_callback.size(); ++i){
      longopt[i] = * m_callback[i]->m_longopt;
      shortopt.push_back(m_callback[i]->m_shortopt);
      if(m_callback[i]->m_requires_arg)
	shortopt.push_back(':');
    }
    longopt[m_callback.size()].name =    NULL;
    longopt[m_callback.size()].has_arg = 0;
    longopt[m_callback.size()].flag =    NULL;
    longopt[m_callback.size()].val =     0;
    
    // parser command line into callback references and argument pointers
    int res;
    while(1){
      res = getopt_long(argc, argv, shortopt.c_str(), longopt, NULL);
      
      // check for errors and end of options
      if((res == '?') || (res == ':')){
	os << argv[0] << ": Problems with option '" << (char) optopt << "'";
	return -1;
      }
      if(res == -1)
	break;
      
      // find corresponding callback index
      index_map_t::const_iterator im(m_index_map.find(static_cast<char>(res)));
      if(im == m_index_map.end()){
	os << argv[0] << ": BUG (inconsistent index map) in "
	   << __FILE__ << "\n";
	return -1;
      }
      
      const size_t index(im->second);
      // check for errors
      if(index >= m_callback.size()){
	os << argv[0] << ": BUG (option index mismatch) in "
	   << __FILE__ << "\n";
	return -1;
      }
      
      // flag as present
      m_present[index] = true;
      if(m_callback[index]->m_requires_arg)
	m_argument[index] = optarg;
    }
    
    // run present callbacks in order of calls to Parser::Add()
    for(size_t ii(0); ii < m_callback.size(); ++ii)
      if(m_present[ii])
	m_callback[ii]->Do(m_argument[ii], os);
    
    return optind;
  }
  
  
  void Parser::
  UsageMessage(std::ostream & os)
  {
    for(size_t ii(0); ii < m_callback.size(); ++ii){
      os << "  -" << m_callback[ii]->m_shortopt
	 << "  --" << std::setw(m_longest_longopt) << std::left
	 << m_callback[ii]->m_longopt_name;
      if(m_callback[ii]->m_requires_arg)
	os << " <arg>  ";
      else
	os << "        ";
      os << m_callback[ii]->m_description << "\n";
    }
  }
  
}
