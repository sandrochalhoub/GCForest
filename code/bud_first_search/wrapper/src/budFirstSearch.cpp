#include "budFirstSearch.h"

#include <iostream>

using namespace primer;

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

Results search(std::vector<std::string> params, std::vector<Example> data) {
  /*
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
  */

  std::vector<char*> cparams;
  for (auto &param : params) {
    cparams.push_back(const_cast<char*>(param.c_str()));
  }

  DTOptions opt = parse_dt(cparams.size(), &cparams[0]);

  Wood yallen;
  BacktrackingAlgorithm A(yallen, opt);

  for (Example &example: data) {
    A.addExample(example.features.begin(), example.features.end(), example.target);
  }

  // Solve
  if (opt.mindepth) {
    if (opt.minsize)
      A.minimize_error_depth_size();
    else
      A.minimize_error_depth();
  } else
    A.minimize_error();

  Tree sol = A.getSolution();
  std::cout << sol << std::endl;
  // Building result object
  Results res;
  addNode(sol, sol.idx, res);
  return res;
}

void addExamples(primer::BacktrackingAlgorithm &algo, std::vector<Example> data) {
  for (Example &example: data) {
    algo.addExample(example.features.begin(), example.features.end(), example.target);
  }
}

DTOptions parse(std::vector<std::string> params) {
  std::vector<char*> cparams;
  for (auto &param : params) {
    cparams.push_back(const_cast<char*>(param.c_str()));
  }

  return parse_dt(cparams.size(), &cparams[0]);
}

void free(void *ptr) {
  delete ptr;
}