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
#include <random>
#include <stdlib.h>

#include "Compiler.hpp"
#include "CSVReader.hpp"
#include "CmdLine.hpp"
#include "DL8.hpp"
#include "DataSet.hpp"
#include "TXTReader.hpp"
#include "Tree.hpp"
#include "TypedDataSet.hpp"
#include "WeightedDataset.hpp"

using namespace std;
using namespace primer;

template <typename Algo_t>
void read(Algo_t &A, DTOptions &opt) {
  WeightedDataset input;

  string ext{opt.instance_file.substr(opt.instance_file.find_last_of(".") + 1)};

  if (opt.format == "csv" or (opt.format == "guess" and ext == "csv")) {
    csv::read_binary(opt.instance_file, [&](vector<int> &data) {
      input.addExample(data.begin(), data.end() - 1, data.back());
    });
  } else if (opt.format == "dl8" or (opt.format == "guess" and ext == "dl8")) {
    txt::read_binary(opt.instance_file, [&](vector<int> &data) {
      auto y = *data.begin();
      input.addExample(data.begin() + 1, data.end(), y);
    });
  } else {
    if (opt.format != "txt" and ext != "txt")
      cout << "p Warning, unrecognized format, trying txt\n";
    txt::read_binary(opt.instance_file, [&](vector<int> &data) {
      input.addExample(data.begin(), data.end() - 1, data.back());
    });
  }

  input.to(A);
}


template <typename E_t = int>
int run_algorithm(DTOptions &opt) {

  Compiler<E_t> A(opt);

  read(A, opt);

  if (opt.print_ins) {
    cout << "d examples=" << A.numExample() << " features=" << A.numFeature()
         << endl;
      for (auto i{0}; i < A.example.size(); ++i) {
        for (auto j{0}; j < A.numFeature(); ++j)
          cout << " " << A.reverse_dataset[j][i];
        cout << endl;
      }
  }

  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d readtime=" << cpu_time() << endl;

  A.initialise_search();

  // cout << A.numFeature() << " " << A.numExample() << " (" << (A.numFeature()
  // * A.numExample()) << ")" << endl;
  //
  // // auto nf{A.numFeature()};
  // // auto ne{A.numExample()};
  // auto nf{5};
  // auto ne{A.numExample()};
  // cout << "f0";
  // for(auto f{1}; f<nf; ++f) {
  // 	cout << ",f" << f;
  // }
  // cout <<  endl;
  // for(auto x{0}; x<ne; ++x) {
  // 	for(auto f{0}; f<nf; ++f) {
  // 		cout << A.reverse_dataset[f][x] << ",";
  // 	}
  // 	cout << "0\n";
  // }
  // cout << endl;

  A.search();
	
  return 0;
}


int main(int argc, char *argv[]) {

  DTOptions opt = parse_dt(argc, argv);

  cout << INFTY(int) << endl;

  cout << (1 << 30) - 1 << endl;

  // cout << (1 << 54) << endl;

  size_t u{1};

  u = (u << 54);

  cout << u << endl;

  cout << log2_64(u) << endl;

  if (opt.print_cmd)
    cout << opt.cmdline << endl;

  if (opt.print_par)
    opt.display(cout);

  return run_algorithm<int>(opt);
}