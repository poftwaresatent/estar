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
  
  Algorithm algo1(false, false, false, false, false);
  Algorithm algo2(false, false, false, false, false);
  
#define TEST_CASE 4
  cout << "init test case " << TEST_CASE << "\n";

// #elif TEST_CASE == 4
  Grid grid1(algo1, 3, 3, FOUR_CONNECTED);
  Grid grid2(algo2, 3, 3, FOUR_CONNECTED);
  LSMKernel kernel1(grid1, 1.25);
  LSMKernel kernel2(grid2, 1.25);

  if( ! skip_input){
    cout << "checking cspace\n";
    ostringstream os;
    if( ! check_cspace(algo1.GetCSpace(), "", os)){
      dump_grid(grid1, stdout);
      cout << os.str();
      exit(EXIT_FAILURE);
    }
    if( ! check_cspace(algo2.GetCSpace(), "", os)){
      dump_grid(grid2, stdout);
      cout << os.str();
      exit(EXIT_FAILURE);
    }
  }
  
  cout << "\n==================================================\n"
       << "algo1.AddGoal(grid1.GetVertex(0, 0), 0)\n";
  algo1.AddGoal(grid1.Index2Vertex(0, 0), 0);
  while(algo1.HaveWork()){
    algo1.ComputeOne(kernel1, 0.5);
  }
  if( ! skip_output)
    dump_grid(grid1, stdout);
  
  cout << "\n==================================================\n"
       << "algo1.SetMeta(grid.Index2Vertex(1, 0), "
       << kernel1.obstacle_meta << ", kernel1)\n";
  algo1.SetMeta(grid1.Index2Vertex(1, 0), kernel1.obstacle_meta, kernel1);
  if( ! skip_output)
    dump_grid(grid1, stdout);
  while(algo1.HaveWork()){
    if(skip_output){
      const size_t step(algo1.GetStep());
      if( ! (step % 1000))
	cout << "step " << step << "\n";
    }
    else{
      dump_queue(algo1, &grid1, 0, stdout);
      if( ! check_queue(algo1, &grid1, "", cout))
	exit(EXIT_FAILURE);
      dump_grid(grid1, stdout);
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
    algo1.ComputeOne(kernel1, 0.5);
  }
  
  cout << "\n==================================================\n"
       << "algo2.AddGoal(grid2.Index2Vertex(0, 0), 0)\n";
  algo2.AddGoal(grid2.Index2Vertex(0, 0), 0);
  cout << "\n==================================================\n"
       << "algo2.SetMeta(grid2.Index2Vertex(1, 0), "
       << kernel2.obstacle_meta << ", kernel2)\n";
  algo2.SetMeta(grid2.Index2Vertex(1, 0), kernel2.obstacle_meta, kernel2);
  while(algo2.HaveWork()){
    algo2.ComputeOne(kernel2, 0.5);
  }
  if( ! skip_output)
    dump_grid(grid2, stdout);
}
