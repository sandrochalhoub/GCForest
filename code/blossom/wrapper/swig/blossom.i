%module blossom
%include "std_vector.i"
%include "std_string.i"

%{
#include "blossom.h"
%}

extern blossom::DTOptions parse(std::vector<std::string> params);
extern void read_binary(blossom::WeightedDataset<int> &input, blossom::DTOptions &opt);

namespace std {
  %template(int_vec) vector<int>;
  %template(cstr_vec) vector<char*>;
  %template(str_vec) vector<string>;
};

namespace blossom {
	
	// DTOptions

	class DTOptions {

	public:
	  // outputs a nice description of all options
	  // void describe(std::ostream &);

	  // the actual options
	  string cmdline; // for reference
	  string instance_file;
	  string debug;
	  string output;
	  string format;

	  int verbosity;

	  int seed;

	  bool print_sol;
	  bool print_par;
	  bool print_ins;
	  bool print_sta;
	  bool print_cmd;

	  bool verified;

	  double sample;

	  int width;
	  double focus;
	  // int max_size;
	  int max_depth;

	  int restart_base;
	  double restart_factor;

	  bool filter;

	  double time;

	  int search;

	  bool bounding;

	  int node_strategy;

	  int feature_strategy;

	  bool binarize;
	  double split;
	  int ada_it;
	  int ada_stop;

	  bool filter;
	  string reference_class;

	  bool mindepth;
	  bool minsize;

	  bool preprocessing;

	  bool progress;

	  string delimiter;
	  int intarget;
	  int outtarget;

	  DTOptions(){};
	  DTOptions(const DTOptions &opt);
/*	      : cmdline(opt.cmdline), instance_file(opt.instance_file),
	        debug(opt.debug), output(opt.output), format(opt.format),
	        verbosity(opt.verbosity), seed(opt.seed), print_sol(opt.print_sol),
	        print_par(opt.print_par), print_ins(opt.print_ins),
	        print_sta(opt.print_sta), print_cmd(opt.print_cmd),
	        verified(opt.verified), sample(opt.sample), width(opt.width),
	        focus(opt.focus), max_depth(opt.max_depth),
	        restart_base(opt.restart_base), restart_factor(opt.restart_factor),
	        filter(opt.filter), time(opt.time), search(opt.search),
	        bounding(opt.bounding), node_strategy(opt.node_strategy),
	        feature_strategy(opt.feature_strategy), binarize(opt.binarize),
	        split(opt.split), ada_it(opt.ada_it), ada_stop(opt.ada_stop),
	        filter(opt.filter), reference_class(opt.reference_class),
	        mindepth(opt.mindepth), minsize(opt.minsize),
	        preprocessing(opt.preprocessing), progress(opt.progress),
	        delimiter(opt.delimiter), intarget(opt.intarget),
	        outtarget(opt.outtarget) {}*/

	  ostream &display(ostream &os);
	};
	
	
  // Tree

  class Wood {
  public:
    Wood();
  };

  class Tree {
  public:
    int idx;

    Tree() = delete;
    Tree(const Wood*, int i);
    int getChild(int node, int branch);
    int getFeature(int node);
  };


	// WeightedDataset

	template <typename E_t>
	class WeightedDataset {
	public:
	  WeightedDataset();

		void addExample(const std::vector<int>& row);
		void preprocess(const bool verbose=false);
/*	  void printDatasetToTextFile(ostream &outfile, const bool first = true) const;
	  void printDatasetToCSVFile(ostream &outfile, const string &delimiter = ",",
	                             const bool first = false) const;*/

	};

  // BacktrackingAlgorithm

  template <template<typename> class ErrorPolicy, typename E_t>
  class BacktrackingAlgorithm {
  public:
    BacktrackingAlgorithm() = delete;
		BacktrackingAlgorithm(const DTOptions &o);
    BacktrackingAlgorithm(const WeightedDataset<E_t>& d, const DTOptions &o);
		void load(const WeightedDataset<E_t> &input);
    void minimize_error();
    void minimize_error_depth();
    void minimize_error_depth_size();
    Tree getSolution();
  };
	
	

  template <class ErrorType> class CardinalityError;
  template <class ErrorType> class WeightedError;
	
	
  class Adaboost {
  public:
    DTOptions &options;

    Adaboost() = delete;
    Adaboost(WeightedDataset<int> &d, DTOptions &opt);
    void train();
    bool predict(const std::vector<int> &example) const;
/*	  void addBitsetExample(const dynamic_bitset<> &sample, const bool y, const size_t weight);*/
  };
	
	%template(WeightedDatasetI) WeightedDataset<int>;
	%template(WeightedDatasetD) WeightedDataset<double>;
	

  %template(BacktrackingAlgo) BacktrackingAlgorithm<CardinalityError, int>;
  %template(WeightedBacktrackingAlgo) BacktrackingAlgorithm<WeightedError, int>;
  %template(WeightedBacktrackingAlgod) BacktrackingAlgorithm<WeightedError, double>;
}
