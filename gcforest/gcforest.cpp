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

// Returns the prediction of the j-th tree of the forest, for the i-th data point
int getPrediction(WeightedDataset<int>::List* X, std::vector<WeakClassifier>* classifiers, int j, int i) {
  Tree<double>* sol = &((*classifiers)[j].T);
  return sol->predict((*X)[i]);
}

// Column generation method with CPLEX
int generateColumns(IloIntArray weights, IloIntArray predictions, IloIntArray labels) {
  IloEnv env;

  try {
      IloModel primal(env);
      IloCplex primalSolver(primal);
      /// COLUMN-GENERATION PROCEDURE
      IloIntArray newWeights(env, weights.getSize());

      for (;;) {
         /// OPTIMIZE OVER CURRENT PATTERNS
         primalSolver.solve();
         /// FIND AND ADD A NEW PATTERN
	 
      }
  } catch (IloException& ex) {
      cerr << "Error: " << ex << endl;
  } catch (...) {
      cerr << "Error" << endl;
  }

  env.end();

  return 0;
}

template <template <typename> class ErrorPolicy = WeightedError,
          typename E_t = int>
int run_algorithm(DTOptions &opt) {

  IloEnv env;

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
  printf("\n");

  // All data points whose class is 0
  auto classZero{(*training_set)[0]};
  // All data points whose class is 1
  auto classOne{(*training_set)[1]};
  // Size of the entire training set
  int data_size = classZero.size() + classOne.size();
  //printf("\n %d \n", data_size);

  std::vector<WeakClassifier> classifiers = A.getClassifier();

  std::vector<std::vector<int>> predictions(classifiers.size(), vector<int>(data_size, 0));
  //IloIntArray cplex_predictions(env);

  std::vector<int> weights(classifiers.size());
  IloIntArray cplex_weights(env);

  std::vector<int> label(data_size, 0);
  IloIntArray cplex_labels(env);

  std::vector<int> decision_vector(data_size);
  IloNumArray cplex_vector(env);

  ////// WORK IN PROGRESS
  for (int j = 0 ; j < classifiers.size() ; j++) {
    //printf("\nTREE NUMBER %d \n\n", j);
    if (opt.verified) {
	for (int i = 0 ; i < data_size ; i++) {
	  ////// CLASS ZERO
	  if (i < classZero.size()) {
	    //printf("%d | ", i);
	    int prediction = getPrediction(&classZero, &classifiers, j, i);
	    predictions[j][i] = prediction;	   
	    ////// Reminder to ask about direct access / push_back / insert
	    //auto posPred = predictions[j].begin() + i;
	    //predictions[j].insert(posPred, prediction);
	    //predictions[j].push_back(prediction);
	    //printf("%d | ", predictions[j][i]);
	  
	    if (prediction == 0) {
	      weights[j] = classZero.weight(i);    
	      //auto posWeights = weights.begin() + j;
	      //weights.insert(posWeights, X.weight(i));
	      //printf("%d | %d \n", i, weights[j]);
	      B.setWeight(0, i, weights[j]);
	      /*
	      int vecWeights = B.getWeight(0, i);
	      printf("%d | %d \n", i, vecWeights);
	      */
	    }
	  ////// CLASS ONE
	  } else {
	      //printf("%d | ", i);
	      int prediction = getPrediction(&classOne, &classifiers, j, i);
	      predictions[j][i] = prediction;	   
	      //auto posPred = predictions[j].begin() + i;
	      //predictions[j].insert(posPred, prediction);
	      //predictions[j].push_back(prediction);
	      //printf("%d | ", predictions[j][i]);
	  
	      if (prediction == 1) {
	        weights[j] = classOne.weight(i);    
	        //auto posWeights = weights.begin() + j;
	        //weights.insert(posWeights, X.weight(i));
	        //printf("%d | %d \n", i, weights[j]);
	        B.setWeight(1, i, weights[j]);
	        /*
	        int vecWeights = B.getWeight(1, i);
	        printf("%d | %d \n", i, vecWeights);
	        */
	    }
	  }
	}
    }
    //printf("\n");
  }


  ////// Computing the f[i] vector, = 1 if the sum > 0, = -1 otherwise
  for (int i = 0 ; i < data_size ; i++) {
    int sum = 0;
    for (int j = 0 ; j < classifiers.size() ; j++) {
	if (predictions[j][i] == 0)
	  sum += weights[j] * -1;
	else
	  sum += weights[j] * 1;
    }
    if(sum >= 0) decision_vector[i] = 1;
    else decision_vector[i] = -1;
    //printf("%d | %d \n", i, decision_vector[i]);
  }

  ////// CPLEX vectors
  // Weights
  for (int i = 0 ; i < weights.size() ; i++) {
    cplex_weights.add(weights[i]);
    //printf("%d | %lu \n", i, cplex_weights.operator[](i));
  }

  // Decision function
  for (int i = 0 ; i < data_size ; i++) {
    cplex_vector.add(decision_vector[i]);
    //printf("%d | %f \n", i, cplex_vector.operator[](i));
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
