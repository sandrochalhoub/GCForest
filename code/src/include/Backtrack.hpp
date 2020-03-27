
#include <iostream>
#include <random>
#include <vector>

#include "DataSet.hpp"
#include "Partition.hpp"
#include "SparseSet.hpp"
#include "Tree.hpp"
#include "utils.hpp"

// #define PRINT_TRACE print_trace();
// #define DO_ASSERTS do_asserts();
#define PRINT_TRACE
#define DO_ASSERTS

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
  Wood &wood;
  DataSet &data;
  DTOptions &options;
  vector<vector<int>> example[2];

  /// store the children of node i
  vector<int> child[2];

  /// store the parent of node i
  vector<int> depth;

  /// store the maximum depth at each level of the search tree
  vector<int> max_depth;

  /// -1 if the optimal subtree was not found yet, parent node if it was
  vector<int> optimal;

  // internal nodes are popped front, potential nodes are back
  SparseSet blossom;

  /// structure to partition the examples in the tree
  TreePartition P[2];

  /// decision nodes in sequence
  vector<int> decision;

  /// buffers to compute the entropy, one copy per node, in order to backtrack
  vector<vector<int>> pos_feature_count[2];

  /// the list of features in the order they will be tried
  vector<vector<int>> ranked_feature;

  /// store the feature tested at node i (in ranked features)
  vector<vector<int>::iterator> feature;

  // Stores the root of the best subtree found for the current feature of the
  // parent node
  // if the node is not optimal, it points to the optimal trees of the childrens
  // given its root-feature
  // therefore, *optimal* best trees should not be freed when pruning the node
  // however, they can be freed when replacing the current best.
  vector<int> best_tree;

  vector<size_t> best_error;
  // vector<size_t> best_size;

  vector<int> f_error;

  mt19937 random_generator;

  size_t num_node;

  size_t ub_node;

  size_t ub_depth;

  size_t ub_error;

  size_t search_size;
  size_t search_limit;

  size_t num_backtracks;

  size_t num_restarts;

  size_t current_error;

  double time_limit;

  double restart_base;

  int restart_limit;

  int backtrack_node;

  int solution_root;
	
	int checking_period;

  void store_new_best();

  int copy_solution(const int node);
  //@}

#ifdef PRINTTRACE
  void print_trace();
  void do_asserts();
  // returns the real error for "leaf" nodes (deepest test), and node_error
  // otherwise
  size_t leaf_error(const int i) const;
#endif

  // resize the data structures for up to k nodes
  void resize(const int k);

  // current size of the data structures (current node capacity)
  size_t size();

  // return true if all features have been tried feature
  bool no_feature(const int node) const;

  // return true if the feature f reduces the error
  bool reduce_error(const int node, const int f) const;

  // return true if the feature f is true/false in all examples
  bool max_entropy(const int node, const int f) const;

  // return true if the feature f classifies all examples
  bool null_entropy(const int node, const int f) const;

  // swap a random feature among the k best for node i with probability p/1000
  void random_perturbation(const int i, const int k, const int p);

	// sort the features by minimum projected error (leave 1-entropy node at the)
  void sort_features(const int selected_node);

	// compute the conditional entropy of feature at node
  double entropy(const int node, const int feature);

	// count, for every feature, the number of examples of class y at node with that feature
  void count_by_example(const int node, const int y);

	// deduce, for every feature, the number of examples of class y at node without that feature
  void deduce_from_sibling(const int parent, const int node, const int sibling,
                           const int y);

  // returns the error if testing feature f at node i (f must be in [1,m],
  // m=data.numFeature)
  int get_feature_error(const int i, const int f) const;

	// returns the number of examples of class y having feature f (f + m represents not-f)
  int get_feature_count(const int y, const int i, const int f) const;

	// returns the error if we do not test any feature on node i
  size_t node_error(const int i) const;

	// select the most promising node to branch on
  int choose() const;

	// whether node is a leaf (deepest test)
  bool isLeaf(const int node) const;

	// save the current subtree of node as best tree
  void store_best_tree(const int node, const bool global);

  // lower bound, maybe?
  bool fail();
	
	// as name suggests
  bool notify_solution();
	
	// remove the node and its descendants from the tree
  void prune(const int node);
	
	// undo the last decision and remove the previous feature as possible choice
  bool backtrack();
	
	// set c as the branch-child of node (branch is 0/1 i.e. left/right)
  void setChild(const int node, const bool branch, const int c);
	
	// branch on node by testing f
  void branch(const int node, const int f);
	
	// select a node to branch on, the feature to test and create the children
  void expend();

  //
  void grow(const int node);

  void restart();

  bool limit_out();

public:
  /*!@name Constructors*/
  //@{
  explicit BacktrackingAlgorithm(DataSet &d, Wood &w, DTOptions &o);
  void seed(const int s);
  //@}

  void separator(const string &msg) const;
  void print_new_best() const;

  void setUbDepth(const size_t u);

  void setUbNode(const size_t u);

  void setUbError(const size_t u);

  void setTimeLimit(const double t);

  void setSearchLimit(const size_t t);

  void search();

  TreeNode getSolution();

  int error() const;

  /*!@name Printing*/
  //@{
  // std::ostream &toCsv(std::ostream &os) const;
  std::ostream &display(std::ostream &os) const;
  //@}
};

std::ostream &operator<<(std::ostream &os, const BacktrackingAlgorithm &x);
}

#endif // _PRIMER_BACKTRACK_HPP
