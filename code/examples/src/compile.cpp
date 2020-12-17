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

#include "CmdLine.hpp"
#include "Compiler.hpp"
#include "Reader.hpp"
#include "Tree.hpp"
#include "TypedDataSet.hpp"
#include "WeightedDataset.hpp"

using namespace std;
using namespace blossom;


template <typename E_t = int>
int run_algorithm(DTOptions &opt) {

  WeightedDataset<E_t> input;

  ////// READING
  if (opt.binarize) {

    read_non_binary(input, opt);

  } else {

    read_binary(input, opt);
  }

  // in compilation, noise and duplicates must be removed (we should probably
  // remove redundant features as well, though it makes sense only for
  // encodings)
  input.preprocess();

  Compiler<E_t> A(opt);

  auto X{input[0]};
  for (auto i : X) {
    A.addExample(X[i]);
  }

  if (opt.print_ins) {
    cout << "d examples=" << A.numExample() << " features=" << A.numFeature()
         << endl;
    for (auto i : X) {
      cout << X[i] << endl;
    }
  }

  exit(1);

  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d readtime=" << cpu_time() << endl;

  A.initialise_search();

  A.search();
	
  return 0;
}


int main(int argc, char *argv[]) {

  DTOptions opt = parse_dt(argc, argv);

  if (opt.print_cmd)
    cout << opt.cmdline << endl;

  if (opt.print_par)
    opt.display(cout);

  return run_algorithm<int>(opt);
}
