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


#ifndef ESTAR_FLEXGRID_ITERATOR_HPP
#define ESTAR_FLEXGRID_ITERATOR_HPP


#include <estar/flexgrid_traits.hpp>
#include <iterator>


namespace estar {
  
  
  template<typename blah>
  struct flexgrid_iterator_traits {
    typedef flexgrid_traits<blah> traits;
    typedef blah value_t;
    typedef value_t * pointer_t;
    typedef value_t & reference_t;
    typedef typename traits::grid_t & grid_ref_t;
    typedef typename traits::line_t & line_ref_t;
  };
  
  
  template<typename blah>
  struct flexgrid_const_iterator_traits {
    typedef flexgrid_traits<blah> traits;
    typedef blah value_t;
    typedef value_t const * pointer_t;
    typedef value_t const & reference_t;
    typedef typename traits::grid_t const & grid_ref_t;
    typedef typename traits::line_t const & line_ref_t;
  };
  
  
  template<typename value_t, typename traits>
  class base_flexgrid_iterator
  {
  public:
    // shortcut
    typedef base_flexgrid_iterator<value_t, traits> self;
    
    // for std iterator compliance... needed?
    typedef std::bidirectional_iterator_tag iterator_category;
    typedef typename traits::value_t        value_type;
    typedef typename std::ptrdiff_t         difference_type;
    typedef typename traits::pointer_t      pointer;
    typedef typename traits::reference_t    reference;
    
    // for const / non-const
    typedef typename traits::grid_ref_t     grid_ref_t;
    typedef typename traits::line_ref_t     line_ref_t;
    
    
    base_flexgrid_iterator(grid_ref_t grid, line_ref_t line,
			   ssize_t ix, ssize_t iy)
      : m_grid(grid), m_line(line), m_ix(ix), m_iy(iy) {}
    
    base_flexgrid_iterator(base_flexgrid_iterator const & orig)
      : m_grid(orig.m_grid), m_line(orig.m_line),
	m_ix(orig.m_ix), m_iy(orig.m_iy) {}
    
    bool at_end() const
    { return (m_ix >= xend()) || (m_iy >= yend()); }
    
    bool at_begin() const
    { return (m_ix <= xbegin()) && (m_iy <= ybegin()); }
    
    reference operator*() const
    { return m_grid.at(m_iy).at(m_ix); }
    
    pointer operator->() const
    { return &(m_grid.at(m_iy).at(m_ix)); }
    
    self & operator++() {
      increment();
      return *this;
    }
    
    self operator++(int) {
      self tmp(*this);
      increment();
      return tmp;
    }
    
    self & operator--() {
      decrement();
      return *this;
    }
    
    self operator--(int) {
      self tmp(*this);
      decrement();
      return tmp;
    }
    
    template<typename other_t>
    bool operator==(other_t const & other) const {
      if (&m_grid != &other.m_grid)
	return false;
      if (at_end() && other.at_end())
	return true;
      if (at_begin() && other.at_begin())
	return true;
      return (m_ix == other.m_ix) && (m_iy == other.m_iy);
    }
    
    template<typename other_t>
    bool operator!=(other_t const & other) const
    { return ! (*this == other); }
    
    ////  protected:
    grid_ref_t m_grid;
    line_ref_t m_line;
    ssize_t m_ix, m_iy;
    
    ssize_t xend() const { return m_line.iend(); }
    ssize_t xbegin() const { return m_line.ibegin(); }
    ssize_t yend() const { return m_grid.iend(); }
    ssize_t ybegin() const { return m_grid.ibegin(); }
    
    void increment() {
      if (at_end())
	return;
      if (at_begin()) {
	m_ix = xbegin() + 1;
	m_iy = ybegin();
	return;
      }
      ++m_ix;
      if (m_ix >= xend()) {
	++m_iy;
	if (m_iy >= yend())
	  return;		// at end
	m_ix = xbegin();
      }
    }
    
    void decrement() {
      if (at_begin())
	return;
      if (at_end()) {
	m_ix = xend() - 1;
	m_iy = yend() - 1;
	return;
      }
      --m_ix;
      if (m_ix < xbegin()) {
	--m_iy;
	if (m_iy < ybegin())
	  return;		// at begin
	m_ix = xend() - 1;
      }
    }
  };
  
  
  template<typename value_t>
  class flexgrid_iterator
    : public base_flexgrid_iterator<value_t,
				    flexgrid_iterator_traits<value_t> >
  {
  public:
    typedef base_flexgrid_iterator<value_t,
				   flexgrid_iterator_traits<value_t> > base;
    
    typedef typename base::grid_ref_t grid_ref_t;
    typedef typename base::line_ref_t line_ref_t;
    
    flexgrid_iterator(grid_ref_t grid, line_ref_t line, ssize_t ix, ssize_t iy)
      : base(grid, line, ix, iy) {}
    
    flexgrid_iterator(flexgrid_iterator const & orig)
      : base(orig) {}
  };
  
  
  template<typename value_t>
  class const_flexgrid_iterator
    : public base_flexgrid_iterator<value_t,
				    flexgrid_const_iterator_traits<value_t> >
  {
  public:
    typedef
    base_flexgrid_iterator<value_t,
			   flexgrid_const_iterator_traits<value_t> > base;
    
    typedef typename base::grid_ref_t grid_ref_t;
    typedef typename base::line_ref_t line_ref_t;
    
    const_flexgrid_iterator(grid_ref_t grid, line_ref_t line,
			    ssize_t ix, ssize_t iy)
      : base(grid, line, ix, iy) {}
    
    const_flexgrid_iterator(const_flexgrid_iterator const & orig)
      : base(orig) {}
    
    const_flexgrid_iterator(flexgrid_iterator<value_t> const & orig)
      : base(reinterpret_cast<base const &>(orig)) {}
  };
  
}

#endif // ESTAR_FLEXGRID_ITERATOR_HPP
