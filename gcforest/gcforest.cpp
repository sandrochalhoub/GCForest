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
	
	// train() currently returns void. I will probably modify Adaboost for the purpose of gcforest, as to make it return the double array w[j][i].
	// Would allow us to feed it directly into WeightedDataset<int> input prior to working on the CG model.
  A.train();


}
