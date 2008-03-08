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


#ifndef ESTAR_BASE_HPP
#define ESTAR_BASE_HPP


#include <estar/numeric.hpp>
#include <boost/graph/adjacency_list.hpp>


namespace estar {
  
  
  /**
     Node flags. Used to be "event priority" classes, now much simpler
     and lightweight.
  */
  typedef enum {
    /** nothing special, node is (locally) consistent */
    NONE,			// 0 = b00
    /** node is in the queue, but not a goal */
    OPEN,			// 1 = b01
    /** goal node, but not in queue */
    GOAL,			// 2 = b10
    /** goal node in queue */
    OPNG			// 3 = b11
  } flag_enum_t;

#ifdef WIN32
  typedef size_t flag_t;
#else
  typedef flag_enum_t flag_t;
#endif // WIN32


  /**
     Debug utility.
     \return name of a flag, or "<invalid>"
  */
  const char * flag_name(flag_t p);
  
  
  //////////////////////////////////////////////////
  // cspace traits
  
  /** C-space graph out-edge list type selector. */
  typedef boost::vecS cspace_OutEdgeListS;

  /** C-space graph vertex list type selector. */
  typedef boost::vecS cspace_VertexListS;

  /** C-space graph edge directedness selector. */
  typedef boost::undirectedS cspace_DirectedS;

  /** Traits class of C-space graph. */
  typedef boost::adjacency_list_traits<cspace_OutEdgeListS,
				       cspace_VertexListS,
				       cspace_DirectedS> cspace_traits;

  /** Vertex (node) descriptor of C-space graph. */
  typedef cspace_traits::vertex_descriptor vertex_t;
  
  
  //////////////////////////////////////////////////
  // vertex properties
  
  /** Value property tag attached to C-space nodes. */
  struct value_p    { typedef boost::vertex_property_tag kind; };
  
  /** Meta property tag attached to C-space nodes. */
  struct meta_p     { typedef boost::vertex_property_tag kind; };
  
  /** "Right-hand-side" property tag attached to C-space nodes. */
  struct rhs_p      { typedef boost::vertex_property_tag kind; };
  
  /** Flag property tag attached to C-space nodes. */
  struct flag_p     { typedef boost::vertex_property_tag kind; };
  
  
  //////////////////////////////////////////////////
  // cspace graph and supplementary traits
  
  /** Properties attached to a C-space node. */
  typedef boost::property<value_p, double,
          boost::property<meta_p, double,
	  boost::property<rhs_p, double,
	  boost::property<flag_p, flag_t
          > > > > cspace_vertex_property;
  
  /** Type describing the C-space graph. */
  typedef boost::adjacency_list<cspace_OutEdgeListS,
				cspace_VertexListS,
				cspace_DirectedS,
				cspace_vertex_property> cspace_t;

  /** Iterator over neighboring nodes. */
  typedef boost::graph_traits<cspace_t>::adjacency_iterator adjacency_it;
  
  /** Iterator over C-space nodes. */
  typedef boost::graph_traits<cspace_t>::vertex_iterator    vertex_it;
  
  
  //////////////////////////////////////////////////
  // property map types
  
  /** Node ID map, for adding custom node properties. */
  typedef
  boost::property_map<cspace_t, boost::vertex_index_t>::type vertexid_map_t;
  
  /** Value property map. */
  typedef boost::property_map<cspace_t, value_p>::type    value_map_t;

  /** Meta information property map. */
  typedef boost::property_map<cspace_t, meta_p>::type     meta_map_t;

  /** "Right-hand-side" property map. */
  typedef boost::property_map<cspace_t, rhs_p>::type      rhs_map_t;

  /** Flag property map. */
  typedef boost::property_map<cspace_t, flag_p>::type     flag_map_t;
  
} // namespace estar

#endif // ESTAR_BASE_HPP
