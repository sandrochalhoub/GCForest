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

  // if (opt.verbosity >= Options::QUIET)
  //   cout << "c filter base\n";
  //
  // base.filter();

  // base.computeBounds();

  // exit(1);

  if (opt.verbosity >= Options::QUIET)
    if (base.count() < count)
      cout << "c filtered " << (count - base.count()) / 2
           << " noisy example(s)\n";

  BacktrackingAlgorithm A(base, opt);

	// DL8 A(base, opt);

  Tree T;

  if (opt.debug != "") {
    ifstream ifs(opt.debug, ios_base::in);

    int i, f, c, k{0};
    while (true) {
      ifs >> i;
      if (not ifs.good())
        break;

      assert(i == k++);

      ifs >> f;
      if (f < 0)
        T.addNode(-1, f);
      else {
        ifs >> c;
        T.addNode(c, f);
        ifs >> c;
      }
    }
    //
    //
    // cout << "debug!\n";
    //
    // cout << T << endl;
    //
    // exit(1);

    A.debug_sol = &T;
  }

  // cout << opt.depth << endl;

  // A.resize(3);
  // A.seed(opt.seed);

  //
  // for(auto f{0}; f<base.numFeature(); ++f)
  // {
  // 	 cout << f << " " << A.entropy(0, f) << endl;
  // }
  //
  // cout << A << endl;
  //
  // A.split(0,4);

  // for (auto c{0}; c < 2; ++c) {
  //   for (auto i{0}; i < base.example[c].size(); ++i) {
  //     if (base.ithHasFeature(c, i, 53) != base.ithHasFeature(c, i, 54))
  //       cout << "diff! " << c << base.ithHasFeature(c, i, 53)
  //            << base.ithHasFeature(c, i, 54) << "\n";
  //     // else
  //     // 	cout << "eq.\n";
  //   }
  // }

  // exit(1);

  // cout << A << endl;

  // auto min_size{static_cast<size_t>(-1)};
  // double max_accuracy{0.0};

  // for (auto i{0}; i < opt.max_iteration; ++i) {
  // A.greedy(opt.width, opt.focus, opt.max_size);
  // A.new_search(); // opt.width, opt.focus, opt.max_size);

	// A.optimize();

  A.search();

  // cout << A.size() << endl;

  // if (A.accuracy() > max_accuracy or
  //     (A.accuracy() == max_accuracy and A.size() < min_size)) {
  //   cout << setw(4) << A.size() << " " << setw(10) << A.accuracy() << endl;
  //   min_size = A.size();
  //   max_accuracy = A.accuracy();
  //   if (max_accuracy == 1)
  //     A.setUb(min_size);
  // }
  //
  // A.clear();
  // }
  // A.expend();
  //
  // cout << A << endl;

  if (opt.print_sol or opt.verified) {
    Tree T;
    for (auto i{0}; i < A.solutionTreeSize(); ++i) {
      T.addNode(A.solutionChild(i, true), A.solutionFeature(i));
    }

    auto e{T.predict(base)};

    if (opt.print_sol) {
      cout << T << endl
           << e << "/" << base.count() << " ("
           << 1.0 - static_cast<double>(e) /
           static_cast<double>(base.count())
           << ")\n";
    }

    if (opt.output != "") {
      ofstream ofs(opt.output, ios_base::out);
      ofs << T << endl;
    }

    assert(e == A.solutionError());
  }
}



