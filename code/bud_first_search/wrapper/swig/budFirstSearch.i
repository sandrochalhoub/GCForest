%module budFirstSearch
%include "std_vector.i"
%include "std_string.i"

%{
#include "budFirstSearch.h"
%}

extern DTOptions parse(std::vector<std::string> params);
extern void read_binary(primer::BacktrackingAlgorithm<CardinalityError, int> &A, DTOptions &opt);

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
  int ada_it;
  int ada_stop;

  bool mindepth;
  bool minsize;

  bool use_weights;
  bool filter_inconsistent;


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

  template <template<typename> class ErrorPolicy, typename E_t>
  class BacktrackingAlgorithm {
  public:
    BacktrackingAlgorithm() = delete;
    BacktrackingAlgorithm(Wood &w, DTOptions &o);
    void minimize_error();
    void minimize_error_depth();
    void minimize_error_depth_size();
    Tree getSolution();
    void addExample(const std::vector<int> &example);
    void addExample(const std::vector<int> &example, E_t weight);
  };

  class Adaboost {
  public:
    DTOptions &options;

    Adaboost() = delete;
    Adaboost(DTOptions &opt);
    void train();
    bool predict(const std::vector<int> &example) const;
    void addExample(const std::vector<int> &example);
  };

  template <class ErrorType> class CardinalityError;
  template <class ErrorType> class WeightedError;

  %template(BacktrackingAlgo) BacktrackingAlgorithm<CardinalityError, int>;
  %template(WeightedBacktrackingAlgo) BacktrackingAlgorithm<WeightedError, int>;
  %template(WeightedBacktrackingAlgod) BacktrackingAlgorithm<WeightedError, double>;
}
