

#ifndef __CMDLINE_HPP
#define __CMDLINE_HPP

#include <tclap/CmdLine.h>

using namespace std;

// namespace somename {

class PrimerOptions {

public:
  // outputs a nice description of all options
  // void describe(std::ostream &);

  // the actual options
  string cmdline; // for reference
  string instance_file;
  string debug;
  string output;

  enum verbosity { SILENT = 0, QUIET, NORMAL, YACKING, SOLVERINFO };
  int verbosity;

  enum class_policy {
    NEGATIVE = 0,
    POSITIVE,
    SMALLEST,
    LARGEST,
    ALTERNATE,
    UNIFORM,
    BIASED,
    ANTI
  };
  int class_policy;

  enum example_policy {
		LOWEST_PROBABILITY = -1,
    FIRST = 0,
    RANDOM = 1,
    HIGHEST_PROBABILITY = 2
  };
  int example_policy;

  enum feature_policy { MIN = 0, LOWEST_ENTROPY, HIGHEST_ENTROPY };
  int feature_policy;

  bool bayesian;

  int max_iteration;

  string delimiter;
  string wildcard;
  bool original;
  bool reduced;

  string caption;

  string format;

  bool mapping;

  int seed;

  bool print_sol;
  bool print_par;
  bool print_mod;
  bool print_ins;
  bool print_sta;
  bool print_cmd;

  bool verified;

  double sample;

  int width;
  double focus;
  int max_size;
  int max_depth;

  int restart_base;
  double restart_factor;

  bool filter;

  PrimerOptions(){};
  PrimerOptions(const PrimerOptions &opt)
      : cmdline(opt.cmdline), instance_file(opt.instance_file),
        debug(opt.debug), verbosity(opt.verbosity),
        class_policy(opt.class_policy), example_policy(opt.example_policy),
        feature_policy(opt.feature_policy), bayesian(opt.bayesian),
        max_iteration(opt.max_iteration), delimiter(opt.delimiter),
        wildcard(opt.wildcard), original(opt.original), reduced(opt.reduced),
        caption(opt.caption), format(opt.format), mapping(opt.mapping),
        seed(opt.seed), print_sol(opt.print_sol), print_par(opt.print_par),
        print_mod(opt.print_mod), print_ins(opt.print_ins),
        print_sta(opt.print_sta), print_cmd(opt.print_cmd),
        verified(opt.verified), sample(opt.sample), width(opt.width),
        focus(opt.focus), max_size(opt.max_size), max_depth(opt.max_depth),
        restart_base(opt.restart_base), restart_factor(opt.restart_factor),
        filter(opt.filter) {}

  ostream &display(ostream &os);
};

PrimerOptions parse_primer(int argc, char *argv[]);

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
  int max_size;
  int max_depth;

  int restart_base;
  double restart_factor;

  bool filter;

  DTOptions(){};
  DTOptions(const DTOptions &opt)
      : cmdline(opt.cmdline), instance_file(opt.instance_file),
        debug(opt.debug), output(opt.output), format(opt.format),
        verbosity(opt.verbosity), seed(opt.seed), print_sol(opt.print_sol),
        print_par(opt.print_par), print_ins(opt.print_ins),
        print_sta(opt.print_sta), print_cmd(opt.print_cmd),
        verified(opt.verified), sample(opt.sample), width(opt.width),
        focus(opt.focus), max_size(opt.max_size), max_depth(opt.max_depth),
        restart_base(opt.restart_base), restart_factor(opt.restart_factor),
        filter(opt.filter) {}

  ostream &display(ostream &os);
};

DTOptions parse_dt(int argc, char *argv[]);
// }

#endif // __CMDLINE_HPP
