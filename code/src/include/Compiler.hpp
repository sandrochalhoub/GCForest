
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "CmdLine.hpp"
#include "Partition.hpp"
#include "SparseSet.hpp"
#include "Tree.hpp"
#include "utils.hpp"

#ifndef _PRIMER_COMPILER_HPP
#define _PRIMER_COMPILER_HPP

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
  // friend ErrorPolicy<E_t>;

  /*!@name Parameters*/
  //@{
  /// Memory management for storing trees
  // Wood &wood;

  /// Command line options
  DTOptions &options;

  int num_feature;

  // data set
  vector<vector<int>> example;
  // vector<instance> dataset[N];
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

  // int minLeaf(const int node) ;
  // {
  //   assert(blossom.contain(node));
  // 		// should look only to the features with max and min counts
  //   for (auto f{feature[node]}; f != end_feature[node]; ++f) {
  //     auto count{pos_feature_frequency[node][*f]};
  //     if (count >= P[node].count() or count == 0 or count == halfsize(node)
  //     or
  //         count == (P[node].count() - halfsize(node)))
  //       return 3;
  //   }
  //   return 4;
  // }

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

  double start_time;

  int min_depth_backtrack;

  // int ub_size;
  int ub_size() const { return best[0]; }

  size_t search_size;

  dynamic_bitset<> branch_features;

  inline int log_size(const int node) const {
    return (num_feature - depth[node]);
  }

  E_t usize(const int node) const { return 1 << log_size(node); }

  bool purePositive(const int node) const;

  E_t halfsize(const int node) const {
    return 1 << (num_feature - depth[node] - 1);
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

  // as name suggests
  bool solutionFound();

  // undo the last decision and remove the previous feature as possible choice
  bool backtrack();
	
  // computes a lower bound on the size that one can get without changing any decision of the current branch (of the DT!!!)
  bool fail();
	
	// change the best (minimum) tree size below node, and recursively update parents
	void updateBest(const int node);

  // branch on node by testing f
  void branch(const int node, const int f);

  // select a node to branch on, the feature to test and create the children
  void expend();

  // returns true if this is a pseudo-leaf
  bool grow(const int node);
	// returns true if this is not a leaf
  bool setChild(const int node, const bool branch, const int c);

  void initialise_search();

public:
  /*!@name Constructors*/
  //@{
  explicit Compiler(DTOptions &o);
  // void setData(const DataSet &data);
  void setReverse();
  template <class rIter>
  void addExample(rIter beg_sample, rIter end_sample, const bool y,
                  const E_t weight = 1);
  void addExample(const std::vector<int> &example, const E_t weight = 1);
  //@}

  size_t numExample() const;
  int numFeature() const;

  void separator(const string &msg) const;
  void print_new_best();
  void print_progress();

  // int getUbSize() const;

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
#endif
  //@}
};

template <typename E_t>
template <class rIter>
inline void Compiler<E_t>::addExample(rIter beg_sample, rIter end_sample,
                                      const bool y, const E_t weight) {

  if (y != 0) {
    return;
  }

  int n{static_cast<int>(end_sample - beg_sample)};

  if (n > num_feature) {
    num_feature = n;
    branch_features.resize(n, 0);
    f_gini.resize(num_feature, 1);
    f_entropy.resize(num_feature, 1);
    f_error.resize(num_feature, 1);
  }

  // dataset.resize(dataset[y].size() + 1);
  example.resize(example.size() + 1);
  // dataset.back().resize(num_feature, false);

  int k{0};
  for (auto x{beg_sample}; x != end_sample; ++x) {
    if (*x) {

      if (*x != 1) {
        cout << "e the dataset is not binary, "
                "rerun with --binarize\n";
        exit(1);
      }

      // dataset[y].back().set(k);
      example.back().push_back(k);
    }
    ++k;
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

#endif // _PRIMER_COMPILER_HPP
