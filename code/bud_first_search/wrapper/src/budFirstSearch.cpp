#include "budFirstSearch.h"

#include <iostream>
/*
#include "Backtrack.hpp"
#include "CmdLine.hpp"
#include "Tree.hpp"*/

Results search(std::vector<std::string> params, std::vector<Example> data) {
  /*DTOptions opt = parse_dt(params.size(), &params[0]);

  Wood yallen;
  BacktrackingAlgorithm A(yallen, opt);

  if (opt.mindepth) {
    if (opt.minsize)
      A.minimize_error_depth_size();
    else
      A.minimize_error_depth();
  } else
    A.minimize_error();

  Tree sol = A.getSolution();

  for (Example &example: data) {
    A.addExample(data.features.begin(), data.features.end(), data.target);
  }*/

  std::cout << "size of params: " << params.size() << ", size of data: " << data.size() << std::endl;

  for (auto &a : params) {
    std::cout << a << std::endl;
  }

  for (auto &b : data) {
    std::cout << "example: ";

    for (int u : b.features) {
      std::cout << u << " ";
    }
    std::cout << b.target << std::endl;
  }

  // Building result object
  Results res;
  res.nodes.push_back(Node{true, 0});
  res.edges.push_back(Edge{0, 0, 0});
  return res;
}