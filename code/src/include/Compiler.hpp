
#ifndef _BLOSSOM_COMPILER_HPP
#define _BLOSSOM_COMPILER_HPP

#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "CmdLine.hpp"
#include "Partition.hpp"
#include "SparseSet.hpp"
#include "Tree.hpp"
#include "utils.hpp"

// #define PRINT_TRACE
#define PRINT_TRACE print_trace();

using namespace boost;
using namespace std;

namespace blossom {

int lbLeaf(const uint64_t P, const int E);

/**********************************************
* Compiler
**********************************************/
template <typename E_t> class Compiler {

public:
  /*!@name Parameters*/
  //@{
  /// Memory management for storing trees
  Wood<E_t> wood;
  int solution_root{-1};

  /// Command line options
  DTOptions &options;

  int num_feature;

  // data set
  vector<vector<int>> example;
  vector<dynamic_bitset<>> reverse_dataset;

  /// structure to partition the examples in the tree
  TreePartition P;

  vector<E_t> f_error;
  vector<double> f_entropy;
  vector<double> f_gini;

  vector<vector<E_t>> pos_feature_frequency;

  int num_leaf;
  int numLeaf(const int node) const { return (node >= 0 ? best[node] : 1); }
	int minLeaf(const int node) const { return (node >= 0 ? lb[node] : 1); }
  int currentSize() const { return 2 * (num_leaf + 3 * blossom.count()) - 1; }

  /// store the children of node i
  vector<int> child[2];

  /// store the parent of node i
  vector<int> parent;

  /// store the depth of node i
  vector<int> depth;

  // internal nodes are popped front, potential nodes are back
  SparseSet blossom;

  /// decision nodes in sequence
  vector<int> decision;

  /// the list of features in the order they will be tried
  vector<vector<int>> ranked_feature;
  /// store the feature tested at node i (in ranked features)
  vector<vector<int>::iterator> feature;
  /// domain
  vector<vector<int>::iterator> end_feature;

  // smallest tree under that branch so far
  vector<int> best;
  vector<int> lb;
	vector<int> best_feature;

  // root of the best tree for this node so far
  vector<int> tree;

  double start_time;

  int min_depth_backtrack;

  int ub_size() const { return best[0]; }

  size_t search_size;

  dynamic_bitset<> branch_features;

  inline int log_size(const int node) const {
    return (num_feature - depth[node]);
  }

  E_t usize(const int node) const { return 1 << log_size(node); }

  bool purePositive(const int node) const;

  E_t halfsize(const int node) const {
		E_t p{1};
    return p << (log_size(node) - 1);
  }
	
	bool smallEnough(const int node) const {
		return (log_size(node) - 1) < std::numeric_limits<E_t>::digits;
	}
  //@}

  // resize the data structures for up to k nodes
  void resize(const int k);

  // current size of the data structures (current node capacity)
  size_t size();

  // return true if all features have been tried feature
  bool no_feature(const int node) const;

  // sort the features by minimum projected error (leave 1-entropy node at the)
  void sort_features(const int node);
  template <class property> void filter_features(const int node, property cond);

  // select the most promising node to branch on
  int highest_error() const;
  int highest_error_reduction() const;

  // remove the node and its descendants from the tree
  void prune(const int node);

  // undo the last decision and remove the previous feature as possible choice
  bool backtrack();
	
  // computes a lower bound on the size that one can get without changing any decision of the current branch (of the DT!!!)
  bool fail();

  // change the best (minimum) tree size below node, and recursively update
  // parents
  void updateTree(const int node);
  void updateBest(const int node, const bool terminal = true);
  int buildTree(const int node, const int depth);
  void storeSolution();

  // branch on node by testing f
  void branch(const int node, const int f);

  // select a node to branch on, the feature to test and create the children
  void expend();

  // returns true if this is a pseudo-leaf
  bool grow(const int node);
	
	// returns true if this is not a leaf
  bool setChild(const int node, const bool branch, const int c);

  void initialise_search();

  bool is_optimal(const int node, const int f) const;

public:
  /*!@name Constructors*/
  //@{
  explicit Compiler(DTOptions &o);
  // void setData(const DataSet &data);
  void setReverse();
  void addExample(const instance &x);
  //@}

  size_t numExample() const;
  int numFeature() const;

  void separator(const string &msg) const;
  void print_new_best();
  void print_progress();

  void search();

  // E_t error() const;
  E_t node_error(const int node) const;

  void count_by_example(const int node);
  void deduce_from_sibling(const int parent, const int node, const int sibling);

  E_t get_feature_frequency(const int n, const int f) const;
  E_t get_feature_error(const int n, const int f) const;

  /*!@name Printing*/
  //@{
  // std::ostream &toCsv(std::ostream &os) const;
  std::ostream &display(std::ostream &os) const;

#ifdef PRINTTRACE
  void print_trace();
  void ptrace();
#endif
  //@}
};

template <typename E_t>
void Compiler<E_t>::addExample(const dynamic_bitset<> &sample) {
  int n{static_cast<int>(sample.size())};

  if (n > num_feature) {
    num_feature = n;
    branch_features.resize(n, 0);
    f_error.resize(num_feature, 1);
    f_entropy.resize(num_feature, 1);
    f_gini.resize(num_feature, 1);
  }

  example.resize(example.size() + 1);

  for (auto f{0}; f < n; ++f)
    if (sample[f]) {
      example.back().push_back(f);
    }
}

template <typename E_t>
template <class property>
inline void Compiler<E_t>::filter_features(const int node, property cond) {
  for (auto f{end_feature[node] - 1}; f >= feature[node]; --f) {
    if (cond(*f)) {
      swap(*f, *(--end_feature[node]));
    }
  }
}

template <typename E_t>
std::ostream &operator<<(std::ostream &os, const Compiler<E_t> &x);
}

#endif // _BLOSSOM_COMPILER_HPP
