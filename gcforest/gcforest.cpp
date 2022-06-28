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
#include <iterator>
#include <algorithm>

ILOSTLBEGIN

using namespace std;
using namespace blossom;

// Returns the prediction of the j-th tree of the forest, for the i-th data point
bool getPrediction(WeightedDataset<double>::List& X, std::vector<WeakClassifier>& classifiers, IloInt j, IloInt i) {
  Tree<double>* sol = &(classifiers[j].T);
  return sol->predict(X[i]);
}

// Column generation method with CPLEX, IN PROGRESS
template <template <typename> class ErrorPolicy = WeightedError,
          typename E_t = double>
IloInt generateColumns(DTOptions &opt, WeightedDataset<E_t> *training_set, IloArray<IloIntArray> decisions) {
  IloEnv env;
  IloInt max_weights = 10;

  try {
      // Model
      IloModel primal(env);
      // Constants
      IloInt forest_size = decisions.getSize();
      IloInt data_size = (*training_set)[0].size() + (*training_set)[1].size();
      // Variables of the LP
      IloNumVarArray weights(env, forest_size, 0, IloInfinity);
      IloNumVarArray z(env, data_size, -IloInfinity, IloInfinity);
      IloNumVar zMin(env, -IloInfinity, IloInfinity);
      // Constraints
      IloRangeArray ct_acc(env, data_size);
      IloRangeArray ct_z(env, data_size);
      IloRangeArray ct_w(env, forest_size);
      IloRangeArray ct_wSum(env, 1);
      
      // Objective function
      primal.add(IloMinimize(env, zMin));

      ///// BUILDING CONSTRAINTS
      // Subject to the accuracy constraint
      for (IloInt i = 0 ; i < data_size ; i++) {
	IloExpr expr(env);
	for (IloInt j = 0 ; j < forest_size ; j++) {
	  expr += decisions[j][i] * weights[j] + z[i];
	}
	ct_acc[i] = IloRange(env, 0, expr, IloInfinity);
      }
      primal.add(ct_acc);
      
      // Subject to the constraint on z
      for (IloInt i = 0 ; i < data_size ; i++) {
	IloExpr expr(env);
	expr += z[i] - zMin;
	ct_z[i] = IloRange(env, -IloInfinity, expr, 0);
      }
      primal.add(ct_z);

      // Subject to the constraint : positive weights
      for (IloInt j = 0 ; j < forest_size ; j++) {
	ct_w[j] = IloRange(env, 0, weights[j], IloInfinity);
      }
      primal.add(ct_w);

      // Subject to the constraint : bounded sum of weights
      IloExpr sum(env);
      for (IloInt j = 0 ; j < forest_size ; j++) {
	sum += weights[j];
      }
      ct_wSum[0] = IloRange(env, -IloInfinity, sum, max_weights);
      primal.add(ct_wSum);

      /// COLUMN-GENERATION PROCEDURE

      IloCplex primalSolver(primal);
      if (primalSolver.solve()) {
         primalSolver.out() << "Solution status: " << primalSolver.getStatus() << endl;
         for (IloInt j = 0; j < forest_size ; j++) {
            primalSolver.out() << "   tree " << j << ": "
                        << primalSolver.getValue(weights[j]) << endl;
         }
         primalSolver.out() << "Total cost = " << primalSolver.getObjValue() << endl;
      }
      else primalSolver.out()<< "No solution" << endl;
      primalSolver.printTime();

      // Array (of size data_size) of the alpha dual constraint
      IloNumArray alpha(env);
      primalSolver.getDuals(alpha, ct_acc);
      //for (int i = 0 ; i < alpha.getSize() ; i++) cout << alpha[i] << " |  ";
      printf("\n\n\n");
      // Array (always of size 1) of the beta dual constraint
      IloNumArray beta(env);
      primalSolver.getDuals(beta, ct_wSum);
      //primalSolver.exportModel("gcforest.lp");

      // All data points whose class is 0
      auto classZero{(*training_set)[0]};
      // All data points whose class is 1
      auto classOne{(*training_set)[1]};

      ///// Not working yet : determining a new tree with BacktrackingAlgorithm
      BacktrackingAlgorithm<ErrorPolicy, E_t> B(*training_set, opt);
      int size = classZero.size();
/*
      for (int i = 0 ; i < B.numExample() ; i++) {
	printf("%d | %d | %f \n", size, i, B.getWeight(0, i));
      }
      for (auto i : classZero) {
	B.setWeight(0, i, alpha[i]);
	printf("%f | ", B.getWeight(0, i));
      }
*/
/*
      for (auto i : classOne) {
	B.setWeight(1, i + size, alpha[i]);
	printf("%f | ", B.getWeight(1, i + size));
      }
	printf("\n\n");
*/
/*
      for (int i = 0 ; i < alpha.getSize() ; i++) {
	printf("%f | ", alpha[i]);
      }
      printf("\n\n");
*/
/*
      for (int i = 0 ; i < B.numExample() ; i++) {
	if (B.getWeight(0, i) == 0 && B.getWeight(1, i) != 0) printf("%f | ", B.getWeight(1, i));
	else printf("%f | ", B.getWeight(0, i));
      }
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
          typename E_t = double>
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

  ////// Size of the entire training set prior to the pre-processing
  IloInt data_size = (*training_set)[0].size() + (*training_set)[1].size();

  ////// PREPROCESSING
  if (opt.preprocessing) {
    training_set->preprocess(opt.verbosity >= DTOptions::NORMAL);
  }

  ////// CREATING THE ALGORITHMS
  // Adaboost for the forest initialization.
  Adaboost A(*training_set, opt);

  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d inputtime=" << cpu_time() << endl;

  ////// SOLVING
  A.train();
  printf("\n");

  // All data points whose class is 0
  auto classZero{(*training_set)[0]};
  // All data points whose class is 1
  auto classOne{(*training_set)[1]};
  
  // The forest built by Adaboost
  std::vector<WeakClassifier> classifiers = A.getClassifier();
  ////// CPLEX vectors
  // Decisions vector == 1 if predictions[j][i] == classes[i], -1 otherwise
  IloArray<IloIntArray> decisions(env, classifiers.size());

  ////// BUILDING PREDICTIONS AND WEIGHTS VECTORS
  for (IloInt j = 0 ; j < classifiers.size() ; j++) {
    decisions[j] = IloIntArray(env, data_size, 0, 1, ILOINT);
    //printf("\nTREE NUMBER %d \n\n", j);
    if (opt.verified) {
      for(auto i : classZero) {
	bool prediction = getPrediction(classZero, classifiers, j, i);
	if (!prediction) decisions[j][i] = 1;
	else decisions[j][i] = -1;
	/*   
	cout << " " << i << ": " << classZero[i] << endl;
	assert(classZero.contain(i));
	if(!classZero.contain(i))
	{
	  cout << "erreur\n";
	  exit(1);
	}
	*/
      }
      //cout << "size=" << classZero.size() << endl << training_set->examples[0] << endl;
      for(auto i : classOne) {
	bool prediction = getPrediction(classOne, classifiers, j, i);
	if (prediction) decisions[j][i + classZero.size()] = 1;
	else decisions[j][i + classZero.size()] = -1;
	/*
	cout << " " << i << ": " << classOne[i] << endl;
	assert(classOne.contain(i));
	if(!classOne.contain(i))
	{
	  cout << "erreur\n";
	  exit(1);
	}
	*/
      }
      //cout << "size=" << classOne.size() << training_set->examples[1] << endl;
	
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
    return run_algorithm<WeightedError, double>(opt);
  } else {
    return run_algorithm<>(opt);
  }

}
