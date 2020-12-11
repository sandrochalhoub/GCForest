#include "Adaboost.hpp"

#include "utils.hpp"

// https://fr.wikipedia.org/wiki/AdaBoost

#define EPSILON std::numeric_limits<double>::epsilon()

namespace blossom {

  WeakClassifier::WeakClassifier(DTOptions &opt) : algo(wood, opt) {}


  // ===== Adaboost

  Adaboost::Adaboost(WeightedDataset<int> &d, DTOptions &opt)
      : options(opt), dataset(d), max_it(opt.ada_it),
        hist_lookup(opt.ada_stop) {}

  // void Adaboost::setErrorOffset(size_t dataset.numInconsistent()) {
  //   this->dataset.numInconsistent() = dataset.numInconsistent();
  // }

  void Adaboost::train() {

    // if (options.split > 0) {
    //   split_dataset(options.split);
    // }

    it_count = 0;
    start_time = cpu_time();
    classifiers.clear();

    while (!should_stop()) {
      iteration();
      ++it_count;
    }

    if (keep_best) {
      it_count = best_it + 1;
      classifiers.resize(it_count);
      std::cout << "d best_iteration_count=" << it_count << " ada_train_acc=" << get_accuracy();
      // if (example_count(test_bitsets)) {
      //     std::cout << " ada_test_acc=" << get_test_accuracy();
      // }
      std::cout << std::endl;
    }
  }

  bool Adaboost::predict(const instance &i) const {
    double pred = 0;
		
    for (auto &clf: classifiers) {
      pred += clf->weight * (clf->algo.getSolution().predict(i) ? 1 : -1);
    }

    return pred > 0;
  }

  // bool Adaboost::predict(const std::vector<int> &sample) const {
  //     instance bsample(sample.size());
  //
  //     for (size_t i = 0; i < sample.size(); ++i) {
  //       if (sample[i] == 1) {
  //         bsample.set(i);
  //       }
  //     }
  //     return predict(bsample);
  // }

  double Adaboost::get_accuracy() const {
    return double(get_correct_count()) / double(dataset.input_example_count());
  }

  size_t Adaboost::get_error() const {
    return dataset.input_example_count() - get_correct_count();
  }

  // void Adaboost::preprocess() {
  //
  // 		cout << 11 << endl;
  //
  //   // split dataset
  //   if (options.split > 0) {
  //     split_dataset(options.split);
  //   }
  //
  // 		cout << 22 << endl;
  //
  //   // filter inconsistent examples on training set
  //   if (options.preprocessing) {
  //     WeightedDataset<int> filter;
  //
  //     for (int y = 0; y < 2; ++y) {
  //       for (int i = 0; i < bitsets[y].size(); ++i) {
  //         filter.addBitsetExample(bitsets[y][i], y);
  //       }
  //
  //       bitsets[y].clear();
  //       // dataset[y].clear();
  //     }
  //
  // 			filter.preprocess(false);
  // 			filter.setup(*this);
  //     // filter.toInc(*this);
  //
  //     std::cout << "Filter inconsistent: train size=" << bitsets[0].size() +
  //     bitsets[1].size() << std::endl;
  //   }
  //
  // 		cout << 33 << endl;
  // }

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

      // for (int i = 0; i < train_size; ++i) {
      //   std::vector<int> example;
      //   for (int j = 0; j < bitsets[y][i].size(); ++j) {
      //     example.push_back(bitsets[y][i][j] ? 1 : 0);
      //   }
      //   dataset[y][i] = example;
      // }

      for (int i = 0; i < test_size; ++i) {
        auto test_example = bitsets[y][train_size + i];
        test_bitsets[y].push_back(test_example);
      }

      bitsets[y].resize(train_size);
      // dataset[y].resize(train_size);

      std::cout << "Split: y=" << y << ", test size=" << test_size << ", train size=" << train_size << std::endl;
    }
  }

  void Adaboost::iteration() {
    // Create the new predictor
    classifiers.emplace_back(std::make_unique<WeakClassifier>(options));
    auto &algo = classifiers.back()->algo;

    // Add examples
    if (it_count == 0) {
			
			cout << "c init weights for classifier " << classifiers.size() << "\n";
			
      initialize_weights();
    }
    else {
			
			cout << "c update weights for classifier " << classifiers.size() << "\n";
			
      update_weights();
    }

    algo.minimize_error();
		
		
    compute_clf_weight();
		
		
		compute_correct_count();
		
		
		cout << "total = " << dataset.input_example_count() << " maximum = " << maximum() << " correct = " << get_correct_count() << " error = " <<  get_error() << endl;

    // Update best error / iteration
    size_t error = get_error();
    classifiers.back()->error = error;

    if (it_count == 0 || error < best_error) {
      best_it = it_count;
      best_error = error;
    }

    // Print stuff to evaluate performance
    std::cout << "d ada_train_acc=" << get_accuracy();
    // if (example_count(test_bitsets)) {
    //     std::cout << " ada_test_acc=" << get_test_accuracy();
    // }
    std::cout << " ada_time=" << cpu_time() - start_time << " ada_size=" << algo.getUbSize() << std::endl;

    // Free memory
    algo.clearExamples();
  }

  void Adaboost::initialize_weights() {
    auto &current_algo = classifiers.back()->algo;
    double m = dataset.example_count();

    for (int y = 0; y < 2; ++y) {
			
			auto X{dataset[y]};
						
			for( auto x : X ) {
				current_algo.addBitsetExample(X[x], y, X.weight(x) / m);
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
      auto X{dataset[y]};

			auto i{0};
      for (auto xi : X) {
				
				//
				//
				// cout << last_algo.dataset[y][i].count() << " " << last_algo.dataset[y][i].size() << " " << last_algo.dataset[y][i] << endl;
				// cout << X[xi].count() << " " << X[xi].size() << " " << X[xi] << endl;
				//
				//
				//
				//
				// // auto z1 = (last_algo.dataset[y][i] - X[xi]);
				// // auto z2 = (X[xi] - last_algo.dataset[y][i]);
				// //
				// // cout << z1.count() << " " << z2.count() << endl;
				// //
				// // assert(z1.count() == 0);
				// // assert(z2.count() == 0);
				//
				// assert(last_algo.dataset[y][i] == X[xi]);
				
				

				double u = y == 0 ? -1 : 1;
				double upred = last_tree.predict(X[xi]) ? 1 : -1;
				
        double d = last_algo.error_policy.get_weight(y, i);
        double dnext = d * exp(- alpha * u * upred) / (2 * sqrt(err * (1 - err)));
				
				++i;
				
				current_algo.addBitsetExample(X[xi], y, dnext);
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
      double alpha = .5 * log((1. - err) / err);
      current_clf.weight = alpha;
    }
  }

  bool Adaboost::should_stop() {
    // Stop if the accuracy does not improve for N iterations
    if (hist_lookup != 0 && classifiers.size() > hist_lookup) {
      if (it_count - best_it >= hist_lookup) {
        std::cout << "c no improvement stop\nd iterations=" << it_count + 1 << std::endl;
        return true;
      }
    }

    if (!classifiers.empty()) {
      if (classifiers.back()->algo.error() < EPSILON) {
        std::cout << "c perfect single classifier stop\nd iterations=" << it_count + 1 << std::endl;
        return true;
      }
      // if (get_correct_count(bitsets, dataset.numInconsistent()) == example_count(bitsets, dataset.numInconsistent())) {
			if (get_correct_count() == maximum()) {
        std::cout << "c perfect classifier stop\nd iterations=" << it_count + 1 << std::endl;
        return true;
      }
    }

    return it_count >= max_it;
  }
	
  size_t Adaboost::maximum() const {
	 	return dataset.input_example_count() - dataset.numInconsistent();	
	}

  size_t Adaboost::get_correct_count() const { 
		return _correct_count + dataset.numInconsistent();
	}
	
	void Adaboost::compute_correct_count() { 
    _correct_count = 0;
		
		size_t verif1 = 0;
		
		size_t verif2 = 0;

    for (int y = 0; y < 2; ++y) {
			
			auto X{dataset[y]};
			
			for(auto x : X) {
				
				// cout << x << ": " << X[x] << " " << predict(X[x]) << " * " << X.weight(x) << endl;
				
				_correct_count += X.weight(x) * (predict(X[x]) == y);
			}
			// cout << endl;
			
			for(auto i{0}; i<dataset.input_count(y); ++i) {
				if(X.contain(i)) {
					// cout << i << ": " << X[i] << " " << predict(X[i]) << " * " << X.weight(i) << endl;
					verif1 += X.weight(i) * (predict(X[i]) == y);				
				}
				
			}
			// cout << endl;
			for(auto i{0}; i<dataset.input_count(y); ++i) {
				
				// cout << i << ": " << X[i] << " " << predict(X[i]) << endl;
				
				verif2 += (predict(X[i]) == y);
			}
			// cout << endl;
			// cout << _correct_count << " / " << verif1 << " / " << verif2 << endl;

    }

		assert(verif2 == _correct_count + dataset.numInconsistent());			

  }
}
