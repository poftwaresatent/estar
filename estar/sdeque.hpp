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


#ifndef ESTAR_SDEQUE_HPP
#define ESTAR_SDEQUE_HPP


#include <deque>
#include <stdexcept>


namespace estar {
  
  
  template<typename value_t>
  class sdeque
  {
  public:
    typedef std::deque<value_t> deque_t;
    typedef typename deque_t::iterator iterator;
    typedef typename deque_t::const_iterator const_iterator;
    
    sdeque(): m_ibegin(0), m_iend(0) {}
    
    sdeque(sdeque const & orig)
      : m_ibegin(orig.m_ibegin), m_iend(orig.m_iend), m_deque(orig.m_deque) {}
    
    value_t & at(ssize_t ii)
    { return m_deque.at(static_cast<size_t>(ii - m_ibegin)); }
    
    value_t const & at(ssize_t ii) const
    { return m_deque.at(static_cast<size_t>(ii - m_ibegin)); }
    
    void resize_begin(ssize_t ibegin, value_t const & value) {
      if (ibegin > m_iend)
	throw std::out_of_range("estar::sdeque::resize_begin() range error");
      ssize_t delta(m_ibegin - ibegin);
      m_ibegin = ibegin;
      if (0 < delta)
	m_deque.insert(m_deque.begin(), static_cast<size_t>(delta), value);
      else if (0 > delta)
	m_deque.erase(m_deque.begin(),
		      m_deque.begin() + static_cast<size_t>(-delta));
      // else nothing to do
    }
    
    void resize_begin(ssize_t ibegin) { resize_begin(ibegin, value_t()); }
    
    void resize_end(ssize_t iend, value_t const & value) {
      if (iend < m_ibegin)
	throw std::out_of_range("estar::sdeque::resize_end() range error");
      m_iend = iend;
      m_deque.resize(static_cast<size_t>(m_iend - m_ibegin), value);
    }
    
    void resize_end(ssize_t iend) { resize_end(iend, value_t()); }
    
    /** This could be implemented a bit more smartly, but the
	performance hit of calling resize_begin() and resize_end()
	shouldn't be too bad. */
    void resize(ssize_t ibegin, ssize_t iend, value_t const & value) {
      resize_begin(ibegin, value);
      resize_end(iend, value);
    }
    
    void resize(ssize_t ibegin, ssize_t iend)
    { resize(ibegin, iend, value_t()); }
    
    ssize_t ibegin() const { return m_ibegin; }
    
    ssize_t iend() const { return m_iend; }
    
    deque_t const & get() const { return m_deque; }
    
    iterator begin() { return m_deque.begin(); }
    
    const_iterator begin() const { return m_deque.begin(); }
    
    iterator end() { return m_deque.end(); }
    
    const_iterator end() const { return m_deque.end(); }
    
  protected:
    ssize_t m_ibegin, m_iend;
    deque_t m_deque;
  };
  
}

#endif // ESTAR_SDEQUE_HPP
