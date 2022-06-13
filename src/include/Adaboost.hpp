
#ifndef _BLOSSOM_ADABOOST_HPP
#define _BLOSSOM_ADABOOST_HPP

#include <memory>

#include "Backtrack.hpp"
#include "WeightedDataset.hpp"

namespace blossom {

struct WeakClassifier {
  Tree<double> T;
  double self_error;
  double alpha;

  size_t global_error{0};

  WeakClassifier() {}
  WeakClassifier(BacktrackingAlgorithm<WeightedError, double> &A);
};

class Adaboost {
public:
  DTOptions &options;

  Adaboost(WeightedDataset<int> &d, DTOptions &opt);

  std::vector<double>* train();

	template<class sample>
  bool predict(const sample &i) const;
	
	template<class sample>
  double guess(const sample &i) const;

  /** Preprocess dataset according to options (split, remove inconsistent) */
  void preprocess();

  double get_accuracy() const;

  size_t get_error() const;

  //Reminder
  std::vector<WeakClassifier> getClassifier();

private:
  // the orginal dataset (weights are only there to represent cardinality)
  WeightedDataset<int> &dataset;
  // previous weights in the adaboost iterations
  std::vector<double> weight[2];

  // what trees are made of
  // Wood wood;
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
	
	
	template<class sample>
	bool Adaboost::predict(const sample &i) const {
	  return guess(i) > 0;
	}
	
	template<class sample>
	double Adaboost::guess(const sample &i) const {
	  double g = 0;
	  for (auto &clf : classifiers) {
	    g += clf.alpha * (clf.T.predict(i) ? 1 : -1);
	  }

	  return g;
	}
	
}

#endif // _BLOSSOM_ADABOOST_HPP
