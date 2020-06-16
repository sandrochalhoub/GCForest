
#ifndef _PRIMER_ADABOOST_HPP
#define _PRIMER_ADABOOST_HPP

#include <memory>

#include "Backtrack.hpp"

namespace primer {

  struct WeakClassifier {
    Wood wood;
    BacktrackingAlgorithm<WeightedError, double> algo;

    double weight = 1;


    WeakClassifier(DTOptions &opt);
  };

  class Adaboost {
  public:
    DTOptions &options;


    Adaboost(DTOptions &opt);

    void train();

    bool predict(const instance &i) const;

    void split_dataset(double split_value);

    double get_accuracy() const;

    double get_test_accuracy() const;

    template <class rIter>
    void addExample(rIter beg_sample, rIter end_sample, const bool y);

  private:
    size_t max_it;
    std::vector<std::vector<int>> dataset[2];
    std::vector<instance> bitsets[2];

    std::vector<instance> test_bitsets[2];

    // internal variables
    size_t it_count;
    double start_time;
    // double total_weight = 0;
    std::vector<std::unique_ptr<WeakClassifier>> classifiers;


    void iteration();

    void initialize_weights();

    /// Add examples to the new predictor and compute the weights for
    /// the upcoming iteration
    void update_weights();

    void compute_clf_weight();

    bool should_stop();

    size_t example_count() const;

    double get_accuracy(const std::vector<instance> *bitsets) const;

    size_t get_correct_count(const std::vector<instance> *bitsets) const;
  };

  template <class rIter>
  inline void Adaboost::addExample(rIter beg_sample, rIter end_sample, const bool y) {
    // Add vector
    std::vector<int> sample(beg_sample, end_sample);
    dataset[y].push_back(sample);

    // Add bitset
    instance bsample(sample.size());

    for (int i = 0; i < sample.size(); ++i) {
      if (sample[i] == 1) {
        bsample.set(i);
      }
    }

    bitsets[y].push_back(bsample);
  }
}

#endif // _PRIMER_ADABOOST_HPP