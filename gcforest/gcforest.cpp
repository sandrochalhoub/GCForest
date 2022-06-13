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

int main(int argc, char *argv[]) {
  DTOptions opt = parse_dt(argc, argv);

  if (opt.print_cmd)
    cout << opt.cmdline << endl;

  if (opt.print_par)
    opt.display(cout);

  WeightedDataset<int> input;
	
  ////// READING
  try {
    read_binary(input, opt);
  } catch (const std::exception &e) {
    if (opt.verbosity >= DTOptions::NORMAL)
      cout << "c format not recognized or input non-binary, binarizing...\n";
    read_non_binary(input, opt);
  }
	
  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d readtime=" << cpu_time() << endl;

  input.preprocess(opt.verbosity >= DTOptions::NORMAL);

  Adaboost A(input, opt);
 
  if(opt.verbosity >= DTOptions::NORMAL)
    cout << "d inputtime=" << cpu_time() << endl;
  
  A.train();
  //printf(A.getClassifier()[0].T);
  std::vector<double>* weights = A.getClassifier()[0].T.getWeights();
  printf("%f ", weights[0][0]);



  // Modification of the original Adaboost train() method: it now returns the first weight vector. This will help feed it into Cplex (don't know how for now).
  // Following won't be kept, but I wanted to check how dramatically Adaboost modifies the weights.
  //std::vector<double>* a;
  //int x = 0;
  //int y = 0;
  //for (int i = 0 ; i < 10 ; i++) {
  //  a = A.train();
  //  for (y = 0 ; y < a->size() ; y++) {
	//for (x = 0 ; a[y].size() ; x++) printf("%f ", a[y][x]);
	//printf("\n");
    //}

  //}

}
