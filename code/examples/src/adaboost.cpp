
#include <iostream>
#include <random>
#include <stdlib.h>

#include "WeightedDataset.hpp"
#include "Backtrack.hpp"
#include "Adaboost.hpp"
#include "CSVReader.hpp"
#include "CmdLine.hpp"
#include "Reader.hpp"
#include "Tree.hpp"

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
  if (opt.binarize) {

    read_non_binary(input, opt);

  } else {

    read_binary(input, opt);
  }
	
  if (opt.verbosity >= DTOptions::NORMAL)
    cout << "d readtime=" << cpu_time() << endl;

  input.preprocess(opt.verbosity >= DTOptions::NORMAL);

	Adaboost A(input, opt);
 
	if(opt.verbosity >= DTOptions::NORMAL)
		cout << "d inputtime=" << cpu_time() << endl;

  A.train();
}
