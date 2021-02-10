

#ifndef __CMDLINE_HPP
#define __CMDLINE_HPP

#include <tclap/CmdLine.h>

using namespace std;

namespace blossom {

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

  enum verbosity { SILENT = 0, QUIET, NORMAL, YACKING, SOLVERINFO };
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

  bool binarize;
  double split;
  int ada_it;
  int ada_stop;

  bool filter;
  string reference_class;

  bool mindepth;
  bool minsize;

  bool preprocessing;
  // bool filterfeature_inconsistent;

  bool progress;

  string delimiter;
  int intarget;
  int outtarget;

  double pruning;

  DTOptions(){};
  DTOptions(const DTOptions &opt)
      : cmdline(opt.cmdline), instance_file(opt.instance_file),
        debug(opt.debug), output(opt.output), format(opt.format),
        verbosity(opt.verbosity), seed(opt.seed), print_sol(opt.print_sol),
        print_par(opt.print_par), print_ins(opt.print_ins),
        print_sta(opt.print_sta), print_cmd(opt.print_cmd),
        verified(opt.verified), sample(opt.sample), width(opt.width),
        focus(opt.focus), max_depth(opt.max_depth),
        restart_base(opt.restart_base), restart_factor(opt.restart_factor),
        time(opt.time), search(opt.search), bounding(opt.bounding),
        node_strategy(opt.node_strategy),
        feature_strategy(opt.feature_strategy), binarize(opt.binarize),
        split(opt.split), ada_it(opt.ada_it), ada_stop(opt.ada_stop),
        filter(opt.filter), reference_class(opt.reference_class),
        mindepth(opt.mindepth), minsize(opt.minsize),
        preprocessing(opt.preprocessing), progress(opt.progress),
        delimiter(opt.delimiter), intarget(opt.intarget),
        outtarget(opt.outtarget), pruning(opt.pruning) {}

  ostream &display(ostream &os);
};

DTOptions parse_dt(int argc, char *argv[]);
}

#endif // __CMDLINE_HPP
