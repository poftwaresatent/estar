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


#ifndef ESTAR_FLEXGRID_HPP
#define ESTAR_FLEXGRID_HPP


#include <estar/sdeque.hpp>
#include <estar/flexgrid_traits.hpp>
#include <estar/flexgrid_iterator.hpp>


namespace estar {
  
  
  template<typename value_t>
  class flexgrid
  {
  public:
    typedef flexgrid_traits<value_t>             traits;
    typedef typename traits::line_t              line_t;
    typedef typename traits::cell_iterator       cell_iterator;
    typedef typename traits::const_cell_iterator const_cell_iterator;
    typedef typename traits::grid_t              grid_t;
    typedef typename traits::line_iterator       line_iterator;
    typedef typename traits::const_line_iterator const_line_iterator;
    typedef flexgrid_iterator<value_t>           iterator;
    typedef const_flexgrid_iterator<value_t>     const_iterator;
    
    value_t & at(ssize_t ix, ssize_t iy) { return m_grid.at(iy).at(ix); }
    
    value_t const & at(ssize_t ix, ssize_t iy) const
    { return m_grid.at(iy).at(ix); }
    
    void resize_xbegin(ssize_t xbegin, value_t const & value) {
      for (line_iterator ii(m_grid.begin()); ii != m_grid.end(); ++ii)
	ii->resize_begin(xbegin, value);
      m_default.resize_begin(xbegin);
    }
    
    void resize_xbegin(ssize_t xbegin) { resize_xbegin(xbegin, value_t()); }
    
    void resize_xend(ssize_t xend, value_t const & value) {
      for (line_iterator ii(m_grid.begin()); ii != m_grid.end(); ++ii)
	ii->resize_end(xend, value);
      m_default.resize_end(xend);
    }
    
    void resize_xend(ssize_t xend) { resize_xend(xend, value_t()); }
    
    void resize_x(ssize_t xbegin, ssize_t xend, value_t const & value) {
      resize_xbegin(xbegin, value);
      resize_xend(xend, value);
    }
    
    void resize_x(ssize_t xbegin, ssize_t xend)
    { resize_x(xbegin, xend, value_t()); }
    
    /** \note Lots of copying if you're growing the range. Prefer the
	version with implicit default value, it's gonna be faster. */
    void resize_ybegin(ssize_t ybegin, value_t const & value) {
      if (ybegin > m_grid.ibegin()) // shrink
	m_grid.resize_begin(ybegin, m_default);
      else if (ybegin < m_grid.ibegin()) { // grow
	line_t vline(m_default);
	fill(vline.begin(), vline.end(), value);
	m_grid.resize_begin(ybegin, vline);
      }
      // else do nothing
    }
    
    void resize_ybegin(ssize_t ybegin)
    { m_grid.resize_begin(ybegin, m_default); }
    
    /** \note Lots of copying if you're growing the range. Prefer the
	version with implicit default value, it's gonna be faster. */
    void resize_yend(ssize_t yend, value_t const & value) {
      if (yend < m_grid.iend()) // shrink
	m_grid.resize_end(yend, m_default);
      else if (yend > m_grid.iend()) { // grow
	line_t vline(m_default);
	fill(vline.begin(), vline.end(), value);
	m_grid.resize_end(yend, vline);
      }
      // else do nothing
    }
    
    void resize_yend(ssize_t yend)
    { m_grid.resize_end(yend, m_default); }
    
    void resize_y(ssize_t ybegin, ssize_t yend, value_t const & value) {
      resize_ybegin(ybegin, value);
      resize_yend(yend, value);
    }
    
    void resize_y(ssize_t ybegin, ssize_t yend) {
      resize_ybegin(ybegin);
      resize_yend(yend);
    }
    
    /** \note Beware of parameter ordering, it is NOT like the
	coordinates of a boudning box. */
    void resize(ssize_t xbegin, ssize_t xend,
		ssize_t ybegin, ssize_t yend,
		value_t const & value) {
      resize_xbegin(xbegin, value);
      resize_xend(xend, value);
      resize_ybegin(ybegin, value);
      resize_yend(yend, value);
    }
    
    /** \note Beware of parameter ordering, it is NOT like the
	coordinates of a boudning box. */
    void resize(ssize_t xbegin, ssize_t xend,
		ssize_t ybegin, ssize_t yend) {
      resize_xbegin(xbegin);
      resize_xend(xend);
      resize_ybegin(ybegin);
      resize_yend(yend);
    }
    
    ssize_t xbegin() const { return m_default.ibegin(); }
    
    ssize_t xend() const { return m_default.iend(); }
    
    ssize_t ybegin() const { return m_grid.ibegin(); }
    
    ssize_t yend() const { return m_grid.iend(); }
    
    /** Automatically resizes the flexgrid as required in order to
	yield a valid value_t reference. */
    value_t & smart_at(ssize_t ix, ssize_t iy) {
      if (ix < m_default.ibegin())
	resize_xbegin(ix);
      else if (ix >= m_default.iend())
	resize_xend(ix + 1);
      if (iy < m_grid.ibegin())
	resize_ybegin(iy);
      else if (iy >= m_grid.iend())
	resize_yend(iy + 1);
      return m_grid.at(iy).at(ix);
    }
    
    line_iterator line_begin() { return m_grid.begin(); }
    
    const_line_iterator line_begin() const { return m_grid.begin(); }
    
    line_iterator line_end() { return m_grid.end(); }
    
    const_line_iterator line_end() const { return m_grid.end(); }
    
    iterator begin() {
      return iterator(m_grid, m_default, m_default.ibegin(), m_grid.ibegin());
    }
    
    const_iterator begin() const {
      return const_iterator(m_grid, m_default,
			    m_default.ibegin(), m_grid.ibegin());
    }
    
    iterator end()
    { return iterator(m_grid, m_default, m_default.iend(), m_grid.iend()); }
    
    const_iterator end() const {
      return const_iterator(m_grid, m_default,
			    m_default.iend(), m_grid.iend());
    }
    
  protected:
    friend class flexgrid_iterator<value_t>;
    
    grid_t m_grid;
    line_t m_default;
  };
  
}

#endif // ESTAR_FLEXGRID_HPP
