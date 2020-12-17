
#ifndef _BLOSSOM_BACKTRACK_HPP
#define _BLOSSOM_BACKTRACK_HPP

#include <iostream>
#include <random>
#include <vector>
#include <cmath>

#include "CmdLine.hpp"
#include "Partition.hpp"
#include "SparseSet.hpp"
#include "Tree.hpp"
#include "utils.hpp"

// #define PRINT_TRACE print_trace();
// #define DO_ASSERTS do_asserts();
#define PRINT_TRACE
#define DO_ASSERTS



using namespace boost;
using namespace std;



namespace blossom {

/// this needs to be templated with "T" and should be 0 for integral types
/// (e.g., static_cast<T>(1.e-9) should work)
// #define FLOAT_PRECISION std::numeric_limits<T>::epsilon()
#define FLOAT_PRECISION static_cast<T>(1.e-6)

template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
equal(const T &a, const T &b) {
  return a == b;
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
lt(const T &a, const T &b) {
  return a < b;
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
equal(const T &a, const T &b) {
  return std::fabs(a - b) < FLOAT_PRECISION;
}

template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
lt(const T &a, const T &b) {
  return a + FLOAT_PRECISION < b;
}

template <typename E_t> class CardinalityError;

template <typename E_t> class WeightedError;

template <typename E_t> class WeightedDataset;

/**********************************************
* BacktrackingAlgorithm
**********************************************/
/// Representation of a list of examples
// template <class ErrorPolicy = CardinalityError<int>, typename E_t = int>
template <template<typename> class ErrorPolicy = CardinalityError, typename E_t = int>
class BacktrackingAlgorithm {

public:

  /*!@name Constructors*/
  //@{
  explicit BacktrackingAlgorithm(const DTOptions &o);
  explicit BacktrackingAlgorithm(const WeightedDataset<E_t> &input,
                                 const DTOptions &o);

  void load(const WeightedDataset<E_t> &input);
  void seed(const int s);
  //@}

  size_t numExample() const;
  size_t numFeature() const;

  // whether feature f_a is equivalent to feature f_b
  bool equal_feature(const int f_a, const int f_b);

  void separator(const string &msg, const int width=82) const;
  void print_new_best();
  void print_progress();

  void setUbDepth(const size_t u);

  void setUbError(const E_t u);

  void addSizeObjective();

  E_t getUbError() const;

  size_t getUbDepth() const;

  size_t getUbSize() const;
	
	size_t getTreeMemory() const;
	
	size_t getSearchSize() const;

  void setTimeLimit(const double t);

  void setSearchLimit(const size_t t);

  bool search();
  void initialise_search();

  void minimize_error();

  void minimize_error_depth();

  void minimize_error_depth_size();

  Tree getSolution() const;
  Tree saveSolution();

  E_t error() const;
  double accuracy() const;

  void addExample(const dynamic_bitset<> &sample, const bool y,
                  const E_t weight = 1);

  /** Removes all the examples to free memory. The error and the model
   * still can be retrieved.  */
  void clearExamples();
	
  /** clear search data structures */
  void clear();

  void setErrorOffset(const E_t e);

  void setWeight(const int y, const size_t i, const E_t w);

  bool isRelevant(const int f) const { return feature_set[f]; };

  /*!@name Printing*/
  //@{
  std::ostream &display(std::ostream &os) const;
  //@}
	
  /*!@name Parameters*/
  //@{
  DTOptions options;
	//@}

private:
  friend ErrorPolicy<E_t>;

  /*!@name Parameters*/
  //@{
  /// Argument
  Wood wood;
  int num_feature;

  vector<vector<int>> example[2];

  vector<int> relevant_features;
  dynamic_bitset<> feature_set;

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
  vector<vector<E_t>> pos_feature_frequency[2];

  /// the list of features in the order they will be tried
  vector<vector<int>> ranked_feature;
  vector<vector<int>::iterator> end_feature;

  /// store the feature tested at node i (in ranked features)
  vector<vector<int>::iterator> feature;

  // Stores the root of the best subtree found for the current feature of the
  // parent node if the node is not optimal, it points to the optimal trees of
  // the childrens given its root-feature
  // therefore, *optimal* best trees should not be freed when pruning the node
  // however, they can be freed when replacing the current best.
  vector<int> best_tree;

  // this is because I'm stupid
  vector<E_t> tree_error;
  vector<size_t> tree_size;

  // optimistic value. updated only when the node becomes optimal
  vector<E_t> min_error;
  vector<size_t> min_size;

  // best value for any possible feature, given the current branch (ancestors)
  // get updated when backtracking from the decision on the current feature
  vector<E_t> max_error;
  vector<size_t> max_size;

  vector<E_t> f_error;
  vector<double> f_entropy;
  vector<double> f_gini;
  // vector<double> f_gini_d;

  mt19937 random_generator;

  size_t num_node;

  size_t ub_size;

  size_t ub_depth;

  E_t ub_error;

  size_t search_size;
  size_t search_limit;

  size_t num_solutions;

  size_t num_backtracks;

  size_t num_restarts;

  E_t current_error;

  size_t current_size;

  double time_limit;

  double restart_base;

  int restart_limit;

  int backtrack_node;

  int solution_root;

  int checking_period;

  bool interrupted;

  double start_time;

  int feature_criterion;

  bool size_matters;

  size_t actual_depth;

  // used to account for suppressed inconsistent examples
  E_t error_offset{0};

  int num_level_zero_feature;
  int num_explored{0};
  bool nb{true};
	
	
  ErrorPolicy<E_t> error_policy;

  vector<instance> dataset[2];
  vector<dynamic_bitset<>> reverse_dataset[2];

  void setReverse();

  void cleaning();

  bool store_new_best();

  int copy_solution(const int node);
//@}

#ifdef PRINTTRACE
  void print_trace();
  void do_asserts();
  // returns the real error for "leaf" nodes (deepest test), and node_error
  // otherwise
  E_t leaf_error(const int i) const;
#endif

  // resize the data structures for up to k nodes
  void resize(const int k);

  // current size of the data structures (current node capacity)
  // size_t size();

  // return true if all features have been tried feature
  bool no_feature(const int node) const;

  // // return true if the feature f reduces the error
  // bool reduce_error(const int node, const int f) const;

  // return true if the feature f is true/false in all examples
  bool max_entropy(const int node, const int f) const;

  // return true if the feature f classifies all examples
  bool null_entropy(const int node, const int f) const;

  // swap a random feature among the k best for node i with probability p/1000
  void random_perturbation(const int i, const int k, const int p);

  // sort the features by minimum projected error (leave 1-entropy node at the)
  void sort_features(const int node);

  template <class property> void filter_features(const int node, property cond);

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
  E_t get_feature_error(const int i, const int f) const;

  // returns the number of examples of class y having feature f (f + m
  // represents not-f)
  E_t get_feature_frequency(const int y, const int i, const int f) const;

  // returns the error if we do not test any feature on node i
  E_t node_error(const int i) const;

  // select the most promising node to branch on
  int highest_error() const;
  int highest_error_reduction() const;
	int lowest_error() const;

  // whether node is a leaf (deepest test)
  bool isLeaf(const int node) const;

  // save the current subtree of node as best tree
  void store_best_tree(const int node, const bool global);

  // lower bound, maybe?
  bool fail();

  // as name suggests
  bool notify_solution(bool &improvement);

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

  void restart(const bool full);

  bool limit_out();

  size_t computeSize(const int node) const;

  size_t maxSize(const int depth) const;

  void singleDecision();

  void noDecision();

  void init();
	
	
};


// Allow only integer types (size_t, )
template <typename E_t>
class CardinalityError {

  typedef BacktrackingAlgorithm<CardinalityError, E_t> Algo;

  Algo &algo;

public:
  CardinalityError(Algo &a) : algo(a) {}

  /** This method is called everytime a new example is added to the dataset.
  * \param i index of the added example */
  void add_example(const int y, const size_t i, const E_t weight = 1) {}

  void update_node(const int n) {}

  void set_weight(const int y, const size_t i, const E_t w) {}

  E_t get_weight(const int y, const size_t i) const;

  E_t node_error(const int i) const;

  void count_by_example(const int node, const int y) const;

  /// Returns the sum of the weights of all the examples at a specific node
  E_t get_total(const int y, const int n) const;

  void clear_examples() {}

  void clear() {}
};

template <typename E_t>
class WeightedError {

  typedef BacktrackingAlgorithm<WeightedError, E_t> Algo;

private:
  Algo &algo;

  // weight of each example when computing the error
  vector<E_t> weights[2];

  // total error for each node
  vector<E_t> weight_total[2];

public:
  WeightedError(Algo &a) : algo(a) {}

  /** This method is called everytime a new example is added to the dataset.
  * \param i index of the added example */
  void add_example(const int y, const size_t i, const E_t weight = 1);

  void update_node(const int n);

  void set_weight(const int y, const size_t i, const E_t w);

  E_t get_weight(const int y, const size_t i) const;

  E_t node_error(const int i) const;

  void count_by_example(const int node, const int y) const;

  /// Returns the sum of the weights of all the examples at a specific node
  E_t get_total(const int y, const int n) const;

  void clear_examples();

  void clear();
};

template <typename E_t>
inline E_t WeightedError<E_t>::get_weight(const int y, const size_t i) const {
  return weights[y][i];
}


template <typename E_t>
inline E_t CardinalityError<E_t>::get_weight(const int y, const size_t i) const {
  return 1;
}

template <typename E_t>
inline void WeightedError<E_t>::add_example(const int y, const size_t i,
                                            const E_t weight) {
  if (weights[y].size() <= i) {
    weights[y].resize(i + 1);
  }
  weights[y][i] = weight;
}

// template <template <typename> class ErrorPolicy, typename E_t>
// template <class rIter>
// inline void BacktrackingAlgorithm<ErrorPolicy, E_t>::addExample(
//     rIter beg_row, rIter end_row, const int target, const E_t weight) {
//   int width{static_cast<int>(end_row - beg_row)};
//   int n{width - 1};
//   int column{(target + width) % width};
//   auto y{*(beg_row + column)};
//
//   if (n > num_feature) {
//     num_feature = n;
//     // auto m{num_feature};
//     f_error.resize(num_feature, 1);
//     f_entropy.resize(num_feature, 1);
//     f_gini.resize(num_feature, 1);
//     // f_gini_d.resize(m, 1);
//   }
//
//   dataset[y].resize(dataset[y].size() + 1);
//   example[y].resize(example[y].size() + 1);
//   dataset[y].back().resize(num_feature, false);
//
//   int k{0};
//   int f{0};
//   for (auto x{beg_row}; x != end_row; ++x) {
//     if (k != column) {
//
//       if (*x) {
//
//         if (*x != 1) {
//           cout << "e the dataset is not binary, rerun with --binarize\n";
//           exit(1);
//         }
//
//         dataset[y].back().set(f);
//         example[y].back().push_back(f);
//       }
//
//       ++f;
//     }
//
//     ++k;
//   }
//
//   error_policy.add_example(y, example[y].size() - 1, weight);
//   // cout << endl;
// }

template <template <typename> class ErrorPolicy, typename E_t>
inline void BacktrackingAlgorithm<ErrorPolicy, E_t>::addExample(
    const dynamic_bitset<> &sample, const bool y, const E_t weight) {
  int n{static_cast<int>(sample.size())};

  if (n > num_feature) {
    num_feature = n;
    f_error.resize(num_feature, 1);
    f_entropy.resize(num_feature, 1);
    f_gini.resize(num_feature, 1);
  }

  dataset[y].push_back(sample);
  example[y].resize(example[y].size() + 1);

  for (auto x{0}; x < n; ++x)
    if (sample[x])
      example[y].back().push_back(x);

  error_policy.add_example(y, example[y].size() - 1, weight);
}

template <template<typename> class ErrorPolicy, typename E_t>
template<class property>
inline void BacktrackingAlgorithm<ErrorPolicy, E_t>::filter_features(const int node, property cond) {
  for (auto f{end_feature[node] - 1}; f >= feature[node]; --f) {
    if (cond(*f)) {
      swap(*f, *(--end_feature[node]));
    }
  }
}

template <template<typename> class ErrorPolicy, typename E_t>
std::ostream &operator<<(std::ostream &os, const BacktrackingAlgorithm<ErrorPolicy, E_t> &x);
}

#endif // _BLOSSOM_BACKTRACK_HPP
