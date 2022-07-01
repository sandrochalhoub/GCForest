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

IloInt nb_iter = 0;
IloInt ITERMAX = 200;
IloNum TARGET_ACCURACY = 0.99;
IloNum EPS = 1e-6;

// Column generation function with CPLEX (Work in Progress)
template <template <typename> class ErrorPolicy = WeightedError, typename E_t = double>
IloInt generateColumns(DTOptions &opt, WeightedDataset<E_t> *training_set, IloArray<IloIntArray> decisions) {
  IloEnv env;

  try {
      // Model
      IloModel primal(env);
      // Constants
      IloInt forest_size = decisions.getSize();
      IloInt data_size = (*training_set)[0].size() + (*training_set)[1].size();
      IloInt weights_sum = 1;
      // Variables of the LP
      IloNumVarArray weights(env, forest_size, 0, 1);
      IloNumVarArray z(env, data_size, -IloInfinity, IloInfinity);
      IloNumVar zMin(env, -IloInfinity, IloInfinity);
      // Constraints
      IloRangeArray ct_acc(env, data_size);
      IloRangeArray ct_z(env, data_size);
      IloRangeArray ct_w(env, forest_size);
      IloRangeArray ct_wSum(env, 1);
      // Objective function
      primal.add(IloMinimize(env, zMin));

      //// CONSTRAINTS
      // Subject to the accuracy constraint
      for (IloInt i = 0 ; i < data_size ; i++) {
        IloExpr expr(env);
        for (IloInt j = 0 ; j < forest_size ; j++) {
          expr += decisions[j][i] * weights[j] + z[i];
	      }
	      ct_acc[i] = IloRange(env, EPS, expr, IloInfinity);
      }
      primal.add(ct_acc);
      
      // Subject to the constraint on z
      for (IloInt i = 0 ; i < data_size ; i++) {
        IloExpr expr(env);
        expr += z[i] - zMin;
        ct_z[i] = IloRange(env, -IloInfinity, expr, -EPS);
      }
      primal.add(ct_z);

      // Subject to positive weights
      for (IloInt j = 0 ; j < forest_size ; j++) {
	      ct_w[j] = IloRange(env, 0, weights[j], IloInfinity);
      }
      primal.add(ct_w);

      // Subject to the sum of weights
      IloExpr sum(env);
      for (IloInt j = 0 ; j < forest_size ; j++) {
	      sum += weights[j];
      }
      ct_wSum[0] = IloRange(env, weights_sum, sum, weights_sum);
      primal.add(ct_wSum);

      //// SOLVING
      IloCplex primalSolver(primal);
      if (primalSolver.solve()) {
         primalSolver.out() << "Solution status: " << primalSolver.getStatus() << endl;
         for (IloInt j = 0; j < forest_size ; j++) {
         	primalSolver.out() << "   weight tree " << j << ": " << primalSolver.getValue(weights[j]) << endl;
         }
         primalSolver.out() << "Total cost = " << primalSolver.getObjValue() << endl;
      }
      else {
	      primalSolver.out()<< "No solution" << endl;
	      env.end();
	      return 0;
      }
      primalSolver.printTime();
      //primalSolver.exportModel("gcforest.lp");

      // Dual variables
      IloNumArray alpha(env, data_size);
      primalSolver.getDuals(alpha, ct_acc);
      IloNumArray beta(env, 1);
      primalSolver.getDuals(beta, ct_wSum);

      // All data points whose class is 0
      auto classZero{(*training_set)[0]};
      // All data points whose class is 1
      auto classOne{(*training_set)[1]};

      // Initializing a new tree with weights = alpha
      BacktrackingAlgorithm<ErrorPolicy, E_t> B(*training_set, opt);
      IloInt k=0;
      for (auto i : classZero) {
        B.setWeight(0, k, alpha[k]);
        ++k;
      }
      k=0;
      for (auto i : classOne) {
				B.setWeight(1, k, alpha[k + classZero.size()]);
				++k;
      }
      B.minimize_error();
      Tree<double> sol = B.getSolution();
      
			if (B.accuracy() > TARGET_ACCURACY) {
        env.end();
        return 0;
      }
      
      //// COLUMN-GENERATION (work in progress, stops after ITERMAX iterations)

      // Decision vector of the new tree
      IloIntArray decision(env, data_size);
      IloArray<IloIntArray> new_decisions(env, forest_size + 1);
      
      k=0;
      for(auto i : classZero) {
        //printf("%lu | %d | %lu\n", data_size, i, classZero.size());
        bool prediction = sol.predict(classZero[i]);
        if (!prediction) decision[k] = 1;
        else decision[k] = -1;
        //printf("i=%d, pred %d, decision %d\n", k, prediction, decision[k]);
        k++;
      }
      k = 0;
      for(auto i : classOne) {
        bool prediction = sol.predict(classOne[i]);
        //printf("%lu | %d | %lu\n", data_size, i, classOne.size());
        if (prediction) decision[k + classZero.size()] = 1;
        else decision[k + classZero.size()] = -1;
        //printf("i=%lu, pred %d, decision %d\n", k + classZero.size(), prediction, decision[k + classZero.size()]);
        k++;
			}
			
			// Dual constraint
			IloNum dual_sum = 0.0;
			for (IloInt i = 0 ; i < data_size ; i++) {
				dual_sum += decision[i] * alpha[i];
			}
		  
		  // If the new tree does not violate the dual constraint, it is added to the forest
		  if (dual_sum > beta[0]) {
		  	cout << "\nDual constraint respected, dual_sum = " << dual_sum << " > beta = " << beta[0] << "\n" << endl;
		    // Adding the new tree to the forest
		    for (IloInt j = 0 ; j < forest_size + 1 ; j++) {
		      if (j < forest_size)
		        new_decisions[j] = IloIntArray(decisions[j]);
		      else
		        new_decisions[j] = IloIntArray(decision);
		    }
		    // Repeat if ITERMAX not reached yet
				if (ITERMAX > nb_iter++)
        	generateColumns(opt, training_set, new_decisions);
		  }

	// Otherwise stop
  } catch (IloException& ex) {
      cerr << "Error: " << ex << endl;
  } catch (...) {
      cerr << "Error" << endl;
  }

  env.end();

  return 0;
}

// Returns the prediction of the j-th tree of the forest, for the i-th data point
bool getPrediction(WeightedDataset<double>::List& X, std::vector<WeakClassifier>& classifiers, IloInt j, IloInt i) {
  Tree<double>* sol = &(classifiers[j].T);
  return sol->predict(X[i]);
}

// Forest initialization
template <template <typename> class ErrorPolicy = WeightedError, typename E_t = double>
IloInt init_algorithm(DTOptions &opt) {
  IloEnv env;

  WeightedDataset<E_t> input;

  //// READING
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

    std::vector<int>::iterator endx[2] = {input.examples[0].bbegin(), input.examples[1].bbegin()};

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

  //// Size of the entire training set prior to the pre-processing
  IloInt data_size = (*training_set)[0].size() + (*training_set)[1].size();

  //// PREPROCESSING
  if (opt.preprocessing) {
    training_set->preprocess(opt.verbosity >= DTOptions::NORMAL);
  }

  //// FIRST FOREST WITH ADABOOST
  Adaboost A(*training_set, opt);

  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d inputtime=" << cpu_time() << endl;

  //// SOLVING
  A.train();
  // If the forest built by Adaboost is already good enough, column generation is unnecessary
  if (A.get_accuracy() >= TARGET_ACCURACY) return 0;
  printf("\n");

  // All data points whose class is 0
  auto classZero{(*training_set)[0]};
  // All data points whose class is 1
  auto classOne{(*training_set)[1]};
  
  // The forest obtained from Adaboost
  std::vector<WeakClassifier> classifiers = A.getClassifier();

  // Decisions vector == 1 if predictions[j][i] == classes[i], -1 otherwise
  IloArray<IloIntArray> decisions(env, classifiers.size());

  //// BUILDING DECISION VECTORS
  for (IloInt j = 0 ; j < classifiers.size() ; j++) {
    decisions[j] = IloIntArray(env, data_size);
    if (opt.verified) {
      IloInt k = 0;
      for(auto i : classZero) {
        //printf("%lu | %d | %lu\n", data_size, i, classZero.size());
        bool prediction = getPrediction(classZero, classifiers, j, i);
        if (!prediction) decisions[j][k] = 1;
        else decisions[j][k] = -1;
        k++;
      }
      k = 0;
      for(auto i : classOne) {
        //printf("%lu | %d | %lu\n", data_size, i, classOne.size());
        bool prediction = getPrediction(classOne, classifiers, j, i);
        if (prediction) decisions[j][k + classZero.size()] = 1;
        else decisions[j][k + classZero.size()] = -1;
        k++;
      }
    }
  }
  printf("\n\n");
  generateColumns(opt, training_set, decisions);

  return 0;
}

int main(int argc, char *argv[]) {
  DTOptions opt = parse_dt(argc, argv);

  if (opt.print_cmd)
    cout << opt.cmdline << endl;

  if (opt.print_par)
    opt.display(cout);

  if (opt.preprocessing) {
    return init_algorithm<WeightedError, double>(opt);
  } else {
    return init_algorithm<>(opt);
  }

}

