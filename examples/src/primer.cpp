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

#include "Cmdline.hpp"
#include "DataSet.hpp"
#include "CSVReader.hpp"

using namespace std;
using namespace primer;

int main(int argc, char* argv[]) {

  Options opt = parse(argc, argv);

  if (opt.print_cmd)
    cout << opt.cmdline << endl;

  if (opt.print_par)
    opt.display(cout);

  srand(opt.seed);

  DataSet base;

  csv::read<int>(opt.instance_file, [&](string &f) { base.addFeature(f); },
                 [&](vector<int> &data, const int e, const int v) {
                   if (v)
                     data.push_back(e);
                 },
                 [&](vector<int> &data, const int y) {
                   dynamic_bitset<> x;
                   x.resize(base.numFeature(), false);
                   for (auto v : data)
                     x.set(v);
                   x.resize(2 * base.numFeature(), true);
                   for (auto v : data)
                     x.reset(base.numFeature() + v);
                   base.add(x, y);
                 });

  base.feature_label.reserve(2 * base.numFeature());
  for (auto &f : base.feature_label) {
    string notf{"¬" + f};
    base.addFeature(notf);
  }

  if (opt.sample != 1) {
    std::mt19937 random_generator;
    base.uniform_sample(0, (double)(base.example[0].size()) * opt.sample,
                        random_generator);
    base.uniform_sample(1, (double)(base.example[1].size()) * opt.sample,
                        random_generator);
  }

  cout << base << endl << endl;

  base.computeDecisionSet(opt);

  cout << base << endl;
	
	base.verify();
}



