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

  std::vector<double>* a = A.train();
  std::vector<WeakClassifier> classifiers = A.getClassifier();
  // printf("%lu \n\n\n", classifiers.size());
  std::vector<double>* weights;

  // These nested for loops print the correct weight values, exactly as returned by the train() function (later on, train() won't be returning the vector anymore, this is just for testing).
  // Still wondering about the upper bound of x.
  //for (int x = 0 ; x < input.example_count(); x++) {
    //printf("%f  |  ", a[0][x]);
    //printf("%f  |  ", a[1][x]);
  //}

  printf("\n\n\n\n\n\n");


  // These nested for loops print the weight values as obtained with the getters I implemented, for now they don't match those returned by train();  
  for (int i = 0 ; i < classifiers.size() ; i++) {
    weights = classifiers[i].T.getWeights();
    for (int x = 0 ; x < weights->size() ; x++) {
	printf("%f  |  ", weights[0][x]);
	printf("%f  |  ", weights[1][x]);
    }
  }

}
