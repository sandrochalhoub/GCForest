
#ifndef _PRIMER_ADABOOST_HPP
#define _PRIMER_ADABOOST_HPP

#include <memory>

#include "Backtrack.hpp"
#include "WeightedDataset.hpp"

namespace blossom {

struct WeakClassifier {
  Tree T;
  double self_error;
  double weight;

  size_t global_error{0};

  WeakClassifier() {}
  WeakClassifier(BacktrackingAlgorithm<WeightedError, double> &A);
};

class Adaboost {
public:
  DTOptions &options;

  Adaboost(WeightedDataset<int> &d, DTOptions &opt);

  void train();

  bool predict(const instance &i) const;

  /** Preprocess dataset according to options (split, remove inconsistent) */
  void preprocess();

  double get_accuracy() const;

  size_t get_error() const;

private:
  // the orginal dataset (weights are only there to represent cardinality)
  WeightedDataset<int> &dataset;
  // previous weights in the adaboost iterations
  std::vector<double> weight[2];

  // what trees are made of
  Wood wood;
  // the algorithm
  BacktrackingAlgorithm<WeightedError, double> algo;

  // one classifier per iteration
  std::vector<WeakClassifier> classifiers;

  // internal variables
  double start_time;
  size_t total_size{0};

  // Iteration at which we reached the best error.
  size_t best_it;
  size_t best_error;
  size_t best_size;

  void iteration();

  void initialize_weights();

  /// Compute the weights for the next iteration
  void update_weights();

  bool should_stop();

  size_t maximum() const;

  size_t _correct_count;
  size_t get_correct_count() const;
  void compute_correct_count();
  };
}

#endif // _PRIMER_ADABOOST_HPP
