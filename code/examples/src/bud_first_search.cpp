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
#include "WeightedDataset.hpp"

using namespace std;
using namespace primer;

template <typename Algo_t>
void read_non_binary(Algo_t &A, DTOptions &opt) {

  TypedDataSet input;

  string ext{opt.instance_file.substr(opt.instance_file.find_last_of(".") + 1)};

  auto target_column{-1};

  if (opt.format == "csv" or (opt.format == "guess" and ext == "csv"))
    csv::read(opt.instance_file,
              [&](vector<string> &f) {
                input.setFeatures(f.begin(), f.end(), target_column);
              },
              [&](vector<string> &data) {
                input.addExample(data.begin(), data.end(), target_column);
              });
  else {

    if (opt.format == "dl8" or (opt.format == "guess" and ext == "dl8")) {
      target_column = 0;
    }

    txt::read(opt.instance_file, [&](vector<string> &data) {
      input.addExample(data.begin(), data.end(), target_column);
    });
  }

	WeightedDataset base;

  input.binarize(base);
	
	base.toInc(A);
}


template <typename Algo_t> void read_binary(Algo_t &A, DTOptions &opt) {
  WeightedDataset input;

  string ext{opt.instance_file.substr(opt.instance_file.find_last_of(".") + 1)};

  auto target_column{-1};

  if (opt.format == "csv" or (opt.format == "guess" and ext == "csv")) {
    csv::read_binary(opt.instance_file, [&](vector<int> &data) {
      input.addExample(data.begin(), data.end(), target_column);
    });
  } else {

    if (opt.format == "dl8" or (opt.format == "guess" and ext == "dl8")) {
      target_column = 0;
    }

    txt::read_binary(opt.instance_file, [&](vector<int> &data) {
      input.addExample(data.begin(), data.end(), target_column);
    });
  }

  input.toInc(A);
}

template <template <typename> class ErrorPolicy = CardinalityError,
          typename E_t = unsigned long>
int run_algorithm(DTOptions &opt) {
  Wood yallen;

  BacktrackingAlgorithm<ErrorPolicy, E_t> A(yallen, opt);

  if (opt.binarize) {

    read_non_binary(A, opt);

  } else {

    read_binary(A, opt);

  } 

  if (opt.print_ins) {
    if (opt.nosolve) {
      // A.setReverse();
      if (opt.output != "") {
        ofstream outfile(opt.output.c_str(), ofstream::out);
        A.printDatasetToFile(outfile);
      } else {
        A.printDatasetToFile(cout);
      }
    } else {
      cout << "d examples=" << A.numExample() << " features=" << A.numFeature()
           << endl;
    }
  }

  // if (not opt.preprocessing and opt.verbosity >= DTOptions::NORMAL)
  cout << "d readtime=" << cpu_time() << endl;

  if (not opt.nosolve) {
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

      E_t tree_error = sol.predict<E_t>(
          A.dataset[0].begin(), A.dataset[0].end(), A.dataset[1].begin(),
          A.dataset[1].end(), [&](const int y, const size_t i) {
            return A.error_policy.get_weight(y, i);
          });

      assert(tree_error == A.error());

      cout << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
           << std::setw(0) << "p solution verified (" << tree_error << " / "
           << A.error() << ")" << endl;
    }
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
