
#include <iostream>
#include <random>
#include <stdlib.h>

#include "WeightedDataset.hpp"
#include "Backtrack.hpp"
#include "Adaboost.hpp"
#include "CSVReader.hpp"
#include "CmdLine.hpp"
#include "TXTReader.hpp"
#include "Tree.hpp"

using namespace std;
using namespace blossom;


template<typename E_t>
void read_binary(WeightedDataset<E_t> &input, DTOptions &opt) {
  // WeightedDataset input;

  string ext{opt.instance_file.substr(opt.instance_file.find_last_of(".") + 1)};

  auto target_column{-1};

  if (opt.format != "guess")
    target_column = opt.intarget;

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

  // input.toInc(A);
}
// template <typename Algo_t>
// void read_binary(Algo_t &A, DTOptions &opt) {
//
//   string ext{opt.instance_file.substr(opt.instance_file.find_last_of(".") + 1)};
//
//   if (opt.format == "csv" or (opt.format == "guess" and ext == "csv")) {
//     csv::read_binary(opt.instance_file, [&](vector<int> &data) {
//       A.addExample(data.begin(), data.end() - 1, data.back());
//     });
//   } else if (opt.format == "dl8" or (opt.format == "guess" and ext == "dl8")) {
//     txt::read_binary(opt.instance_file, [&](vector<int> &data) {
//       auto y = *data.begin();
//       A.addExample(data.begin() + 1, data.end(), y);
//     });
//   } else {
//     if (opt.format != "txt" and ext != "txt")
//       cout << "p Warning, unrecognized format, trying txt\n";
//     txt::read_binary(opt.instance_file, [&](vector<int> &data) {
//       A.addExample(data.begin(), data.end() - 1, data.back());
//     });
//   }
// }

int main(int argc, char *argv[]) {
  DTOptions opt = parse_dt(argc, argv);

  if (opt.print_cmd)
    cout << opt.cmdline << endl;

  if (opt.print_par)
    opt.display(cout);

  Adaboost A(opt);
  WeightedDataset<int> input;
	read_binary(input, opt);
	input.preprocess(opt.verbosity >= DTOptions::NORMAL);
	input.setup(A);
	
  // read_binary(A, opt);

  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d readtime=" << cpu_time() << endl;

  A.preprocess();
  // TODO choose what to minimize?
  A.train();
}
