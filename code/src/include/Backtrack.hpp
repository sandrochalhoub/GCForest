
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
  // DataSet &data;
  int num_feature;
  // size_t numExample() const;
  // size_t numExample[2];

  DTOptions &options;
  vector<vector<int>> example[2];

  /// store the children of node i
  vector<int> child[2];

  /// store the parent of node i
  vector<int> parent;

  /// store the depth of node i
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
  vector<vector<int>> pos_feature_frequency[2];

  /// the list of features in the order they will be tried
  vector<vector<int>> ranked_feature;
  vector<vector<int>::iterator> end_feature;

  /// store the feature tested at node i (in ranked features)
  vector<vector<int>::iterator> feature;

  // Stores the root of the best subtree found for the current feature of the
  // parent node
  // if the node is not optimal, it points to the optimal trees of the childrens
  // given its root-feature
  // therefore, *optimal* best trees should not be freed when pruning the node
  // however, they can be freed when replacing the current best.
  vector<int> best_tree;

  // this is because I'm stupid
  vector<size_t> tree_error;

  // optimistic value. updated only when the node becomes optimal
  vector<size_t> min_error;

  // best value for any possible feature, given the current branch (ancestors)
  // get updated when backtracking from the decision on the current feature
  vector<size_t> max_error;
  vector<size_t> max_size;

  vector<int> f_error;
  vector<double> f_entropy;
  vector<double> f_gini;
  // vector<double> f_gini_d;

  mt19937 random_generator;

  size_t num_node;

  size_t ub_node;

  size_t ub_depth;

  size_t ub_error;

  size_t search_size;
  size_t search_limit;

  size_t num_solutions;

  size_t num_backtracks;

  size_t num_restarts;

  size_t current_error;

  double time_limit;

  double restart_base;

  int restart_limit;

  int backtrack_node;

  int solution_root;

  int checking_period;

  bool interrupted;

  double start_time;

  // bool use_entropy;
  int feature_criterion;

  dynamic_bitset<> buffer[2];

  // dynamic_bitset<> neg_buffer;

  void cleaning();

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
  void sort_features(const int node);

  void filter_features(const int node);

  // compute the conditional entropy of feature at node
  double entropy(const int node, const int feature);

  double gini(const int node, const int feature);

  // count, for every feature, the number of examples of class y at node with
  // that feature
  void count_by_example(const int node, const int y);

  // deduce, for every feature, the number of examples of class y at node
  // without that feature
  void deduce_from_sibling(const int parent, const int node, const int sibling,
                           const int y);

  // returns the error if testing feature f at node i (f must be in [1,m],
  // m=data.numFeature)
  int get_feature_error(const int i, const int f) const;

  // returns the number of examples of class y having feature f (f + m
  // represents not-f)
  int get_feature_frequency(const int y, const int i, const int f) const;

  // returns the error if we do not test any feature on node i
  size_t node_error(const int i) const;

  // select the most promising node to branch on
  int highest_error() const;
  int highest_error_reduction() const;

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

  bool update_upperbound(const int node);

  // undo the last decision and remove the previous feature as possible choice
  bool backtrack();

  // set c as the branch-child of node (branch is 0/1 i.e. left/right)
  void setChild(const int node, const bool branch, const int c);

  // branch on node by testing f
  void branch(const int node, const int f);

  // select a node to branch on, the feature to test and create the children
  void expend();

  //
  bool grow(const int node);

  void restart();

  bool limit_out();

  void initialise_search();

public:
	
	vector<instance> dataset[2];
        vector<dynamic_bitset<>> reverse_dataset[2];

        /*!@name Constructors*/
        //@{
        explicit BacktrackingAlgorithm(Wood &w, DTOptions &o);
        void setData(const DataSet &data);
        void setReverse();
        void seed(const int s);
        //@}

        size_t numExample() const;
        size_t numFeature() const;

        // whether
        bool dominate(const int f_a, const int f_b) const;

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

        template <class rIter>
        void addExample(rIter beg_sample, rIter end_sample, const bool y) {
          int n{static_cast<int>(end_sample - beg_sample)};
          // for(auto i{0}; i<2; ++i)
          // 	numExample[i] = data.example[i].count();

          if (n > num_feature) {
            num_feature = n;
            //
            // auto m{num_feature};
            f_error.resize(num_feature, 1);
            f_entropy.resize(num_feature, 1);
            f_gini.resize(num_feature, 1);
            // f_gini_d.resize(m, 1);
          }

          // cout << dataset[y].size() << ":";

          dataset[y].resize(dataset[y].size() + 1);
          example[y].resize(example[y].size() + 1);
          dataset[y].back().resize(num_feature, false);

          int k{0};
          for (auto x{beg_sample}; x != end_sample; ++x) {
            if (*x) {

              if (*x != 1) {
                cout << "e the dataset is not binary, rerun with --binarize\n";
                exit(1);
              }

              dataset[y].back().set(k);
              example[y].back().push_back(k);

              // cout << " " << k;
            }
            ++k;
          }
          // cout << endl;
  }

  /*!@name Printing*/
  //@{
  // std::ostream &toCsv(std::ostream &os) const;
  std::ostream &display(std::ostream &os) const;
  //@}
};

std::ostream &operator<<(std::ostream &os, const BacktrackingAlgorithm &x);
}

#endif // _PRIMER_BACKTRACK_HPP
