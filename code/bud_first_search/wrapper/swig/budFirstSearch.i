%module budFirstSearch
%include "std_vector.i"
%include "std_string.i"

%{
#include "budFirstSearch.h"
%}

extern DTOptions parse(std::vector<std::string> params);
extern void read_binary(primer::BacktrackingAlgorithm<IntegerError<int>, int> &A, DTOptions &opt);

namespace std {
  %template(int_vec) vector<int>;
  %template(cstr_vec) vector<char*>;
  %template(str_vec) vector<string>;
};

// DTOptions

class DTOptions {
public:
  std::string instance_file;
  std::string debug;
  std::string output;
  std::string format;

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

  template <class Error, class ErrorType>
  class BacktrackingAlgorithm {
  public:
    BacktrackingAlgorithm() = delete;
    BacktrackingAlgorithm(Wood &w, DTOptions &o);
    void minimize_error();
    void minimize_error_depth();
    void minimize_error_depth_size();
    Tree getSolution();
    void addExample(const std::vector<int> &example);
    void addExample(const std::vector<int> &example, int weight);
  };

  template <class ErrorType> class IntegerError;
  template <class ErrorType> class WeightedError;

  %template(BacktrackingAlgo) BacktrackingAlgorithm<IntegerError<int>, int>;
  %template(WeightedBacktrackingAlgo) BacktrackingAlgorithm<WeightedError<int>, int>;
  %template(WeightedBacktrackingAlgod) BacktrackingAlgorithm<WeightedError<double>, double>;
}
