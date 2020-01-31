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

#include "CSVReader.hpp"
#include "CmdLine.hpp"
#include "DataSet.hpp"
#include "TXTReader.hpp"
#include "TypedDataSet.hpp"
#include "Backtrack.hpp"

using namespace std;
using namespace primer;

int main(int argc, char *argv[]) {

  Options opt = parse(argc, argv);

  if (opt.print_cmd)
    cout << opt.cmdline << endl;

  if (opt.print_par)
    opt.display(cout);

  std::mt19937 random_generator;
  random_generator.seed(opt.seed);

  TypedDataSet input;

  if (opt.format == "csv")
    csv::read(
        opt.instance_file,
        [&](vector<string> &f) { input.setFeatures(f.begin(), f.end() - 1); },
        [&](vector<string> &data) {
          auto y = data.back();
          data.pop_back();
          input.addExample(data.begin(), data.end(), y);
        });
  else
    txt::read(opt.instance_file, [&](vector<string> &data) {
      auto y = data.back();
      data.pop_back();
      input.addExample(data.begin(), data.end(), y);
    });

  DataSet base;

  input.binarize(base);

  if (opt.sample != 1) {

    base.uniformSample(0, (double)(base.positiveCount()) * opt.sample,
                       random_generator);
    base.uniformSample(1, (double)(base.negativeCount()) * opt.sample,
                       random_generator);
  }

  auto count{base.count()};

  if (opt.verbosity >= Options::QUIET)
    cout << "c filter base\n";

  base.filter();

  // base.computeBounds();

  // exit(1);

  if (opt.verbosity >= Options::QUIET)
    if (base.count() < count)
      cout << "c filtered " << (count - base.count()) / 2
           << " noisy example(s)\n";


	BacktrackingAlgorithm A(base, 10);
	
	
	//
	// for(auto f{0}; f<base.numFeature(); ++f)
	// {
	// 	 cout << f << " " << A.entropy(0, f) << endl;
	// }
	//
	// cout << A << endl;
	//
	// A.split(0,4);
	
	cout << A << endl;
	
	A.expend();
	
	cout << A << endl;
	
	A.expend();
	
	cout << A << endl;
	
	A.expend();
	
	cout << A << endl;
	
	// A.expend();
	//
	// cout << A << endl;
	

}



