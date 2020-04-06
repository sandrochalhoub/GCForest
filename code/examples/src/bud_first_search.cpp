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

#include "Backtrack.hpp"
#include "CSVReader.hpp"
#include "CmdLine.hpp"
#include "DL8.hpp"
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

  Wood yallen;

  BacktrackingAlgorithm A(yallen, opt);

  string ext{opt.instance_file.substr(opt.instance_file.find_last_of(".") + 1)};

  if (opt.format == "csv" or (opt.format == "guess" and ext == "csv")) {
    csv::read_binary(opt.instance_file, [&](vector<int> &data) {
      A.addExample(data.begin(), data.end() - 1, data.back());
    });
  } else if (opt.format == "dl8" or (opt.format == "guess" and ext == "dl8")) {
    txt::read_binary(opt.instance_file, [&](vector<int> &data) {
      auto y = *data.begin();
      A.addExample(data.begin() + 1, data.end(), y);
    });
  } else {
    if (opt.format != "txt" and ext != "txt")
      cout << "p Warning, unrecognized format, trying txt\n";
    txt::read_binary(opt.instance_file, [&](vector<int> &data) {
      A.addExample(data.begin(), data.end() - 1, data.back());
    });
  }

  if (opt.print_ins)
    cout << "d examples=" << A.numExample() << " features=" << A.numFeature()
         << endl;

  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d readtime=" << cpu_time() << endl;

  A.search();

  TreeNode sol = A.getSolution();

  if (opt.print_sol) {
    cout << sol << endl;
  }

  if (opt.verified) {
    assert(sol.predict(A.dataset[0].begin(), A.dataset[0].end(), A.dataset[1].begin(), A.dataset[1].end()) == A.error());

  		cout << "p solution verified (" << A.error() << ")" << endl;
  }
}
