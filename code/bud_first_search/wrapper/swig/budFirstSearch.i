%module budFirstSearch
%include "std_vector.i"
%include "std_string.i"

%{
#include "budFirstSearch.h"
%}

struct Example {
  std::vector<int> features;
  int target;

  Example();
  Example(std::vector<int> features, int target);
};

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

extern void free(void* ptr);

namespace std {
  %template(node_vec) vector<Node>;
  %template(edge_vec) vector<Edge>;
  %template(example_vec) vector<Example>;

  %template(int_vec) vector<int>;
  %template(cstr_vec) vector<char*>;
  %template(str_vec) vector<string>;
};

// DTOptions

class DTOptions {
public:
  DTOptions();
};

namespace primer {
  // Tree

  class Wood {
  public:
    Wood();
  };

  class Tree {
  public:
    int idx;

    Tree() = delete;
    Tree(Wood*, int i);
    int getChild(int node, int branch);
    int getFeature(int node);
  };

  // BacktrackingAlgorithm

  class BacktrackingAlgorithm {
  public:
    BacktrackingAlgorithm() = delete;
    BacktrackingAlgorithm(Wood &w, DTOptions &o);
    void minimize_error();
    void minimize_error_depth();
    void minimize_error_depth_size();
    Tree getSolution();
  };
}
