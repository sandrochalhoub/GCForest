
#ifndef _PRIMER_ADABOOST_HPP
#define _PRIMER_ADABOOST_HPP

#include <memory>

#include "Backtrack.hpp"

namespace blossom {

  struct WeakClassifier {
    Wood wood;
    BacktrackingAlgorithm<WeightedError, double> algo;

    double weight = 1;
    size_t error;


    WeakClassifier(DTOptions &opt);
  };

  class Adaboost {
  public:
    DTOptions &options;


    Adaboost(DTOptions &opt);

    void setErrorOffset(size_t error_offset);

    void train();

    bool predict(const instance &i) const;

    bool predict(const std::vector<int> &example) const;

    /** Preprocess dataset according to options (split, remove inconsistent) */
    void preprocess();

    void split_dataset(double split_value);

    double get_accuracy() const;

    double get_test_accuracy() const;

    size_t get_error() const;

    // template <class rIter>
    // void addExample(rIter beg_sample, rIter end_sample, const bool y);
    //
    // /**
    // * So that Adaboost supports weights, and we can use WeightedDataset on Adaboost,
    // * without any change.
    // */
    // template <class rIter>
    // void addExample(rIter beg_sample, rIter end_sample, const bool y, const size_t weight);
		
	  void addBitsetExample(const dynamic_bitset<> &sample, const bool y,
	                  const size_t weight);

    // void addExample(const std::vector<int> &example) {
    //     addExample(example.begin(), example.end() - 1, example.back());
    // }

  private:
    size_t max_it;
    size_t hist_lookup = 0;
    // std::vector<std::vector<int>> dataset[2];
    std::vector<instance> bitsets[2];

    std::vector<instance> test_bitsets[2];

    size_t error_offset = 0;

    // internal variables
    size_t it_count;
    double start_time;
    size_t best_error;
    /// Iteration at which we reached the best error.
    size_t best_it;
    /// Final result is the best iteration.
    bool keep_best = true;
    // double total_weight = 0;
    std::vector<std::unique_ptr<WeakClassifier>> classifiers;


    void iteration();

    void initialize_weights();

    /// Add examples to the new predictor and compute the weights for
    /// the upcoming iteration
    void update_weights();

    void compute_clf_weight();

    bool should_stop();

    size_t example_count(const std::vector<instance> *bitsets, size_t error_offset = 0) const;

    double get_accuracy(const std::vector<instance> *bitsets, size_t error_offset = 0) const;

    size_t get_correct_count(const std::vector<instance> *bitsets, size_t error_offset = 0) const;
  };

  // template <class rIter>
  // inline void Adaboost::addExample(rIter beg_sample, rIter end_sample, const bool y) {
  //   // // Add vector
  //   // std::vector<int> sample(beg_sample, end_sample);
  //   // dataset[y].push_back(sample);
  //
  //   // Add bitset
  //   instance bsample(end_sample - beg_sample, 0);
  // 		for(auto i{beg_sample}; i!=end_sample; ++i)
  // 			if(*i)
  // 				bsample.set(i - beg_sample);
  //
  //   // for (size_t i = 0; i < sample.size(); ++i) {
  //   //   if (sample[i] == 1) {
  //   //     bsample.set(i);
  //   //   }
  //   // }
  //
  //   bitsets[y].push_back(bsample);
  // }
	
  inline void Adaboost::addBitsetExample(const dynamic_bitset<> &sample, const bool y,
	                  const size_t weight) {
    // Add vector
		// std::vector<int> vsample(sample.size(), 0);
		
		for (size_t i = 0; i < weight; ++i) {
   	 bitsets[y].push_back(sample);
		}
    // Add bitset
    // instance bsample(sample.size());

		//     for (size_t i = 0; i < sample.size(); ++i) {
		//       if (sample[i]) {
		//         vsample[i] = 1;
		//       }
		//     }
		//
		// for (size_t i = 0; i < weight; ++i) {
		// dataset[y].push_back(vsample);
		// }
  }

  // template <class rIter>
  // inline void Adaboost::addExample(rIter beg_sample, rIter end_sample, const bool y, const size_t weight) {
  //   // TODO add with weights once we have implemented:
  //   // - splitting weighted datasets
  //   // - Adaboost weights taking weights into account
  //   for (size_t i = 0; i < weight; ++i) {
  //     addExample(beg_sample, end_sample, y);
  //   }
  // }
}

#endif // _PRIMER_ADABOOST_HPP
