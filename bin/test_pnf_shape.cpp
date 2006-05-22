/* 
 * Copyright (C) 2006 Roland Philippsen <roland dot philippsen at gmx dot net>
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


#include <pnf/RobotShape.hpp>
#include <sstream>
#include <iostream>


using namespace std;


int main(int argc, char ** argv)
{
  double radius(10);
  if(argc > 1){
    istringstream is(argv[1]);
    if( ! (is >> radius)){
      cerr << argv[0] << ": couldn't read radius from \"" << argv[1] << "\"\n";
      return 1;
    }
  }
  
  pnf::Sprite shape(radius, 1);
  shape.Dump(cout);
}
