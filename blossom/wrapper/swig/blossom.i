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
	  string tree_file;
	  string debug;
	  string output;
	  string format;

	  enum verbosity { SILENT = 0, QUIET, NORMAL, YACKING, SOLVERINFO };
	  int verbosity;

	  int seed;

	  bool print_sol;
	  bool print_par;
	  bool print_ins;
	  bool print_sta;
	  bool print_cmd;

	  bool verified;

	  double test_sample;

	  int width;
	  double focus;
	  int max_depth;

	  int restart_base;
	  double restart_factor;

	  bool filterfeature;

	  double time;

	  int search;

	  bool bounding;

	  enum node_strategy { FIRST = 0, RANDOM = 1, ERROR = 2, ERROR_REDUCTION = 3, ANTIERROR = 4 };
  	int node_strategy;

  	enum feature_strategy { MINERROR = 0, ENTROPY = 1, GINI = 2, HYBRID = 3 };
  	int feature_strategy;

/*	  bool binarize;*/
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

	  double pruning;

  	bool sample_only;

	  DTOptions(){};
	  DTOptions(const DTOptions &opt);
/*	      : cmdline(opt.cmdline), instance_file(opt.instance_file),
        tree_file(opt.tree_file), debug(opt.debug), output(opt.output),
        format(opt.format), verbosity(opt.verbosity), seed(opt.seed),
        print_sol(opt.print_sol), print_par(opt.print_par),
        print_ins(opt.print_ins), print_sta(opt.print_sta),
        print_cmd(opt.print_cmd), verified(opt.verified),
        test_sample(opt.test_sample), width(opt.width), focus(opt.focus),
        max_depth(opt.max_depth), restart_base(opt.restart_base),
        restart_factor(opt.restart_factor), time(opt.time), search(opt.search),
        bounding(opt.bounding), node_strategy(opt.node_strategy),
        feature_strategy(opt.feature_strategy), split(opt.split),
        ada_it(opt.ada_it), ada_stop(opt.ada_stop), filter(opt.filter),
        reference_class(opt.reference_class), mindepth(opt.mindepth),
        minsize(opt.minsize), preprocessing(opt.preprocessing),
        progress(opt.progress), delimiter(opt.delimiter),
        intarget(opt.intarget), outtarget(opt.outtarget), pruning(opt.pruning),
        sample_only(opt.sample_only) {} */

	  ostream &display(ostream &os);
	};
	
	
  // Tree

	template <typename E_t>
  class Wood {
  public:
    Wood();
  };

	template <typename E_t>
  class Tree {
  public:
    int idx;

    Tree() = delete;
    Tree(Wood<E_t>*, int i);
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
    Tree<E_t> getSolution();
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
	
/*	%template(WoodI) Wood<int>;
	%template(WoodD) Wood<double>;*/

	%template(TreeI) Tree<int>;
	%template(TreeD) Tree<double>;
	
	%template(WeightedDatasetI) WeightedDataset<int>;
	%template(WeightedDatasetD) WeightedDataset<double>;
	
  %template(BacktrackingAlgo) BacktrackingAlgorithm<CardinalityError, int>;
  %template(WeightedBacktrackingAlgo) BacktrackingAlgorithm<WeightedError, int>;
  %template(WeightedBacktrackingAlgod) BacktrackingAlgorithm<WeightedError, double>;
}
