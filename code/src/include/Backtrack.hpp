
#include <iostream>
#include <random>
#include <vector>

#include "DataSet.hpp"
#include "Partition.hpp"
#include "SparseSet.hpp"

#ifndef _PRIMER_BACKTRACK_HPP
#define _PRIMER_BACKTRACK_HPP

using namespace boost;
using namespace std;

namespace primer {

/**********************************************
* BacktrackingAlgorithm
**********************************************/
/// Representation of a list of examples
class BacktrackingAlgorithm {

private:
  /*!@name Parameters*/
  //@{
  /// Argument
  DataSet &data;
  Options &options;
  vector<vector<int>> example[2];

  /// store the parent of node i
  vector<int> parent;

  /// store the parent of node i
  vector<int> depth;
	
  /// store the maximum depth at each level of the search tree
  vector<int> max_depth;

  /// -1 if the optimal subtree was not found yet, parent node if it was
  vector<int> optimal;

  // internal nodes are popped front, potential nodes and leaves are back
  SparseSet blossom;
	
  // // stores leaves (blossom nodes that cannot be expended because of depth constraints, or because we already explored subtrees where this node is expended)
  // SparseSet leaf;

  /// structure to partition the examples in the tree
  TreePartition P[2];

  /// buffers to compute the entropy, one copy per node, in order to backtrack
  vector<vector<int>> pos_feature_count[2];

  /// the list of features in the order they will be tried
  vector<vector<int>> ranked_feature;

  /// store the feature tested at node i (in ranked features)
  vector<vector<int>::iterator> feature;

  /// best solution
  vector<int> best_feature;
  vector<size_t> best_error;
  vector<size_t> best_size;

  // vector<size_t> st_trail;
  // vector<size_t> sz_trail;

  // vector<int> best_feature;
  vector<double> f_entropy;
  vector<size_t> buffer;

  mt19937 random_generator;

  size_t num_node;

  size_t ub_node;

  size_t ub_depth;

  size_t ub_error;

  size_t search_size;

  size_t num_backtracks;

  size_t num_restarts;

  void print_new_best() const;
  //@}

public:
  /*!@name Constructors*/
  //@{
  explicit BacktrackingAlgorithm(DataSet &d, Options &o);
  void resize(const int num_nodes);
  void seed(const int s);
  //@}

  size_t depth_lower_bound();

  size_t node_lower_bound();

  size_t error_lower_bound();

  /*!@name Accessors*/
  //@{
  size_t size();

  void setUbDepth(const size_t u);

  void setUbNode(const size_t u);

  void setUbError(const size_t u);

  // void setMaxNodes(const size_t m);

  void clear(int &node);

  double accuracy();

  size_t error();
	
	// whether we reached a leaf of the SEARCH tree
	// if we did and it's a solution, new best are stored
	bool dead_end(const int node);

  void set_optimal(const int node);

  void unset_optimal(const int node);

  bool is_optimal(const int node) const;

  bool no_feature(const int node) const;

  bool not_branched(const int node) const;

  // return true if the feature f is true/false in all examples
  bool max_entropy(const int node, const int f) const;

  // return true if the feature f classifies all examples
  bool null_entropy(const int node, const int f) const;

  void undo(const int node);

  // void undo(const int node);

  void backtrack(int &node);

  // void greedy(const int kbest, const double focus, const int m);

  void search(); // const int kbest, const double focus, const int m);

  int select();

  void random_perturbation(const int selected_node, const int kbest,
                           const int p);

  void sort_features(const int selected_node);

  void expend(const int selected_node);

  void split(const int node, const int feature);

  double entropy(const int node, const int feature);

  void count_by_example(const int node, const int y);

  void deduce_from_sibling(const int node, const int sibling, const int y);

  int get_feature_count(const int y, const int n, const int f) const;
  //@}

  /*!@name Printing*/
  //@{
  // std::ostream &toCsv(std::ostream &os) const;
  std::ostream &display(std::ostream &os) const;
  //@}
};


std::ostream &operator<<(std::ostream &os, const BacktrackingAlgorithm &x);
}

#endif // _PRIMER_BACKTRACK_HPP
