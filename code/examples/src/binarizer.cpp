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
#include "CmdLine.hpp"
#include "Reader.hpp"
#include "Tree.hpp"
#include "WeightedDataset.hpp"

using namespace std;
using namespace blossom;

template <typename E_t>
void printToFile(WeightedDataset<E_t> &input, DTOptions &opt) {

  //// CREATING THE ALGORITHM

  std::function<bool(const int f)> relevant = [](const int f) { return true; };

	BacktrackingAlgorithm<WeightedError, E_t> A(opt);

  if (opt.filter) {
		A.load(input);
    A.initialise_search();
    relevant = [&](const int f) { return A.isRelevant(f); };
  }

  auto numRelevantFeature{0};
  for (auto f{0}; f < input.numFeature(); ++f)
    numRelevantFeature += A.isRelevant(f);
  cerr << numRelevantFeature << " x " << input.count(0) << " | "
       << input.count(1) << endl;

  string ext{opt.output.substr(opt.output.find_last_of(".") + 1)};

  if (opt.output != "") {

    ofstream outfile(opt.output.c_str(), ofstream::out);

    if (ext == "csv")
      input.printDatasetToFile(outfile, opt.delimiter, relevant,
                               opt.outtarget == 0, true);
    else
      input.printDatasetToFile(outfile, string(" "), relevant,
                               opt.outtarget != -1, false);

  } else
    input.printDatasetToFile(cout, string(" "), relevant, opt.outtarget != -1,
                             false);
}

int main(int argc, char *argv[]) {
  DTOptions opt = parse_dt(argc, argv);

  WeightedDataset<int> input;

  ////// READING
  if (opt.binarize) {

    read_non_binary(input, opt);

  } else {

    read_binary(input, opt);
  }

  ////// PREPROCESING
  if (opt.preprocessing)
    input.preprocess(opt.verbosity >= DTOptions::NORMAL);

  ////// PRINTING
  printToFile(input, opt);

  return 1;
}
