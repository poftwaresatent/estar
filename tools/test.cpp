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
  
  Algorithm algo;
  
#define TEST_CASE 6
  cout << "init test case " << TEST_CASE << "\n";

#if TEST_CASE == 1
  Grid grid(algo, 5, 3, EIGHT_CONNECTED);
  AlphaKernel kernel(1);

#elif TEST_CASE == 2
  Grid grid(algo, 5, 3, FOUR_CONNECTED);
  NF1Kernel kernel;

#elif TEST_CASE == 3
  Grid grid(algo, 5, 3, FOUR_CONNECTED);
  AlphaKernel kernel(1);

#elif TEST_CASE == 4
  Grid grid(algo, 5, 3, FOUR_CONNECTED);
  LSMKernel kernel(grid, 1);

#elif TEST_CASE == 5
  Grid grid(algo, 500, 300, FOUR_CONNECTED);
  LSMKernel kernel(grid, 1);
  skip_input = true;
  skip_output = true;

#elif TEST_CASE == 6
  Grid grid(algo, 5, 3, HEX_GRID);
  AlphaKernel kernel(1);

#else
#error "illegal TEST_CASE"
#endif
  
  if( ! skip_input){
    cout << "checking cspace\n";
    ostringstream os;
    if( ! check_cspace(algo.GetCSpace(), "", os)){
      dump_grid(grid, stdout);
      cout << os.str();
      exit(EXIT_FAILURE);
    }
  }
  
  cout << "\n==================================================\n"
       << "algo.AddGoal(grid.GetVertex(0, 0), 0)\n";
  algo.AddGoal(grid.GetVertex(0, 0), 0);
  while(algo.HaveWork()){
    if(skip_output){
      const size_t step(algo.GetStep());
      if( ! (step % 1000))
	cout << "step " << step << "\n";
    }
    else{
      dump_queue(algo, &grid, 0, stdout);
      if( ! check_queue(algo, &grid, "", cout))
	exit(EXIT_FAILURE);
      dump_grid(grid, stdout);
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
    algo.ComputeOne(kernel);
  }
  if( ! skip_output)
    dump_grid(grid, stdout);
  
  cout << "\n==================================================\n"
       << "algo.SetMeta(grid.GetVertex(1, 1), "
       << kernel.obstacle_meta << ", kernel)\n";
  algo.SetMeta(grid.GetVertex(1, 1), kernel.obstacle_meta, kernel);
  while(algo.HaveWork()){
    if(skip_output){
      const size_t step(algo.GetStep());
      if( ! (step % 1000))
	cout << "step " << step << "\n";
    }
    else{
      dump_queue(algo, &grid, 0, stdout);
      if( ! check_queue(algo, &grid, "", cout))
	exit(EXIT_FAILURE);
      dump_grid(grid, stdout);
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
    algo.ComputeOne(kernel);
  }
  if( ! skip_output)
    dump_grid(grid, stdout);
}
