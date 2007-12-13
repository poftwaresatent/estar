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


#include <estar/Algorithm.hpp>
#include <estar/NF1Kernel.hpp>
#include <estar/AlphaKernel.hpp>
#include <estar/LSMKernel.hpp>
#include <estar/Grid.hpp>
#include <estar/check.hpp>
#include <estar/dump.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>


using namespace estar;
using namespace boost;
using namespace std;


int main(int argc, char ** argv)
{
  bool skip_output(false);
  bool skip_input(false);
  if(argc > 1){
    const string arg(argv[1]);
    if(arg == "i"){
      cout << "skipping input\n";
      skip_input = true;
    }
    else if(arg == "o"){
      cout << "skipping output\n";
      skip_output = true;
    }
    else if((arg == "io") || (arg == "oi")){
      cout << "skipping input and output\n";
      skip_input = true;
      skip_output = true;
    }
    else
      cout << "don't understand arg \"" << arg << "\"\n";
  }
  
  int const test_case(6);
  cout << "test case " << test_case << "\n";
  
  shared_ptr<Grid> grid;
  shared_ptr<Kernel> kernel;
  if (1 == test_case) {
    grid.reset(new Grid(Grid::EIGHT));
    kernel.reset(new AlphaKernel(1));
    grid->Init(0, 5, 0, 3, kernel->freespace_meta);
  }
  else if (2 == test_case) {
    grid.reset(new Grid(Grid::FOUR));
    kernel.reset(new NF1Kernel());
    grid->Init(0, 5, 0, 3, kernel->freespace_meta);
  }
  else if (3 == test_case) {
    grid.reset(new Grid(Grid::FOUR));
    kernel.reset(new AlphaKernel(1));
    grid->Init(0, 5, 0, 3, kernel->freespace_meta);
  }
  else if (4 == test_case) {
    grid.reset(new Grid(Grid::FOUR));
    kernel.reset(new LSMKernel(grid->GetCSpace(), 1));
    grid->Init(0, 5, 0, 3, kernel->freespace_meta);
  }
  else if (5 == test_case) {
    grid.reset(new Grid(Grid::FOUR));
    kernel.reset(new LSMKernel(grid->GetCSpace(), 1));
    grid->Init(0, 500, 0, 300, kernel->freespace_meta);
    skip_input = true;
    skip_output = true;
  }
  else if (6 == test_case) {
    grid.reset(new Grid(Grid::SIX));
    kernel.reset(new AlphaKernel(1));
    grid->Init(0, 5, 0, 3, kernel->freespace_meta);
  }
  else {
    cerr << "invalid test_case " << test_case << "\n";
    exit(EXIT_FAILURE);
  }
  
  Algorithm algo(grid->GetCSpace(), false, false, false, false, false);
  
  if( ! skip_input){
    cout << "checking cspace\n";
    ostringstream os;
    if( ! check_cspace(algo.GetCSpaceGraph(), "", os)){
      dump_grid(*grid, stdout);
      cout << os.str();
      exit(EXIT_FAILURE);
    }
  }
  
  cout << "\n==================================================\n"
       << "add goal (0, 0)\n";
  algo.AddGoal(grid->GetNode(0, 0)->vertex, 0);
  while(algo.HaveWork()){
    if(skip_output){
      const size_t step(algo.GetStep());
      if( ! (step % 1000))
	cout << "step " << step << "\n";
    }
    else{
      dump_queue(algo, grid.get(), 0, stdout);
      if( ! check_queue(algo, grid->GetCSpace().get(), "", cout))
	exit(EXIT_FAILURE);
      dump_grid(*grid, stdout);
    }
    if( ! skip_input){
      string cmd;
      cout << "[ENTER]";
      getline(cin, cmd);
      if(cmd == "q")
	exit(EXIT_SUCCESS);
      if(cmd == "s")
	skip_input = true;
    }
    algo.ComputeOne(*kernel, 0.5);
  }
  if( ! skip_output)
    dump_grid(*grid, stdout);
  
  cout << "\n==================================================\n"
       << "set meta (1, 1) to " << kernel->obstacle_meta << "\n";
  algo.SetMeta(grid->GetNode(1, 1)->vertex, kernel->obstacle_meta, *kernel);
  while(algo.HaveWork()){
    if(skip_output){
      const size_t step(algo.GetStep());
      if( ! (step % 1000))
	cout << "step " << step << "\n";
    }
    else{
      dump_queue(algo, grid.get(), 0, stdout);
      if( ! check_queue(algo, grid->GetCSpace().get(), "", cout))
	exit(EXIT_FAILURE);
      dump_grid(*grid, stdout);
    }
    if( ! skip_input){
      string cmd;
      cout << "[ENTER]";
      getline(cin, cmd);
      if(cmd == "q")
	exit(EXIT_SUCCESS);
      if(cmd == "s")
	skip_input = true;
    }
    algo.ComputeOne(*kernel, 0.5);
  }
  if( ! skip_output)
    dump_grid(*grid, stdout);
}
