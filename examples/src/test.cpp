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

#include <CmdLine.hpp>
#include <SparseSet.hpp>
#include <IntList.hpp>
#include <Partition.hpp>

#include <boost/dynamic_bitset.hpp>

using namespace std;
using namespace primer;

int main(int argc, char* argv[]) {

  double x{3.0};

  cout << log2(x) << endl;
	
	auto n{50};
	
	vector<int> nxt(n);
	vector<int> prv(n);
	
	IntList l1(prv,nxt);
	vector<int> i1{3,2,7,20,1,5};
	l1.copy(i1.begin(), i1.end());
	
	IntList l2(prv,nxt);
	vector<int> i2{10,9,22,4};
	l2.copy(i2.begin(), i2.end());
	
	cout << l1 << endl;
	cout << l2 << endl;
	
	auto l3 = l1.append(l2);
	
	cout << l3 << endl;
	
	IntList l4(prv,nxt);
	IntList l5(prv,nxt);
	
	// l4.add(3,20);
	//
	// cout << l4 << endl;
	// cout << l5 << endl;
	//
	// l5.add(1,5);
	//
	// cout << l4 << endl;
	// cout << l5 << endl;
	//
	// l4.add(10,10);
	//
	// cout << l4 << endl;
	// cout << l5 << endl;
	//
	// l5.add(9,4);
	
	IntList lx[2] = {l4,l5};
	l3.split(lx, [&](const int i) { return i<7; });
	
	cout << l4 << endl;
	cout << l5 << endl;
	
	
	
	
	cout << endl << endl;
	
	
	
	
	vector<int> elt{3,2,7,20,1,5,10,9,22,4};
	
	TreePartition P;
	P.copy(elt.begin(), elt.end());
	
	
	auto r{P.addNode()};
	
	cout << P << endl;
	
	
	P.branch(r, [&](const int i) { return not (i%2); });
	
	
	cout << P << endl;
	
	
	// Part p1(elt);
	//
	// cout << p1 << endl;
	//
	//
	// Part p2(elt);
	// Part p3(elt);
	//
	// p1.split(p2, p3, [&](const int i) { return not (i%2); });
	//
	// cout << "p2: " << p2 << endl;
	// cout << "p3: " << p3 << endl;
	//
	//
	// // p1 = p2.append(p3);
	//
	//
	// cout << "p1: " << p1 << endl;
	//
	//
	//   // Options opt = parse(argc, argv);
	//   //
	//   //
	//   // if(opt.print_cmd)
	//   // 	cout << opt.cmdline << endl;
	//   //
	//   // if(opt.print_par)
	//   // 	opt.display(cout);
	//   //
	//   //
	//   //   boost::dynamic_bitset<> x(5); // all 0's by default
	//   //   x[0] = 1;
	//   //   x[1] = 1;
	//   //   x[4] = 1;
	//   //   for (boost::dynamic_bitset<>::size_type i = 0; i < x.size(); ++i)
	//   //     std::cout << x[i];
	//   //   std::cout << "\n";
	//   //   std::cout << x << "\n";
	//   //   return EXIT_SUCCESS;
}



