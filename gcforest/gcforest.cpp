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

// Returns the prediction of the current tree of the forest, for the i-th for data point
int getPrediction(WeightedDataset<int>::List X, std::vector<WeakClassifier> classifiers, int j, int i) {
  Tree<double>* sol = &(classifiers[j].T);
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
  ////// Adaboost for the forest initialization.
  Adaboost A(*training_set, opt);
  ////// BacktrackingAlgorithm for all subsequent iterations.
  BacktrackingAlgorithm<ErrorPolicy, E_t> B(*training_set, opt);

  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d inputtime=" << cpu_time() << endl;

  ////// SOLVING
  A.train();
  printf("\n");

  std::vector<WeakClassifier> classifiers = A.getClassifier();
  std::vector<std::vector<int>> predictions;
  std::vector<int> weights;
  std::vector<std::vector<int>> label;
  std::vector<int> cplex_vector;
  long unsigned int data_size; // Size of the training set

  ////// Backtracking initialization
  /*
  if (opt.mindepth) {
    if (opt.minsize)
      B.minimize_error_depth_size();
    else
      B.minimize_error_depth();
  } else {
    if (opt.minsize)
     B.set_size_objective();
    B.minimize_error();
  }
  */


  ////// Work in progress: solving with CG
  for (int j = 0 ; j < classifiers.size() ; j++) {
    if (opt.verified) {
      //E_t tree_error = 0;
      for (auto y{0}; y < 2; ++y) {
        auto X{(*training_set)[y]};
	data_size = X.size();
	for (auto i : X) {
	  int sum = 0;
	  int prediction = getPrediction(X, classifiers, j, i);
	  //printf("%d | ", prediction);	   
	  //// For some reason, push_back putting very random values at some places here. Probably a memory problem, but why?
	  
	  //auto posPred = predictions[j].begin() + i;
	  //predictions[j].insert(posPred, prediction);
	  //predictions[j].push_back(prediction);
	  //printf("%d | ", predictions[j][i]);
	  
          //tree_error += (sol.predict(X[j]) != y) * X.weight(j);
	  
	  if (prediction == y) {
	    auto posWeights = weights.begin() + j;
	    weights.insert(posWeights, X.weight(i));
	    sum += weights[j] * prediction;
	    printf("%d | ", weights[j]);
	    //// Which parameters for setWeight ?
	    B.setWeight(y, j, weights[j]);
	    //int vecWeights = B.getWeight(y, j);
	    //printf("%d ", vecWeights);

	    /*
	    if (i == data_size) {
	      //auto posCplex = cplex_vector.begin() + i;
	      if(weights[j] * prediction > 0) cplex_vector.push_back(1);
    	      else cplex_vector.push_back(-1);
	      printf("%d | ", cplex_vector[i]);
	    }
	    */
	  }
	}
      }
    }
    //printf("\n");
  }

  ////// Computing the f[i] vector, = 1 if the sum > 0, = -1 otherwise

  /*
  for (int i = 0 ; i < data_size ; i++) {
    int sum = 0
    for (int j = 0 ; j < classifiers.size() ; j++) {
	sum += weights[j] * predictions[j][i];
    }
    if(weights[j] * predictions[j][i] > 0) cplex_vector[i] = 1;
    else cplex_vector[i] = -1;
  }
  */
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
