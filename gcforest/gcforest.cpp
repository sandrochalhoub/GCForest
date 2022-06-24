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
#include <typeinfo>

ILOSTLBEGIN

using namespace std;
using namespace blossom;

// Returns the prediction of the j-th tree of the forest, for the i-th data point

bool getPrediction(WeightedDataset<int>::List& X, std::vector<WeakClassifier>& classifiers, IloInt j, IloInt i) {
  Tree<double>* sol = &(classifiers[j].T);
  return sol->predict(X[i]);
}

// Column generation method with CPLEX, IN PROGRESS
IloInt generateColumns(IloArray<IloIntArray> decisions) {
  IloEnv env;
  IloInt max_weights = 10;

  try {
      // Model
      IloModel primal(env);
      // Constants
      IloInt forestSize = decisions.getSize();
      IloInt datasetSize = decisions[0].getSize();
      // Variables of the LP
      IloNumVarArray weights(env, forestSize, 0, IloInfinity);
      IloNumVarArray z(env, datasetSize, -IloInfinity, IloInfinity);
      IloNumVar zMin(env, -IloInfinity, IloInfinity);
      // Constraints
      IloRangeArray ct_acc(env, datasetSize);
      IloRangeArray ct_z(env, datasetSize);
      IloRangeArray ct_w(env, forestSize);
      IloRangeArray ct_wSum(env, forestSize);
      
      // Objective function
      //IloObjective obj = IloAdd(primal, IloMinimize(env, zMin));
      primal.add(IloMinimize(env, zMin));

      ///// BUILDING CONSTRAINTS
      // Subject to the accuracy constraint
      for (IloInt i = 0 ; i < datasetSize ; i++) {
	IloExpr expr(env);
	for (IloInt j = 0 ; j < forestSize ; j++) {
	  expr += decisions[j][i] * weights[j] + z[i];
	}
	ct_acc[i] = IloRange(env, 0, expr, IloInfinity);
      }
      primal.add(ct_acc);
      
      // Subject to the constraint on z
      for (IloInt i = 0 ; i < datasetSize ; i++) {
	IloExpr expr(env);
	expr += z[i] - zMin;
	ct_z[i] = IloRange(env, -IloInfinity, expr, 0);
      }
      primal.add(ct_z);

      // Subject to the constraint : positive weights
      for (IloInt j = 0 ; j < forestSize ; j++) {
	ct_w[j] = IloRange(env, 0, weights[j], IloInfinity);
      }
      primal.add(ct_w);

      // Subject to the constraint : bounded sum of weights
      for (IloInt j = 0 ; j < forestSize ; j++) {
	IloExpr expr(env);
	expr += weights[j];
	ct_wSum[j] = IloRange(env, -IloInfinity, expr, max_weights);
      }
      primal.add(ct_wSum);

      /// COLUMN-GENERATION PROCEDURE

      IloCplex primalSolver(primal);
      primalSolver.solve();

      if (primalSolver.solve()) {
        primalSolver.out() << "Solution status: " << primalSolver.getStatus() << endl;
	primalSolver.out() << "Total cost = " << primalSolver.getObjValue() << endl;
      }

      //primalSolver.exportModel("gcforest.lp");
      /*
      //for (;;) {
         /// OPTIMIZE OVER CURRENT PATTERNS
         //primalSolver.solve();
         /// FIND AND ADD A NEW PATTERN
	 
      //}

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
  
  printf("Data size = %lu, among which class zero size = %lu and class one size = %lu. \n\n", data_size, classZero.size(), classOne.size());
  
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
			
			
			for(auto i : classZero) {
				cout << " " << i << ": " << classZero[i] << endl;
				assert(classZero.contain(i));
				if(!classZero.contain(i))
				{
					cout << "erreur\n";
					exit(1);
				}
			}
			cout << "size=" << classZero.size() << endl << training_set->examples[0] << endl;
			
			for(auto i : classOne) {
				cout << " " << i << ": " << classOne[i] << endl;
				assert(classOne.contain(i));
				if(!classOne.contain(i))
				{
					cout << "erreur\n";
					exit(1);
				}
			}
			cout << "size=" << classOne.size() << training_set->examples[1] << endl;
			
			
			
	for (IloInt i = 0 ; i < data_size ; i++) {
	  ////// CLASS ZERO
	  if (i < classZero.size()) {
	    //printf("%d | ", i);
			
			
			assert(classZero.contain(i));


	    bool prediction = getPrediction(classZero, classifiers, j, i);

	    //predictions[j][i] = prediction;
            if (!prediction) decisions[j][i] = 1;
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
			
			assert(classOne.contain(i));


	      bool prediction = getPrediction(classOne, classifiers, j, i - classZero.size());
	      if (prediction) decisions[j][i] = 1;
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
  }
  printf("\n\n");
  generateColumns(decisions);

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
