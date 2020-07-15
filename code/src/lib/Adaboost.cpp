#include "Adaboost.hpp"

#include "utils.hpp"
#include "WeightedDataset.hpp"

// https://fr.wikipedia.org/wiki/AdaBoost

#define EPSILON std::numeric_limits<double>::epsilon()

namespace primer {

  WeakClassifier::WeakClassifier(DTOptions &opt) : algo(wood, opt) {}


  // ===== Adaboost

  Adaboost::Adaboost(DTOptions &opt)
    : options(opt), max_it(opt.ada_it) {

  }

  void Adaboost::setErrorOffset(size_t error_offset) {
    this->error_offset = error_offset;
  }

  void Adaboost::train() {
    it_count = 0;
    start_time = cpu_time();
    classifiers.clear();

    while (!should_stop()) {
      iteration();
      ++it_count;
    }
  }

  bool Adaboost::predict(const instance &i) const {
    double pred = 0;

    for (auto &clf: classifiers) {
      pred += clf->weight * (clf->algo.getSolution().predict(i) ? 1 : -1);
    }

    return pred > 0;
  }

  bool Adaboost::predict(const std::vector<int> &sample) const {
      instance bsample(sample.size());

      for (size_t i = 0; i < sample.size(); ++i) {
        if (sample[i] == 1) {
          bsample.set(i);
        }
      }
      return predict(bsample);
  }

  double Adaboost::get_accuracy() const {
    return get_accuracy(bitsets, this->error_offset);
  }

  double Adaboost::get_test_accuracy() const {
    return get_accuracy(test_bitsets);
  }

  void Adaboost::preprocess() {
    // split dataset
    if (options.split > 0) {
      split_dataset(options.split);
    }

    // filter inconsistent examples on training set
    if (options.filter_inconsistent) {
      WeightedDataset filter;

      for (int y = 0; y < 2; ++y) {
        for (int i = 0; i < dataset[y].size(); ++i) {
          filter.addExample(dataset[y][i].begin(), dataset[y][i].end(), y);
        }

        bitsets[y].clear();
        dataset[y].clear();
      }

      filter.toInc(*this);

      std::cout << "Filter inconsistent: train size=" << bitsets[0].size() + bitsets[1].size() << std::endl;
    }
  }

  void Adaboost::split_dataset(double split_value) {
    // TODO if dataset is already splitted, reset before splitting again
    // This is only needed if we call split_dataset() multiple times
    std::mt19937 rng;
    rng.seed(options.seed);

    for (int y = 0; y < 2; ++y) {
      std::shuffle(bitsets[y].begin(), bitsets[y].end(), rng);
      auto size = bitsets[y].size();
      auto test_size = size_t(split_value * size);
      auto train_size = size - test_size;

      for (int i = 0; i < train_size; ++i) {
        std::vector<int> example;
        for (int j = 0; j < bitsets[y][i].size(); ++j) {
          example.push_back(bitsets[y][i][j] ? 1 : 0);
        }
        dataset[y][i] = example;
      }

      for (int i = 0; i < test_size; ++i) {
        auto test_example = bitsets[y][train_size + i];
        test_bitsets[y].push_back(test_example);
      }

      bitsets[y].resize(train_size);
      dataset[y].resize(train_size);

      std::cout << "Split: y=" << y << ", test size=" << test_size << ", train size=" << train_size << std::endl;
    }
  }

  void Adaboost::iteration() {
    // Create the new predictor
    classifiers.emplace_back(std::make_unique<WeakClassifier>(options));
    auto &algo = classifiers.back()->algo;

    // Add examples
    if (it_count == 0) {
      initialize_weights();
    }
    else {
      update_weights();
    }

    algo.minimize_error();
    compute_clf_weight();

    // Print stuff to evaluate performance
    std::cout << "d ada_train_acc=" << get_accuracy();
    if (example_count(test_bitsets)) {
        std::cout << " ada_test_acc=" << get_test_accuracy();
    }
    std::cout << " ada_time=" << cpu_time() - start_time << " ada_size=" << algo.getUbSize() << std::endl;
  }

  void Adaboost::initialize_weights() {
    auto &current_algo = classifiers.back()->algo;
    double m = example_count(bitsets);

    for (int y = 0; y < 2; ++y) {
      int count = dataset[y].size();

      for (int i = 0; i < count; ++i) {
        auto &sample = dataset[y][i];
        current_algo.addExample(sample.begin(), sample.end(), y, 1 / m);
      }
    }
  }

  void Adaboost::update_weights() {
    auto &last_clf = *classifiers.at(classifiers.size() - 2);
    auto &last_algo = last_clf.algo;
    auto &current_clf = *classifiers.back();
    auto &current_algo = current_clf.algo;
    auto last_tree = last_algo.getSolution();

    double err = last_algo.error();
		assert(err > 0);

    double alpha = last_clf.weight;

    for (int y = 0; y < 2; ++y) {
      int count = dataset[y].size();

      for (int i = 0; i < count; ++i) {
        auto &sample = dataset[y][i];
        auto &bsample = bitsets[y][i];

        double u = y == 0 ? -1 : 1;
        double upred = last_tree.predict(bsample) ? 1 : -1;

        double d = last_algo.error_policy.get_weight(y, i);
        double dnext = d * exp(- alpha * u * upred) / (2 * sqrt(err * (1 - err)));

        current_algo.addExample(sample.begin(), sample.end(), y, dnext);
      }
    }
  }

  void Adaboost::compute_clf_weight() {
    auto &current_clf = *classifiers.back();
    auto &current_algo = current_clf.algo;

    double err = current_algo.error();
    if (err < EPSILON) {
      current_clf.weight = 1e10;
    }
    else {
      double alpha = 1. / 2. * log((1. - err) / err);
      current_clf.weight = alpha;
    }
  }

  bool Adaboost::should_stop() {
    if (!classifiers.empty()) {
      if (classifiers.back()->algo.error() < EPSILON) {
        std::cout << "r iterations=" << it_count << std::endl;
        return true;
      }
      if (get_correct_count(bitsets, error_offset) == example_count(bitsets, error_offset)) {
        std::cout << "r iterations=" << it_count << std::endl;
        return true;
      }
    }

    return it_count >= max_it;
  }

  size_t Adaboost::example_count(const std::vector<instance> *bitsets, size_t error_offset) const {
    return bitsets[0].size() + bitsets[1].size() + error_offset * 2;
  }

  double Adaboost::get_accuracy(const std::vector<instance> *bitsets, size_t error_offset) const {
    // std::cout << get_correct_count(bitsets, error_offset) << "/" << example_count(bitsets, error_offset) << std::endl;
    return get_correct_count(bitsets, error_offset) / double(example_count(bitsets, error_offset));
  }

  size_t Adaboost::get_correct_count(const std::vector<instance> *bitsets, size_t error_offset) const {
    size_t correct = 0;

    for (int y = 0; y < 2; ++y) {
      size_t count = bitsets[y].size();

      for (size_t i = 0; i < count; ++i) {
        if (predict(bitsets[y][i]) == y) {
          ++correct;
        }
      }
    }

    return correct + error_offset;
  }
}
