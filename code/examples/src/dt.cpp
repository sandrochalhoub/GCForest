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
#include <stdlib.h>
#include <random>

#include "DL8.hpp"
#include "Backtrack.hpp"
#include "CSVReader.hpp"
#include "CmdLine.hpp"
#include "DataSet.hpp"
#include "TXTReader.hpp"
#include "Tree.hpp"
#include "TypedDataSet.hpp"

using namespace std;
using namespace primer;

int main(int argc, char *argv[]) {

  DTOptions opt = parse_dt(argc, argv);

  if (opt.print_cmd)
    cout << opt.cmdline << endl;

  if (opt.print_par)
    opt.display(cout);

  std::mt19937 random_generator;
  random_generator.seed(opt.seed);

  TypedDataSet input;
	
	
	string ext{opt.instance_file.substr(opt.instance_file.find_last_of(".") + 1)};
	


  if (opt.format == "csv" or (opt.format == "guess" and ext == "csv"))
    csv::read(
        opt.instance_file,
        [&](vector<string> &f) { input.setFeatures(f.begin(), f.end() - 1); },
        [&](vector<string> &data) {
          auto y = data.back();
          data.pop_back();
          input.addExample(data.begin(), data.end(), y);
        });
  else {
		if(opt.format != "txt" and ext != "txt")
			cout << "p Warning, unrecognized format, trying txt\n";
		
    txt::read(opt.instance_file, [&](vector<string> &data) {		
      auto y = data.back();
      data.pop_back();			
      input.addExample(data.begin(), data.end(), y);
    });
	}

  DataSet base;

  input.binarize(base);

  if (opt.sample != 1) {

    base.uniformSample(0, (double)(base.positiveCount()) * opt.sample,
                       random_generator);
    base.uniformSample(1, (double)(base.negativeCount()) * opt.sample,
                       random_generator);
  }

  auto count{base.count()};

  if (opt.filter) {

    base.filter();

    if (opt.verbosity >= DTOptions::QUIET)
      cout << "c filter base: " << (count - base.count())
           << "examples suppressed\n";
  }

  if (opt.print_ins)
    cout << "d examples=" << base.count() << " features=" << base.numFeature()
         << endl;

  Wood yallen;

  // vector<int> nd;
  // for (auto i{0}; i < 6; ++i) {
  //   auto n{yallen.grow()};
  //   yallen.setFeature(n, 100 + i);
  //   nd.push_back(n);
  //
  //   cout << n << endl;
  // }
  //
  // yallen.setChild(nd[0], 0, nd[1]);
  // yallen.setChild(nd[0], 1, nd[2]);
  // yallen.setChild(nd[1], 0, nd[3]);
  // yallen.setChild(nd[1], 1, nd[4]);
  // yallen.setChild(nd[2], 0, nd[5]);
  // yallen.setChild(nd[2], 1, 0);
  // yallen.setChild(nd[3], 0, 1);
  // yallen.setChild(nd[3], 1, 0);
  // yallen.setChild(nd[4], 0, 0);
  // yallen.setChild(nd[4], 1, 1);
  // yallen.setChild(nd[5], 0, 0);
  // yallen.setChild(nd[5], 1, 1);
  //
  // cout << yallen[nd[0]] << endl;
  //
  // exit(1);

  BacktrackingAlgorithm A(base, yallen, opt);

  A.search();

  TreeNode sol = A.getSolution();

  if (opt.print_sol) {
    cout << sol << endl;
  }

  if (opt.verified) {
    assert(sol.predict(base) == A.error());
		
		cout << "p solution verified (" << A.error() << ")" << endl;
  }
}



