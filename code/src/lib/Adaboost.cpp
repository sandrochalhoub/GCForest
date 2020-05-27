#include "Adaboost.hpp"

// https://fr.wikipedia.org/wiki/AdaBoost

namespace primer {

  WeakClassifier::WeakClassifier(DTOptions &opt) : algo(wood, opt) {}


  // ===== Adaboost

  // Ce qui reste à faire :
  // - quand l'erreur min (?) est inférieure à 0 ?

  Adaboost::Adaboost(DTOptions &opt)
    : options(opt), max_it(30) {

  }

  void Adaboost::train() {
    it_count = 0;
    classifiers.clear();

    while (!should_stop()) {
      iteration();
      ++it_count;
    }
  }

  bool Adaboost::predict(const instance &i) const {
    double pred = 0;

    for (auto &clf: classifiers) {
      pred += clf->weight * (clf->algo.getSolution().predict(i) ? 0 : 1);
    }

    return pred > 0.5;
  }

  double Adaboost::get_accuracy() const {
    int correct = 0;

    for (int y = 0; y < 2; ++y) {
      int count = bitsets[y].size();

      for (int i = 0; i < count; ++i) {
        if (predict(bitsets[y][i]) == y) {
          ++correct;
        }
      }
    }

    return correct / double(example_count());
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

    // Print stuff to evaluate performance
    auto ex_count = example_count();
    std::cout << "error: " << algo.error() * ex_count << "/" << ex_count << std::endl;
  }

  void Adaboost::initialize_weights() {
    auto &current_algo = classifiers.back()->algo;
    double m = example_count();

    for (int y = 0; y < 2; ++y) {
      int count = dataset[y].size();

      for (int i = 0; i < count; ++i) {
        auto &sample = dataset[y][i];
        current_algo.addExample(sample.begin(), sample.end(), y, 1 / m);
      }
    }
  }

  void Adaboost::update_weights() {
    auto &last_clf = classifiers.at(classifiers.size() - 2)->algo;
    auto &current_clf = *classifiers.back();
    auto &current_algo = current_clf.algo;
    auto last_tree = last_clf.getSolution();

    double err = last_clf.error();
    double alpha = 1. / 2. * log((1. - err) / err);

    current_clf.weight = alpha;

    for (int y = 0; y < 2; ++y) {
      int count = dataset[y].size();

      for (int i = 0; i < count; ++i) {
        auto &sample = dataset[y][i];
        auto &bsample = bitsets[y][i];

        double u = y == 0 ? -1 : 1;
        double upred = last_tree.predict(bsample) ? 1 : -1;

        double d = last_clf.error_policy.get_weight(y, i);
        double dnext = d * exp(- alpha * u * upred) / (2 * sqrt(err * (1 - err)));

        current_algo.addExample(sample.begin(), sample.end(), y, dnext);
      }
    }
  }

  bool Adaboost::should_stop() {
    return it_count >= max_it;
  }

  size_t Adaboost::example_count() const {
    return dataset[0].size() + dataset[1].size();
  }
}