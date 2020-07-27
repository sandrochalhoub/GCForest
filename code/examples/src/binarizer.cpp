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

#include "TXTReader.hpp"
#include "CSVReader.hpp"
#include "CmdLine.hpp"
#include "TypedDataSet.hpp"

using namespace std;
using namespace primer;

int main(int argc, char* argv[]) {
    DTOptions opt = parse_dt(argc, argv);

    if (opt.print_cmd)
        cout << opt.cmdline << endl;

    if (opt.print_par)
        opt.display(cout);

    TypedDataSet input;

    string ext{opt.instance_file.substr(opt.instance_file.find_last_of(".") + 1)};

    if (opt.format == "csv" or (opt.format == "guess" and ext == "csv"))
      csv::read(
          opt.instance_file,
          [&](vector<string> &f) { input.setFeatures(f.begin(), f.end() - 1); },
          [&](vector<string> &data) {
            auto y = data.back();
            data.pop_back();
            input.addExample(data.begin(), data.end(), y);
          });
    else if (opt.format == "dl8" or (opt.format == "guess" and ext == "dl8")) {
      txt::read(opt.instance_file, [&](vector<string> &data) {
        auto y = *data.begin();
        input.addExample(data.begin() + 1, data.end(), y);
      });
    } else {
      if (opt.format != "txt" and ext != "txt")
        cout << "p Warning, unrecognized format, trying txt\n";

      txt::read(opt.instance_file, [&](vector<string> &data) {
        auto y = data.back();
        data.pop_back();
        input.addExample(data.begin(), data.end(), y);
      });
    }

    if (opt.print_sol) {
        cout << input << endl;
    }

    DataSet bin;

    input.binarize(bin);

    // TODO maybe use levels of verbosity ? output file ?
    if (opt.print_sol) {
        cout << bin << endl;
    }
    else {
        std::string delimiter(",");
        std::string wildcard("\n");
        bin.write(cout, delimiter, wildcard, true, true, false);
    }
}
