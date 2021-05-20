#include "blossom.h"

#include <iostream>

#include "CSVReader.hpp"
#include "TXTReader.hpp"

using namespace blossom;


blossom::DTOptions parse(std::vector<std::string> params) {
  std::vector<char*> cparams;
  for (auto &param : params) {
    cparams.push_back(const_cast<char*>(param.c_str()));
  }

  return parse_dt(cparams.size(), &cparams[0]);
}


void read_binary(WeightedDataset<int> &input, DTOptions &opt) {
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
