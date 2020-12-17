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
#include "Tree.hpp"
#include "WeightedDataset.hpp"
#include "Reader.hpp"

using namespace std;
using namespace blossom;

template <template <typename> class ErrorPolicy = CardinalityError,
          typename E_t = unsigned long>
int run_algorithm(DTOptions &opt) {

  WeightedDataset<E_t> input;

  ////// READING
  if (opt.binarize) {

    read_non_binary(input, opt);

  } else {

    read_binary(input, opt);
  }

  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d readtime=" << cpu_time() << endl;

  ////// PREPROCESING
  if (opt.preprocessing)
    input.preprocess(opt.verbosity >= DTOptions::NORMAL);

  ////// CREATING THE ALGORITHM
  BacktrackingAlgorithm<ErrorPolicy, E_t> A(input, opt);

  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d inputtime=" << cpu_time() << endl;

  ////// PRINTING DATA INFO
  if (opt.print_ins)
    cout << "d examples=" << A.numExample() << " features=" << A.numFeature()
         << endl;

  ////// SOLVING
  if (opt.mindepth) {
    if (opt.minsize)
      A.minimize_error_depth_size();
    else
      A.minimize_error_depth();
  } else
    A.minimize_error();

  Tree sol = A.getSolution();

  if (opt.print_sol) {
    cout << sol << endl;
  }

  if (opt.verified) {

    E_t tree_error = 0;
    for (auto y{0}; y < 2; ++y) {
      auto X{input[y]};
      for (auto i : X)
        tree_error += (sol.predict(X[i]) != y) * X.weight(i);
    }

    assert(tree_error == A.error());

    cout << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
         << std::setw(0) << "p solution verified (" << tree_error << " / "
         << A.error() << ")" << endl;
  }
  return 1;
}


int main(int argc, char *argv[]) {
  DTOptions opt = parse_dt(argc, argv);

  if (opt.print_cmd)
    cout << opt.cmdline << endl;

  if (opt.print_par)
    opt.display(cout);

  if (opt.preprocessing) {
    return run_algorithm<WeightedError, unsigned long>(opt);
  } else {
    return run_algorithm<>(opt);
  }
}
