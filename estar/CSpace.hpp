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


#ifndef ESTAR_CSPACE_HPP
#define ESTAR_CSPACE_HPP


#include <estar/base.hpp>


namespace estar {
  
  
  struct vertex_read_iteration
  {
    vertex_it vi, vi_end;
    
    vertex_read_iteration(cspace_t const & cspace)
    { boost::tie(vi, vi_end) = boost::vertices(cspace); }
    
    vertex_read_iteration & operator ++ () { ++vi; return *this; }
    
    vertex_t operator * () const { return *vi; }
    
    bool at_end() const { return vi == vi_end; }
    
    bool not_at_end() const { return vi != vi_end; }
    
    template<typename some_map_t>
    typename some_map_t::value_type get(some_map_t const & some_map) const
    { return boost::get(some_map, *vi); }
  };
  
  
  struct edge_read_iteration
  {
    adjacency_it ei, ei_end;
    
    edge_read_iteration(cspace_t const & cspace, vertex_t from)
    { boost::tie(ei, ei_end) = boost::adjacent_vertices(from, cspace); }
    
    edge_read_iteration & operator ++ () { ++ei; return *this; }
    
    vertex_t operator * () const { return *ei; }
    
    bool at_end() const { return ei == ei_end; }
    
    bool not_at_end() const { return ei != ei_end; }
    
    template<typename some_map_t>
    typename some_map_t::value_type get(some_map_t const & some_map) const
    { return boost::get(some_map, *ei); }
  };
  
  
  /**
     \note You cannot add vertices to this base class, use one of the
     derived classes instead. If you need no custom data to be stored,
     just use CSpace.
  */
  class BaseCSpace
  {
  public:
    BaseCSpace();
    
    /** loop over all vertices */
    vertex_read_iteration begin() const
    { return vertex_read_iteration(m_cspace); }
    
    /** loop over all neighbors of a vertex */
    edge_read_iteration begin(vertex_t from) const
    { return edge_read_iteration(m_cspace, from); }
    
    double GetValue(vertex_t vertex) const;
    double GetMeta(vertex_t vertex) const;
    double GetRhs(vertex_t vertex) const;
    flag_t GetFlag(vertex_t vertex) const;
    
    const cspace_t & GetGraph() const { return m_cspace; }
    const value_map_t & GetValueMap() const { return m_value; }
    const meta_map_t & GetMetaMap() const { return m_meta; }
    const rhs_map_t & GetRhsMap() const { return m_rhs; }
    const flag_map_t & GetFlagMap() const { return m_flag; }
    const vertexid_map_t & GetVertexIdMap() const { return m_vertexid; }

    void SetValue(vertex_t vertex, double value);
    void SetMeta(vertex_t vertex, double meta);
    void SetRhs(vertex_t vertex, double rhs);
    void SetFlag(vertex_t vertex, flag_t flag);
    
    cspace_t & GetGraph() { return m_cspace; }
    value_map_t & GetValueMap() { return m_value; }
    meta_map_t & GetMetaMap() { return m_meta; }
    rhs_map_t & GetRhsMap() { return m_rhs; }
    flag_map_t & GetFlagMap() { return m_flag; }
    vertexid_map_t & GetVertexIdMap() { return m_vertexid; }
    
    /**
       Create an edge between two nodes in the C-space graph, where
       edges are non-directional. See AddVertex() to create nodes in
       the first place. Use a Grid instance for creating grids, it's
       much more convenient.
    */
    void AddNeighbor(vertex_t from, vertex_t to);
    
    
  protected:
    cspace_t m_cspace;
    value_map_t m_value;
    meta_map_t m_meta;
    rhs_map_t m_rhs;
    flag_map_t m_flag;
    vertexid_map_t m_vertexid;
    
    /**
       Create a new node, initializes it, and adds it to the
       underlying C-space graph. If you are creating a regular grid,
       save yourself the trouble and use a Grid instance.
       
       \return The vertex identifier that can be used to retrieve this
       node from the graph. You typically hold on to this at least
       until you've linked the node into the graph using
       AddNeighbor().
    */
    vertex_t
    AddVertex(/** The initial value of the navigation function at this
		  node. The default is infinity, as this makes the
		  most sense prior to the very first propagation. */
	      double value,
	      /** The "meta" information attached to this node. The
		  default is one, which means "freespace" for the
		  LSMKernel. Other Kernel subclasses might need
		  something else here. See also InitAllMeta() if you
		  need a different initial value and SetMeta() for
		  more information about why we use such a seemingly
		  fuzzy concept. */
	      double meta,
	      /** The initial "right-hand-side" value (the one-step
		  lookahead estimation of the optimal value). Recall
		  that the Queue is sorted by ascending min(value,
		  rhs), and that nodes stay on the queue as long as
		  value != rhs. The default is infinity, because at
		  first all nodes are consistently unreachable as we
		  haven't set a goal yet. */
	      double rhs,
	      /** The initial flags of the node. As we have neither
		  goal nor wavefront yet, it makes sense to use the
		  default which is NONE. */
	      flag_t flag);
  };
  
  
  class CSpace
    : public BaseCSpace
  {
  public:
    vertex_t AddVertex(double value = infinity,
		       double meta = 1,
		       double rhs = infinity,
		       flag_t flag = NONE)
    { return BaseCSpace::AddVertex(value, meta, rhs, flag); }
  };
  

  /**
     It is recommened to use a custom_t that is cheap to copy by
     value. For example, use boost::shared_ptr for larger objects.
  */  
  template<typename custom_t>
  class CustomCSpace
    : public BaseCSpace
  {
  public:
    custom_t & Lookup (vertex_t vertex)
    { return boost::get(m_custom_map, vertex); }
    
    custom_t const & Lookup (vertex_t vertex) const
    { return boost::get(m_custom_map, vertex); }
    
    vertex_t AddVertex(custom_t custom,
		       double value = infinity,
		       double meta = 1,
		       double rhs = infinity,
		       flag_t flag = NONE)
    {
      vertex_t const vertex(BaseCSpace::AddVertex(value, meta, rhs, flag));
      m_custom_vector.push_back(custom);
      m_custom_map = custom_map_t(m_custom_vector.begin(), m_vertexid);
      return vertex;
    }
    
  protected:
    typedef std::vector<custom_t> custom_vector_t;
    typedef typename custom_vector_t::iterator custom_iterator_t;
    typedef boost::iterator_property_map<custom_iterator_t,
					 vertexid_map_t> custom_map_t;
    
    custom_vector_t m_custom_vector;
    custom_map_t m_custom_map;
  };
  
  
} // namespace estar

#endif // ESTAR_CSPACE_HPP
