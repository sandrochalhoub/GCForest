#include "blossomWrapper.h"

#include <iostream>

#include "CSVReader.hpp"
#include "TXTReader.hpp"

using namespace blossom;

/*
void addNode(Tree &tree, int node, Results &res) {
  if (node <= 1) {
    // add node
    res.nodes.push_back({true, node});
  }
  else {
    int id = res.nodes.size();
    res.nodes.push_back({false, tree.getFeature(node)});

    // add edge 1
    int node0 = tree.getChild(node, 0);
    res.edges.push_back({id, res.nodes.size(), 0});
    addNode(tree, node0, res);

    // add edge 2
    int node1 = tree.getChild(node, 1);
    res.edges.push_back({id, res.nodes.size(), 1});
    addNode(tree, node1, res);
  }
}
*/

DTOptions parse(std::vector<std::string> params) {
  std::vector<char*> cparams;
  for (auto &param : params) {
    cparams.push_back(const_cast<char*>(param.c_str()));
  }

  return parse_dt(cparams.size(), &cparams[0]);
}

// void read_binary(BacktrackingAlgorithm<> &A, DTOptions &opt) {
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

void read_binary(WeightedDataset &input, DTOptions &opt) {
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
