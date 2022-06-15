#include <cstdio>
#include <iostream>
#include <random>
#include <stdlib.h>
#include "../src/include/WeightedDataset.hpp"
#include "../src/include/Backtrack.hpp"
#include "../src/include/Adaboost.hpp"
#include "../src/include/CSVReader.hpp"
#include "../src/include/CmdLine.hpp"
#include "../src/include/Reader.hpp"
#include "../src/include/Tree.hpp"
#include "/net/phorcys/data/roc/Logiciels/CPLEX_Studio201/cplex/include/ilcplex/ilocplex.h"

ILOSTLBEGIN

//IloInt nbElements;
//IloNumArray p;

using namespace std;
using namespace blossom;

// Returns the prediction of the j-th tree of the forest, for the i-th for data point
int getPrediction(WeightedDataset<int>::List X, Tree<double> * sol, int j, int i) {
  return sol->predict(X[i]);
}

template <template <typename> class ErrorPolicy = WeightedError,
          typename E_t = int>
int run_algorithm(DTOptions &opt) {

  WeightedDataset<E_t> input;

  ////// READING

  try {
    read_binary(input, opt);
  } catch (const std::exception &e) {
    if (opt.verbosity >= DTOptions::NORMAL)
      cout << "c format not recognized or input non-binary, binarizing...\n";
    read_non_binary(input, opt);
  }

  WeightedDataset<E_t> *test_set = new WeightedDataset<E_t>();
  WeightedDataset<E_t> *training_set = new WeightedDataset<E_t>();

  if (opt.test_sample != 0) {

    std::vector<int>::iterator endx[2] = {input.examples[0].bbegin(),
                                          input.examples[1].bbegin()};

    input.drawSample(opt.test_sample, *training_set, *test_set, opt.seed);

    if (opt.sample_only) {
      for (auto y{0}; y < 2; ++y) {
        cout << y << " " << (endx[y] - input.examples[y].bbegin());
        for (auto x{input.examples[y].bbegin()}; x != endx[y]; ++x) {
          cout << " " << *x;
        }
        cout << endl;
      }
    }

  } else {
    training_set = &input;
  }

  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d readtime=" << cpu_time() << endl;

  ////// PREPROCESSING
  if (opt.preprocessing) {
    training_set->preprocess(opt.verbosity >= DTOptions::NORMAL);
  }

  ////// CREATING THE ALGORITHMS

  // Adaboost for the forest initialization.
  Adaboost A(*training_set, opt);
  // BacktrackingAlgorithm for all subsequent iterations.
  BacktrackingAlgorithm<ErrorPolicy, E_t> B(*training_set, opt);

  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d inputtime=" << cpu_time() << endl;

  ////// SOLVING
  A.train();
  printf("\n\n");

  // Adaboost resulting forest
  std::vector<WeakClassifier> classifiers = A.getClassifier();
  std::vector<int> predictions[classifiers.size()];
  std::vector<int> label[classifiers.size()];

  // Backtracking initialization
  //if (opt.mindepth) {
    //if (opt.minsize)
      //B.minimize_error_depth_size();
    //else
      //B.minimize_error_depth();
  //} else {
    //if (opt.minsize)
     //B.set_size_objective();
    //B.minimize_error();
  //}


  // Work in progress: solving with CG
  for (int i = 0 ; i < classifiers.size() ; i++) {
    Tree<double> * sol = &(classifiers[i].T);
    if (opt.verified) {
      //E_t tree_error = 0;
      for (auto y{0}; y < 2; ++y) {
        auto X{(*training_set)[y]};
	for (auto j : X) {
	  int prediction = getPrediction(X, sol, i, j);
	  // For some reason, push_back putting very random values at some places here. Probably a memory problem, but why? 
	  //predictions[i].push_back(prediction);
	  //printf("%d | ", predictions[i][j]);
          //tree_error += (sol.predict(X[i]) != y) * X.weight(i);
	  if (prediction == y) {
	    // Which parameters for setWeight ?
	    B.setWeight(y, i, X.weight(i));
	    //int vecWeights = B.getWeight(y, i);
	    //if (vecWeights == X.weight(i)) printf("YES ");
	  }
	}
      }
    }
    //printf("\n");
  }

  return 0;
}


int main(int argc, char *argv[]) {
  DTOptions opt = parse_dt(argc, argv);

  if (opt.print_cmd)
    cout << opt.cmdline << endl;

  if (opt.print_par)
    opt.display(cout);

  if (opt.preprocessing) {
    return run_algorithm<WeightedError, int>(opt);
  } else {
    return run_algorithm<>(opt);
  }

}
