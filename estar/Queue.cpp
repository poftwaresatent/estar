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


#include "Queue.hpp"
#include "numeric.hpp"
#include <estar/util.hpp>


using std::make_pair;


#ifdef DEBUG
# define ESTAR_QUEUE_DEBUG
#else
# undef ESTAR_QUEUE_DEBUG
#endif

#ifdef ESTAR_QUEUE_DEBUG
# define PDEBUG PDEBUG_OUT
#else
# define PDEBUG PDEBUG_OFF
#endif


namespace estar {
  

  bool Queue::
  IsEmpty() const
  {
    return m_queue.empty();
  }
  
  
  vertex_t Queue::
  Pop(flag_map_t & flag_map)
  {
    BOOST_ASSERT( ! m_queue.empty() );
    queue_it iq(m_queue.begin());
    const vertex_t vertex(iq->second);
    m_queue.erase(iq);
    m_map.erase(vertex);
    put(flag_map, vertex, static_cast<flag_t>(get(flag_map, vertex) ^ OPEN));
    PDEBUG("f: %s i: %lu\n", flag_name(get(flag_map, vertex)), vertex);
    return vertex;
  }
  
  
  void Queue::
  Requeue(vertex_t vertex,
	  flag_map_t & flag_map,
	  const value_map_t & value_map,
	  const rhs_map_t & rhs_map)
  {
    const double value(get(value_map, vertex));
    const double rhs(get(rhs_map, vertex));
    const flag_t flag(get(flag_map, vertex));
    
    if(absval(value - rhs) < epsilon){
      PDEBUG("CONSISTENT f: %s i: %lu v: %g rhs: %g\n",
	     flag_name(flag), vertex, value, rhs);
      if(flag & OPEN){
	DoDequeue(vertex, m_queue.begin());
	m_map.erase(vertex);
	put(flag_map, vertex, static_cast<flag_t>(flag ^ OPEN));
      }
      return;
    }
    
    const double key(minval(value, rhs));
    if( ! (flag & OPEN)){
      m_queue.insert(make_pair(key, vertex));
      m_map.insert(make_pair(vertex, key));
      put(flag_map, vertex, static_cast<flag_t>(flag | OPEN));
      PDEBUG("ENQUEUE f: %s i: %lu v: %g rhs: %g\n",
	     flag_name(flag), vertex, value, rhs);
      return;
    }
    
    queue_map_t::iterator im(m_map.find(vertex));
    BOOST_ASSERT( im != m_map.end() );
    if(absval(im->second - key) < epsilon){
      PDEBUG("KEEP f: %s i: %lu v: %g rhs: %g (absval(%g) < %g)\n",
	     flag_name(flag), vertex, value, rhs, im->second - key, epsilon);
      return;
    }
    
    PDEBUG("REQUEUE f: %s i: %lu v: %g rhs: %g\n",
	   flag_name(flag), vertex, value, rhs);
    DoDequeue(vertex, m_queue.find(im->second));
    m_queue.insert(make_pair(key, vertex));
    im->second = key;
  }
  
  
  void Queue::
  Clear()
  {
    m_queue.clear();
    m_map.clear();
  }
  
  
  void Queue::
  DoDequeue(vertex_t vertex, queue_t::iterator iq)
  {
    for(/**/; iq != m_queue.end(); ++iq){
      if(iq->second == vertex)
	break;
    }
    if(iq == m_queue.end()){
      PDEBUG("WARNING search again!\n");
      for(iq = m_queue.begin(); iq != m_queue.end(); ++iq){
	if(iq->second == vertex)
	  break;
      }
    }
    BOOST_ASSERT( iq != m_queue.end() );
    m_queue.erase(iq);
  }
  
  
  bool Queue::
  VitaminB(vertex_t vertex)
  {
    queue_map_t::iterator im(m_map.find(vertex));
    if(im == m_map.end())
      return false;
    DoDequeue(vertex, m_queue.find(im->second));
    const double key(m_queue.begin()->second - 1);
    m_queue.insert(make_pair(key, vertex));
    im->second = key;
    return true;
  }
  
} // namespace estar
