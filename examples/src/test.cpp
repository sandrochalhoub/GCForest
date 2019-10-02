/*************************************************************************
minicsp

Copyright 2010--2011 George Katsirelos

Minicsp is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Minicsp is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with minicsp.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/

#include <iostream>

#include <Cmdline.hpp>
#include <SparseSet.hpp>

#include <boost/dynamic_bitset.hpp>

using namespace std;

int main(int argc, char* argv[]) {
	
	Options opt = parse(argc, argv);
	
	
	if(opt.print_cmd)
		cout << opt.cmdline << endl;	
	
	if(opt.print_par)	
		opt.display(cout);


  boost::dynamic_bitset<> x(5); // all 0's by default
  x[0] = 1;
  x[1] = 1;
  x[4] = 1;
  for (boost::dynamic_bitset<>::size_type i = 0; i < x.size(); ++i)
    std::cout << x[i];
  std::cout << "\n";
  std::cout << x << "\n";
  return EXIT_SUCCESS;

}



