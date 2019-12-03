

#ifndef __CMDLINE_HPP
#define __CMDLINE_HPP

#include <tclap/CmdLine.h>

using namespace std;

// namespace somename {

class Options {

public:
  // outputs a nice description of all options
  // void describe(std::ostream &);

  // the actual options
  string cmdline; // for reference
  string instance_file;
  string output;

  enum verbosity { SILENT = 0, QUIET, NORMAL, YACKING, SOLVERINFO };
  int verbosity;

  enum class_policy { NEGATIVE = 0, POSITIVE, ALTERNATE, UNIFORM, BIASED };
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

  int seed;

  bool print_sol;
  bool print_par;
  bool print_mod;
  bool print_ins;
  bool print_sta;
  bool print_cmd;

  bool verified;

  double sample;

  Options(){};
  Options(const Options &opt)
      : cmdline(opt.cmdline), instance_file(opt.instance_file),
        verbosity(opt.verbosity), seed(opt.seed), print_sol(opt.print_sol),
        print_par(opt.print_par), print_mod(opt.print_mod),
        print_ins(opt.print_ins), print_sta(opt.print_sta),
        print_cmd(opt.print_cmd) {}

  ostream &display(ostream &os);
};

Options parse(int argc, char *argv[]);
// }

#endif // __CMDLINE_HPP
