#include <vector>
#include <string>

#include "Tree.hpp"
#include "Backtrack.hpp"
#include "CmdLine.hpp"

#define SWIG_FILE_WITH_INIT

// Dataset

struct Example {
  std::vector<int> features;
  int target;

  Example() {}

  Example(std::vector<int> features, int target)
    : features(features), target(target) {}
};

// Tree

struct Node {
  bool leaf;
  int feat;
};

struct Edge {
  int parent;
  int child;
  int val;
};

struct Results {
  std::vector<Node> nodes;
  std::vector<Edge> edges;
};

extern Results search(std::vector<std::string> params, std::vector<Example> data);
extern void addExamples(primer::BacktrackingAlgorithm &algo, std::vector<Example> data);
extern DTOptions parse(std::vector<std::string> params);

extern void free(void *ptr);
