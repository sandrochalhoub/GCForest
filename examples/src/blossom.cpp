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

  try {
    read_binary(input, opt);
  } catch (const std::exception &e) {
    if (opt.verbosity >= DTOptions::NORMAL)
      cout << "c format not recognized or input non-binary, binarizing...\n";
    read_non_binary(input, opt);
  }

  // vector<size_t> subset;
  WeightedDataset<E_t> *test_set = new WeightedDataset<E_t>();
  WeightedDataset<E_t> *training_set = new WeightedDataset<E_t>();

  if (opt.test_sample != 0) {

    std::vector<int>::iterator endx[2] = {input.examples[0].bbegin(),
                                          input.examples[1].bbegin()};

    input.drawSample(opt.test_sample, *training_set, *test_set, opt.seed);

    if (opt.sample_only) {
      for (auto y{0}; y < 2; ++y) {
        cout << y << " " << (endx[y] - input.examples[y].bbegin());
        for (auto x{input.examples[y].bbegin()}; x != endx[y]; ++x) {
          cout << " " << *x;
        }
        cout << endl;
        // cout << input.examples[y] << endl;
      }
      return 0;
    }

    // cout << *training_set << endl;
    // cout << *test_set << endl;

  } else {
    training_set = &input;
  }

  // cout << training_set << endl;

  // cout << input.example_count() << endl;

  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d readtime=" << cpu_time() << endl;

  ////// PREPROCESING
  if (opt.preprocessing) {
    training_set->preprocess(opt.verbosity >= DTOptions::NORMAL);
  }

  ////// CREATING THE ALGORITHM
  BacktrackingAlgorithm<ErrorPolicy, E_t> A(*training_set, opt);

  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d inputtime=" << cpu_time() << endl;

  ////// PRINTING DATA INFO
  if (opt.print_ins)
    cout << "d examples=" << A.numExample() << " features=" << A.numFeature()
         << endl;

  // A.perfectTree();
  // A.minimize_error();

  ////// SOLVING
  if (opt.mindepth) {
    if (opt.minsize)
      A.minimize_error_depth_size();
    else
      A.minimize_error_depth();
  } else {
    if (opt.minsize)
      A.set_size_objective();
    A.minimize_error();
  }

  Tree<E_t> sol = A.getSolution();

  if (opt.verified) {

    E_t tree_error = 0;
    for (auto y{0}; y < 2; ++y) {
      auto X{(*training_set)[y]};
      for (auto i : X)
        tree_error += (sol.predict(X[i]) != y) * X.weight(i);
    }

    assert(tree_error == A.error());

    cout << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
         << std::setw(0) << "p solution verified (" << tree_error << " / "
         << A.error() << ")" << endl;
      
      
//      cout << sol << endl;
  }

  if (opt.pruning) {

    // cout << "p post-pruning (additional error up to " << opt.pruning << ")\n";

    size_t total[2] = {training_set->total(0), training_set->total(1)};

    // E_t current_error{A.error() + training_set->numInconsistent()};

    // cout << "cur: " << current_error << endl;

    // E_t target_error{static_cast<E_t>(static_cast<double>(total[0] +
    // total[1]) * (1.0 - opt.pruning))};

    // cout << "target: " << target_error << endl;

    // E_t limit{target_error - current_error};

    // // E_t limit{static_cast<E_t>(opt.pruning) - A.error() -
    // //           training_set->numInconsistent()};
    // if (target_error < current_error)
    //   limit = 0;

    // cout << "limit: " << limit << endl;

    sol = sol.prune(total, opt.pruning);

    E_t tree_error = 0;
    for (auto y{0}; y < 2; ++y) {
      auto X{(*training_set)[y]};
      for (auto i : X)
        tree_error += (sol.predict(X[i]) != y) * X.weight(i);
    }

    // double t{cpu_time() - start_time};

    double accuracy{
        1.0 -
        static_cast<double>(tree_error + training_set->numInconsistent()) /
            static_cast<double>(training_set->input_example_count())};

    cout << left << "d accuracy=" << setw(6) << setprecision(4)
         << fixedwidthfloat(accuracy, 4) << " error=" << setw(4)
         << tree_error + training_set->numInconsistent() << " depth=" << setw(3)
         << sol.depth() << " size=" << setw(3) << sol.size()
         // << " time=" << setprecision(max(4, static_cast<int>(log10(t))))
         // << fixedwidthfloat(t, 3) << right
         << endl;

    // cout << "after pruning: " << tree_error << endl;
    // cout << sol.size() << " " << sol.depth() << endl;
  }

  if (opt.tree_file != "") {
    ofstream treefile(opt.tree_file, ios_base::out);
    // treefile << A << endl;
    treefile << sol << endl;
  }

  if (opt.test_sample != 0) {

    E_t tree_error = 0;
    for (auto y{0}; y < 2; ++y) {
      auto X{(*test_set)[y]};
      for (auto i : X) {
        assert(X.weight(i) == 1);
        tree_error += (sol.predict(X[i]) != y) * X.weight(i);
      }
    }

    // assert(tree_error == A.error());

    cout << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
         << std::setw(0) << "d test_error=" << tree_error
         << " test_accuracy=" << setprecision(7)
         << 1.0 -
                static_cast<double>(tree_error) /
                    static_cast<double>(test_set->example_count())
         << endl;
  }

  if (opt.print_sol) {
    cout << sol << endl;
  }

  return 0;
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
