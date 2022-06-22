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

using namespace std;
using namespace blossom;

// Returns the prediction of the j-th tree of the forest, for the i-th data point
IloInt getPrediction(WeightedDataset<int>::List* X, std::vector<WeakClassifier>* classifiers, IloInt j, IloInt i) {
  Tree<double>* sol = &((*classifiers)[j].T);
  return sol->predict((*X)[i]);
}

// Column generation method with CPLEX, IN PROGRESS
IloInt generateColumns(IloArray<IloIntArray> decisions) {
  IloEnv env;

  try {
      // Model
      IloModel primal(env);
      // Constants
      IloInt forestSize = decisions.getSize();
      IloInt datasetSize = decisions[0].getSize();
      // Variables
      IloNumVarArray weights(env, forestSize);
      IloNumVarArray z(env, datasetSize);
      IloNumVar zMin;
      // Objective function
      IloObjective obj = IloAdd(primal, IloMinimize(env, zMin));
      // Constraints
      for (IloInt i = 0 ; i < datasetSize ; i++) {
        primal.add(z[i] - zMin <= 0);
      }
      for (IloInt j = 0 ; j < forestSize ; j++) {
	primal.add(weights[j] >= 0);
	for (IloInt i = 0 ; i < datasetSize ; i++) {
	  primal.add(weights[j] * decisions[j][i] + z[i] >= 0);
	}
      }   

      /// COLUMN-GENERATION PROCEDURE

      IloCplex primalSolver(primal);
      primalSolver.exportModel("gcforest.lp");

      //for (;;) {
         /// OPTIMIZE OVER CURRENT PATTERNS
         //primalSolver.solve();
         /// FIND AND ADD A NEW PATTERN
	 
      //}
      /*
      if (primalSolver.solve()) {
         primalSolver.out() << "Solution status: " << primalSolver.getStatus() << endl;
         for (IloInt j = 0; j < forestSize ; j++) {
            primalSolver.out() << "   tree " << j << ": "
                        << primalSolver.getValue(weights[j]) << endl;
         }
         //primalSolver.out() << "Total cost = " << primalSolver.getObjValue() << endl;
      }
      else primalSolver.out()<< "No solution" << endl;
      primalSolver.printTime();
      */
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
IloInt run_algorithm(DTOptions &opt) {

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
  //BacktrackingAlgorithm<ErrorPolicy, E_t> B(*training_set, opt);

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
  IloInt data_size = classZero.size() + classOne.size();
  // The forest built by Adaboost
  std::vector<WeakClassifier> classifiers = A.getClassifier();

  ////// CPLEX vectors
  // Decisions vector == 1 if predictions[j][i] == classes[i], -1 otherwise
  IloArray<IloIntArray> decisions(env, classifiers.size());

  ////// Obsolete
  /*std::vector<std::vector<int>> predictions(classifiers.size(), vector<int>(data_size, 0));
  IloArray<IloIntArray> predictions(env, classifiers.size());
  std::vector<int> weights(classifiers.size());
  std::vector<int> decision_vector(data_size);
  IloIntArray weights(env);
  */

  ////// BUILDING PREDICTIONS AND WEIGHTS VECTORS
  for (IloInt j = 0 ; j < classifiers.size() ; j++) {
    //predictions[j] = IloIntArray(env, data_size, 0, 1, ILOINT);
    decisions[j] = IloIntArray(env, data_size, 0, 1, ILOINT);
    //printf("\nTREE NUMBER %d \n\n", j);
    if (opt.verified) {
	for (IloInt i = 0 ; i < data_size ; i++) {
	  ////// CLASS ZERO
	  if (i < classZero.size()) {
	    //printf("%d | ", i);
	    IloInt prediction = getPrediction(&classZero, &classifiers, j, i);
	    //predictions[j][i] = prediction;
            if (prediction == 0) decisions[j][i] = 1;
	    else decisions[j][i] = -1;	   
	    //printf("%d\n", decisions[j][i]);
	    /*
	    if (prediction == 0) {
	      weights[j] = classZero.weight(i);    
	      printf("%d | %d \n", i, weights[j]);
	      B.setWeight(0, i, weights[j]);
	      IloInt vecWeights = B.getWeight(0, i);
	      printf("%d | %d \n", i, vecWeights);
	    }
	    */
	  ////// CLASS ONE
	  } else {
	      //printf("%d | ", i);
	      IloInt prediction = getPrediction(&classOne, &classifiers, j, i);
	      if (prediction == 1) decisions[j][i] = 1;
	      else decisions[j][i] = -1;
	      //printf("%d\n", decisions[j][i]);
	      /*
	      if (prediction == 1) {
	        weights[j] = classOne.weight(i);    
	        //printf("%d | %d \n", i, weights[j]);
	        B.setWeight(1, i, weights[j]);
	        IloInt vecWeights = B.getWeight(1, i);
	        printf("%d | %d \n", i, vecWeights);
	      }
	      */
	  }
	}
    }
    //printf("\n");
  }

  //generateColumns(predictions, classes);

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
