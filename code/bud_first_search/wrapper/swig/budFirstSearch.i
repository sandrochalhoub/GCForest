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

extern void addExamples(primer::BacktrackingAlgorithm &algo, std::vector<Example> data);
extern DTOptions parse(std::vector<std::string> params);

extern void free(void* ptr);

namespace std {
  %template(example_vec) vector<Example>;

  %template(int_vec) vector<int>;
  %template(cstr_vec) vector<char*>;
  %template(str_vec) vector<string>;
};

// DTOptions

class DTOptions {
public:
  int verbosity;

  int seed;

  bool print_sol;
  bool print_par;
  bool print_ins;
  bool print_sta;
  bool print_cmd;

  bool verified;

  double sample;

  int width;
  double focus;

  int max_depth;

  int restart_base;
  double restart_factor;

  bool filter;

  double time;

  int search;

  bool bounding;

  int node_strategy;

  int feature_strategy;

  bool binarize;

  bool mindepth;
  bool minsize;


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
