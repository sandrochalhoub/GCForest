#include "Adaboost.hpp"

#include "utils.hpp"

// https://fr.wikipedia.org/wiki/AdaBoost

#define EPSILON std::numeric_limits<double>::epsilon()

namespace blossom {

WeakClassifier::WeakClassifier(BacktrackingAlgorithm<WeightedError, double> &A)
    : T(A.saveSolution()), self_error(A.error()),
      weight(self_error >= EPSILON ? .5 * log((1. - self_error) / self_error)
                                   : 1e10) {}

// ===== Adaboost

Adaboost::Adaboost(WeightedDataset<int> &d, DTOptions &opt)
    : options(opt), dataset(d), algo(wood, opt) {
  algo.options.verbosity = DTOptions::SILENT;
}

void Adaboost::train() {

  start_time = cpu_time();
  classifiers.clear();
	
	algo.separator("adaboost", 86);

  while (!should_stop()) {
    iteration();
  }

  classifiers.resize(best_it + 1);

  if (options.verbosity >= DTOptions::QUIET) {
    std::cout << std::left << "d  num=" << std::setw(3) << classifiers.size()
              << " accuracy=" << std::setw(8) << std::setprecision(5) << get_accuracy()
              << " error=" << std::setw(6) << best_error 
              << " size=" << setw(5) << best_size << endl;
  }
}

bool Adaboost::predict(const instance &i) const {
  double pred = 0;
  for (auto &clf : classifiers) {
    pred += clf.weight * (clf.T.predict(i) ? 1 : -1);
  }

  return pred > 0;
}

double Adaboost::get_accuracy() const {
  return double(get_correct_count()) / double(dataset.input_example_count());
}

size_t Adaboost::get_error() const {
  return dataset.input_example_count() - get_correct_count();
}

void Adaboost::iteration() {

  // Add examples
  if (classifiers.size() == 0) {
    initialize_weights();
  } else {
    // total_size = classifiers.back().global_size;
    update_weights();
		
		// algo.options.verbosity = 7;
  }

  algo.minimize_error();
  classifiers.push_back(WeakClassifier(algo));

  compute_correct_count();

  // Update best error / iteration
  size_t error = get_error();
  classifiers.back().global_error = error;
  total_size += algo.getUbSize();

  // assert(classifiers.size() == it_count+1);
  if (classifiers.size() == 1 || error < best_error) {
    best_it = classifiers.size() - 1;
    best_error = error;
    best_size = total_size;
  }

  // Print stuff to evaluate performance
  if (options.verbosity >= DTOptions::QUIET) {

    // total_size += algo.getUbSize();

    double t{static_cast<double>(
                 static_cast<int>(100.0 * (cpu_time() - start_time))) /
             100.0};

    std::cout << std::left << "d iter=" << std::setw(3) << classifiers.size()
              << " accuracy=" << std::setw(8) << std::setprecision(5) << get_accuracy()
              << " error=" << std::setw(6) << get_error() 
              << " size=" << setw(5) << total_size << " choices=" << setw(9)
              << algo.search_size << " mem=" << setw(3) << wood.size()
              << " time=" << t << right << endl;
  }

  // Reinitialize the algorithm
  algo.clear();
}

void Adaboost::initialize_weights() {
  double m = dataset.example_count();

  for (int y = 0; y < 2; ++y) {
    auto X{dataset[y]};
    for (auto x : X) {
      weight[y].push_back(double(X.weight(x)) / m);
      algo.addBitsetExample(X[x], y, weight[y].back());
    }
  }
}

void Adaboost::update_weights() {
  auto last_tree = classifiers.back().T;
  double err = classifiers.back().self_error;
  double alpha = classifiers.back().weight;

  for (int y = 0; y < 2; ++y) {
    auto X{dataset[y]};

    auto i{0};
    for (auto xi : X) {
      double u = y == 0 ? -1 : 1;
      double upred = last_tree.predict(X[xi]) ? 1 : -1;
      weight[y][i] *= exp(-alpha * u * upred) / (2 * sqrt(err * (1 - err)));
      algo.setWeight(y, i, weight[y][i]);

      ++i;
    }
  }
}

bool Adaboost::should_stop() {
	int width{86};

  // Stop if the accuracy does not improve for N iterations
  if (options.ada_stop != 0 and classifiers.size() > options.ada_stop and classifiers.size() - best_it >= options.ada_stop) {
			algo.separator("local optimum", width);
      return true;
  }

	// Stop if the accuracy is perfect
  if (!classifiers.empty() and (classifiers.back().self_error < EPSILON or get_correct_count() == maximum())) {
			algo.separator("perfect", width);
			return true;
  }

	// Stop if the limit of iterations has been reached
	auto stop{classifiers.size() >= options.ada_it};
	if(stop) {
		algo.separator("interrupted", width);
	}
  return stop;
}

size_t Adaboost::maximum() const {
  return dataset.input_example_count() - dataset.numInconsistent();
}

size_t Adaboost::get_correct_count() const {
  return _correct_count + dataset.numInconsistent();
}

void Adaboost::compute_correct_count() {
  _correct_count = 0;
  for (int y = 0; y < 2; ++y) {
    auto X{dataset[y]};
    for (auto x : X) {
      _correct_count += X.weight(x) * (predict(X[x]) == y);
    }
  }
}
}
