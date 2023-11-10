
#include <limits>
#include <numeric>

#include <CmdLine.hpp>

using namespace std;

using namespace blossom;

struct argbase {
  virtual ~argbase() {}
  virtual void assign() = 0;
};

template <typename Opt, typename ClapArg, typename E = void>
struct arg : public argbase {
  ClapArg carg;
  Opt &opt;

  template <typename... T>
  arg(TCLAP::CmdLine &cmd, Opt &opt, T &&... args)
      : carg(std::forward<T>(args)...), opt(opt) {
    cmd.add(carg);
  }

  virtual void assign() override { opt = carg.getValue(); }
};

template <typename Opt, typename ClapArg>
struct arg<Opt, ClapArg, typename std::enable_if<std::is_enum<Opt>{}>::type>
    : public argbase {
  ClapArg carg;
  Opt &opt;

  template <typename... T>
  arg(TCLAP::CmdLine &cmd, Opt &opt, T &&... args)
      : carg(std::forward<T>(args)...), opt(opt) {
    cmd.add(carg);
  }

  virtual void assign() override {
    opt =
        static_cast<typename std::remove_reference<Opt>::type>(carg.getValue());
  }
};

struct cmdline {
  TCLAP::CmdLine cmd;
  std::vector<std::unique_ptr<argbase>> args;

  cmdline(const std::string &message, const char delimiter = ' ',
          const std::string &version = "none", bool helpAndVersion = true)
      : cmd(message, delimiter, version, helpAndVersion) {}

  template <typename ClapArg, typename Opt, typename... T>
  void add(Opt &opt, T &&... clapargs) {
    args.emplace_back(std::move(std::make_unique<arg<Opt, ClapArg>>(
        cmd, opt, std::forward<T>(clapargs)...)));
  }

  void parse(int argc, char *argv[]) {
    cmd.parse(argc, argv);
    for (auto &arg : args)
      arg->assign();
  }
};

template <class T> class RangeConstraint : virtual public TCLAP::Constraint<T> {
private:
  T lb;
  T ub;

public:
  RangeConstraint(T l, T u) : lb(l), ub(u) {}

  /**
  * Returns a description of the Constraint.
  */
  virtual std::string description() const {
    return "value must be in the range [" + to_string(lb) + "," +
           to_string(ub) + "]";
  }

  /**
  * Returns the short ID for the Constraint.
  */
  virtual std::string shortID() const { return "range"; }

  /**
  * The method used to verify that the value parsed from the command
  * line meets the constraint.
  * \param value - The value that will be checked.
  */
  virtual bool check(const T &value) const {
    return value >= lb and value <= ub;
  }

  /**
  * Destructor.
  * Silences warnings about Constraint being a base class with virtual
  * functions but without a virtual destructor.
  */
  virtual ~RangeConstraint() { ; }
};


DTOptions blossom::parse_dt(int argc, char *argv[]) {
  using namespace TCLAP;
  // using namespace string_literals;
  cmdline cmd("dt", ' ');

  DTOptions opt;
  opt.cmdline =
      accumulate(argv, argv + argc, ""s,
                 [&](string acc, const char *arg) { return acc + " " + arg; });

  cmd.add<UnlabeledValueArg<std::string>>(opt.instance_file, "file",
                                          "instance file", true, "", "string");

  // cmd.add<ValueArg<string>>(opt.test, "", "test", "test file", false, "",
  //                           "string");

  cmd.add<ValueArg<string>>(opt.tree_file, "", "tree_file", "tree file", false,
                            "", "string");

  cmd.add<ValueArg<string>>(opt.debug, "", "debug", "debug file", false, "",
                            "string");

  cmd.add<ValueArg<string>>(opt.output, "o", "output", "output file", false, "",
                            "string");

  cmd.add<ValueArg<string>>(opt.format, "", "format",
                            "input format (csv or txt, default:guess)", false,
                            "guess", "string");

  cmd.add<ValueArg<int>>(
      opt.verbosity, "v", "verbosity",
      "verbosity level (0:silent,1:quiet,2:improvements only,3:verbose", false,
      2, "int");

  cmd.add<ValueArg<int>>(opt.seed, "s", "seed", "random seed", false, 12345,
                         "int");

  cmd.add<SwitchArg>(opt.print_sol, "", "print_sol",
                     "print the best found schedule", false);

  cmd.add<SwitchArg>(opt.print_par, "", "print_par", "print the paramters",
                     false);

  cmd.add<SwitchArg>(opt.print_ins, "", "print_ins", "print the instance",
                     false);

  cmd.add<SwitchArg>(opt.print_sta, "", "print_sta", "print the statistics",
                     false);

  cmd.add<SwitchArg>(opt.print_cmd, "", "print_cmd", "print the command-line",
                     false);

  cmd.add<SwitchArg>(opt.verified, "", "verified",
                     "switch tree verification on", false);

  cmd.add<SwitchArg>(opt.verified, "", "noverification",
                     "switch tree verification off", true);
                     
  //Reminder
  cmd.add<ValueArg<int>>(opt.itermax, "", "itermax", "max number of iterations for gcforest / solutions for blossom",
  			 false, numeric_limits<int>::max(), "int");
  
  cmd.add<ValueArg<int>>(opt.obj_check, "", "obj_check", "number of iterations after which the objective is checked",
  			 false, 1, "int");
  
  cmd.add<ValueArg<double>>(opt.obj_eps, "", "obj_eps", "objective improvement under which gcforest stops",
  			    false, 1e-6, "double");
  
  cmd.add<SwitchArg>(opt.bounding, "", "bounding", "switch bound reasoning on",
                     false);

  cmd.add<SwitchArg>(opt.bounding, "", "nolb", "switch bound reasoning off",
                     true);

  cmd.add<ValueArg<double>>(opt.split, "", "split", "proportion of examples in the test set",
                            false, 0.0, "double");

  cmd.add<ValueArg<int>>(opt.ada_it, "", "ada_it", "maximum number of iterations for adaboost",
                         false, 30, "int");

  cmd.add<ValueArg<int>>(opt.ada_stop, "", "ada_stop", "number of iteration without any improvement "
                        "of accuracy after which Adaboost stops", false, 0, "int");

  cmd.add<SwitchArg>(opt.mindepth, "", "depthobjective",
                     "switch depth objective on", false);

  cmd.add<SwitchArg>(opt.mindepth, "", "nodepthobjective",
                     "switch depth objective off", true);

  cmd.add<SwitchArg>(opt.minsize, "", "sizeobjective",
                     "switch size objective on", false);

  cmd.add<SwitchArg>(opt.minsize, "", "nosizeobjective", "switch size objective off",
                     true);

  cmd.add<SwitchArg>(opt.filter, "", "filter",
                     "remove redundant features before solving", false);

  cmd.add<SwitchArg>(opt.filter, "", "nofilter",
                     "switch off the removal of redundant features", true);

  cmd.add<ValueArg<string>>(opt.reference_class, "", "class", "reference class",
                            false, "", "string");

  // ValueArg (const std::string &flag, const std::string &name, const
  // std::string &desc, bool req, T value, Constraint< T > *constraint, Visitor
  // *v=NULL)

  // Constraint<double> *range = new RangeConstraint<double>(0, 1);
  cmd.add<ValueArg<double>>(opt.test_sample, "", "test_sample",
                            "sampling ratio (test)", false, 0.0, "double");

  // Constraint<double> *range = new RangeConstraint<double>(0, 1);
  cmd.add<ValueArg<double>>(opt.sample, "", "sample", "sampling ratio (train)",
                            false, 1.0, "double");

  cmd.add<ValueArg<int>>(opt.width, "", "width",
                         "number of tied features for random selection", false,
                         1, "int");

  cmd.add<ValueArg<double>>(opt.focus, "", "focus",
                            "probability of choosing the best feature", false,
                            .9, "int");

  // cmd.add<ValueArg<int>>(opt.max_size, "", "max_size",
  //                        "maximum number of nodes in the tree", false,
  //                        numeric_limits<int>::max(), "int");

  cmd.add<ValueArg<int>>(opt.max_depth, "", "max_depth",
                         "maximum depth of the tree", false,
                         numeric_limits<int>::max(), "int");

  cmd.add<ValueArg<int>>(opt.restart_base, "", "restart_base",
                         "number of backtracks before first restart", false, -1,
                         "int");

  cmd.add<ValueArg<double>>(opt.restart_factor, "", "restart_factor",
                            "geometric factor", false, 1.1, "double");

  cmd.add<ValueArg<double>>(opt.time, "", "time", "time limit", false, 0,
                            "double");

  cmd.add<ValueArg<int>>(opt.search, "", "search", "search limit", false, 0,
                         "int");

  cmd.add<ValueArg<int>>(opt.node_strategy, "", "node_strategy",
                         "node selection strategy 0:first, 1:random, 2:max. "
                         "error 3:max. reduction",
                         false, 0, "int");

  cmd.add<ValueArg<int>>(opt.feature_strategy, "", "feature_strategy",
                         "feature selection strategy 0:min error, 1:min "
                         "entropy, 2:min gini impurity ",
                         false, 2, "int");

  cmd.add<SwitchArg>(opt.preprocessing, "", "preprocessing",
                     "switch test_sample preprocessing on", false);

  cmd.add<SwitchArg>(opt.preprocessing, "", "nopreprocessing",
                     "switch test_sample preprocessing off", true);

  cmd.add<SwitchArg>(opt.progress, "", "progress", "print progress", false);

  cmd.add<ValueArg<int>>(opt.intarget, "", "intarget",
                         "target feature when writing (use column <target % "
                         "#columns> as target class)",
                         false, 0, "int");

  cmd.add<ValueArg<int>>(opt.outtarget, "", "outtarget",
                         "target feature when writing can only be first (0) or "
                         "last (-1)",
                         false, 1, "int");

  cmd.add<ValueArg<string>>(opt.delimiter, "", "delimiter",
                            "delimiter used when writing a csv file", false,
                            ",", "string");
  // cmd.add<ValueArg<string>>(opt.delimiter, "", "delimiter",
  //                           "delimiter used when writing a csv file", false,
  //                           ",", "string");

  cmd.add<ValueArg<double>>(opt.pruning, "", "pruning", "pruning (maximum)",
                            false, 0, "double");

  cmd.add<SwitchArg>(opt.sample_only, "", "sample_only", "stop after sampling",
                     false);

  cmd.parse(argc, argv);
  return opt;
}

ostream &DTOptions::display(ostream &os) {
  os << setw(20) << left << "p data file:" << setw(30) << right << instance_file
     << endl
     << setw(20) << left << "p seed:" << setw(30) << right << seed << endl
     // << setw(20) << left << "p sampling ratio:" << setw(30) << right <<
     // test_sample
     // << endl
     << setw(20) << left << "p minimize depth:" << setw(30) << right
     << (mindepth ? "yes" : "no") << endl
     << setw(20) << left << "p minimize size:" << setw(30) << right
     << (mindepth and minsize ? "yes" : "no") << endl
     << setw(20) << left << "p verbosity:" << setw(30) << right
     << (verbosity == SILENT
             ? "silent"
             : (verbosity == QUIET
                    ? "quiet"
                    : (verbosity == NORMAL ? "normal" : "yacking")))
     << endl
     << setw(20) << left << "p verified:" << setw(30) << right
     << (verified ? "yes" : "no") << endl
     << setw(20) << left << "p filter data set:" << setw(30) << right
     << (filter ? "yes" : "no") << endl
     << setw(20) << left << "p bound cuts:" << setw(30) << right
     << (bounding ? "yes" : "no") << endl
     << setw(20) << left << "p randomization:" << setw(30) << right;

  if (width == 1 or focus == 1)
    os << "no" << endl;
  else {
    stringstream ss;
    ss << "among " << width << " best w prob. " << (1 - focus);
    os << ss.str() << endl;
  }

  os << setw(20) << left << "p restart:" << setw(30) << right;

  if (restart_base < 0)
    os << "no" << endl;
  else {
    stringstream ss;
    ss << "base " << restart_base << " factor " << restart_factor;
    os << ss.str() << endl;
  }

  os << setw(20) << left << "p node strategy:" << setw(30) << right
     << (node_strategy == FIRST
             ? "first"
             : (node_strategy == RANDOM
                    ? "random"
                    : (node_strategy == ERROR ? "max error"
                                              : (node_strategy == ERROR_REDUCTION ? "max error reduction"
																																									: "min error"))))
     << endl
     << setw(20) << left << "p feature strategy:" << setw(30) << right;

  // if (feature_strategy > 2) {
  //   stringstream ss;
  //   ss << "E -> min err. @" << feature_strategy << "-th solution";
  //   os << ss.str();
  // } else
	if (feature_strategy == MINERROR) {
    os << "min error";
  } else if (feature_strategy == ENTROPY) {
    os << "min entropy";
  } else if (feature_strategy == GINI) {
    os << "min gini impurity";
  } else {
    os << "also gini";
  }
  os << endl
     << setw(20) << left << "p maximum depth:" << setw(30) << right << max_depth
     << endl
     << setw(20) << left << "p preprocessing:" << setw(30) << right
     << (preprocessing ? "yes" : "no") << endl;
  // << setw(20) << left << "p maximum size:" << setw(30) << right << max_size
  // << endl;

  return os;
}
