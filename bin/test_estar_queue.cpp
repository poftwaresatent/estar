/* 
 * Copyright (C) 2006 Roland Philippsen <roland dot philippsen at gmx net>
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


#include <estar/Algorithm.hpp>
#include <estar/NF1Kernel.hpp>
#include <estar/AlphaKernel.hpp>
#include <estar/LSMKernel.hpp>
#include <estar/Grid.hpp>
#include <estar/check.hpp>
#include <estar/dump.hpp>
#include <boost/scoped_ptr.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>


using namespace estar;
using namespace boost;
using namespace std;


int main(int argc, char ** argv)
{
  static bool const check(true);
  
  Grid::neighborhood_t connect(Grid::FOUR);
  string kernel_name("lsm");
  for (int argi(1); argi < argc; ++argi) {
    string const arg(argv[argi]);
    if (arg == "hex") {
      connect = Grid::SIX;
      cout << "hexgrid\n";
    }
    else if (arg == "eight") {
      connect = Grid::EIGHT;
      cout << "eight-grid\n";
    }
    else if (arg == "four")
      cout << "four-grid\n";	// default
    else if (arg == "lsm")
      cout << "LSM kernel\n";	// default
    else if (arg == "alpha") {
      kernel_name = "alpha";
      cout << "ALPHA kernel\n";
    }
    else {
      cout << "what do you mean, \"" << arg << "\"?\n";
      exit(EXIT_FAILURE);
    }
  }
  cout << "\"" << kernel_name << "\" kernel\n";
  
  Grid grid(connect);
  Algorithm algo(grid.GetCSpace(), false, false, false, false, false);
  scoped_ptr<Kernel> kernel;
  if (kernel_name == "lsm")
    kernel.reset(new LSMKernel(grid.GetCSpace(), 1));
  else if (kernel_name == "alpha")
    kernel.reset(new AlphaKernel(1));
  else {
    cout << "what's a \"" << kernel_name << "\" kernel?\n";
    exit(EXIT_FAILURE);
  }
  grid.Init(0, 4, 0, 2, kernel->freespace_meta);
  
  if (check && ( ! check_cspace(algo.GetCSpaceGraph(), "", cout)))
    exit(EXIT_FAILURE);
  
  cout << "\n==================================================\n"
       << "add goal (0, 0)\n";
  algo.AddGoal(grid.GetNode(0, 0)->vertex, 0);
  while (true) {
    dump_upwind(algo, &grid, stdout);
    dump_queue(algo, &grid, 0, stdout);
    dump_grid(grid, stdout);
    if (check && ( ! check_queue(algo, grid.GetCSpace().get(), "", cout)))
      exit(EXIT_FAILURE);
    
    while (true) {
      string cmd;
      cout << argv[0] << " > ";
      getline(cin, cmd);

      if (cmd == "")
	break;
      else{

	if (cmd[0] == 'q')
	  exit(EXIT_SUCCESS);

	else if (cmd[0] == 'h') {
	  cout << "available commands:\n  q - quit\n  h - help"
	       << "\n  m - change meta\n  a - add goal\n  r - remove goal\n"
	       << "  <number> - change queue order\n";
	}

	else if (cmd[0] == 'm') {
	  cout << "  m <vertex> { <meta> | 'inf' }\n";
 	  istringstream is(cmd);
	  string junk;
	  vertex_t vertex;
	  string meta_token;
	  double meta(-1);
	  is >> junk >> vertex >> meta_token;
	  if (meta_token == "inf")
	    meta = infinity;
	  else {
	    istringstream mis(meta_token);
	    mis >> meta;
	    if ( ! mis)
	      cout << "oops parsing meta \"" << meta_token << "\"\n";
	  }
	  if ((meta < 0) || (! is))
	    cout << "oops parsing \"" << cmd << "\"\n";
	  else {
	    algo.SetMeta(vertex, meta, *kernel);
	    dump_upwind(algo, &grid, stdout);
	    dump_queue(algo, &grid, 0, stdout);
	    dump_grid(grid, stdout);
	    cout << "changed meta of " << vertex << " to " << meta << "\n";
	  }
	}

	else if (cmd[0] == 'a') {
	  cout << "  a <vertex> <value>\n";
 	  istringstream is(cmd);
	  string junk;
	  vertex_t vertex;
	  double value;
	  is >> junk >> vertex >> value;
	  if ( ! is)
	    cout << "oops parsing \"" << cmd << "\"\n";
	  else {
	    algo.AddGoal(vertex, value);
	    dump_upwind(algo, &grid, stdout);
	    dump_queue(algo, &grid, 0, stdout);
	    dump_grid(grid, stdout);
	    cout << "added/changed goal " << vertex << " to " << value << "\n";
	  }
	}

	else if (cmd[0] == 'r') {
	  cout << "  r <vertex>\n";
 	  istringstream is(cmd);
	  string junk;
	  vertex_t vertex;
	  is >> junk >> vertex;
	  if ( ! is)
	    cout << "oops parsing \"" << cmd << "\"\n";
	  else {
	    algo.RemoveGoal(vertex);
	    dump_upwind(algo, &grid, stdout);
	    dump_queue(algo, &grid, 0, stdout);
	    dump_grid(grid, stdout);
	    cout << "removed goal " << vertex << "\n";
	  }
	}

	else {
	  cout << "  <number>\n";
 	  istringstream is(cmd);
	  vertex_t vertex;
	  is >> vertex;
	  if ( ! is)
	    cout << "oops parsing \"" << cmd << "\"\n";
	  else {
	    if (algo.GetQueue().VitaminB(vertex)) {
	      cout << "queue reordered\n";
	      dump_queue(algo, &grid, 1, stdout);
	    }
	    else
	      cout << "vitamin B failed on vertex " << vertex << "\n";
	  }
	}
      }
    }

    if (algo.HaveWork())
      algo.ComputeOne(*kernel, 0.5);
    else
      cout << "nothing left to do\n";
  }
  dump_upwind(algo, &grid, stdout);
  dump_grid(grid, stdout);
}
