
#include <numeric>

#include <CmdLine.hpp>

using namespace std;

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

Options parse(int argc, char *argv[]) {
  using namespace TCLAP;
  // using namespace string_literals;
  cmdline cmd("NameOfTheExec", ' ');

  Options opt;
  opt.cmdline =
      accumulate(argv, argv + argc, ""s,
                 [&](string acc, const char *arg) { return acc + " " + arg; });

  cmd.add<UnlabeledValueArg<std::string>>(opt.instance_file, "file",
                                          "instance file", true, "", "string");

  cmd.add<ValueArg<int>>(
      opt.verbosity, "", "verbosity",
      "verbosity level (0:silent,1:quiet,2:improvements only,3:verbose", false,
      2, "int");

  cmd.add<ValueArg<int>>(
      opt.class_policy, "", "class_policy",
      "policy for selecting the class "
      "(0:negative,1:positive,2:alternate,3:random uniform,4:biased",
      false, 2, "int");

  cmd.add<ValueArg<int>>(opt.example_policy, "", "example_policy",
                         "policy for selecting the example (0:first,1:random",
                         false, 1, "int");

  cmd.add<ValueArg<int>>(opt.seed, "", "seed", "random seed", false, 12345,
                         "int");

  cmd.add<SwitchArg>(opt.print_sol, "", "print_sol",
                     "print the best found schedule", false);

  cmd.add<SwitchArg>(opt.print_par, "", "print_par", "print the paramters",
                     false);

  cmd.add<SwitchArg>(opt.print_mod, "", "print_mod", "print the model", false);

  cmd.add<SwitchArg>(opt.print_ins, "", "print_ins", "print the instance",
                     false);

  cmd.add<SwitchArg>(opt.print_sol, "", "print_sta", "print the statistics",
                     false);

  cmd.add<SwitchArg>(opt.print_cmd, "", "print_cmd", "print the command-line",
                     false);

  cmd.add<SwitchArg>(opt.verified, "", "verified", "verify the explanations",
                     false);

  // ValueArg (const std::string &flag, const std::string &name, const
  // std::string &desc, bool req, T value, Constraint< T > *constraint, Visitor
  // *v=NULL)

  // Constraint<double> *range = new RangeConstraint<double>(0, 1);
  cmd.add<ValueArg<double>>(opt.sample, "", "sample", "sampling ratio", false,
                            1.0, "double");

  cmd.parse(argc, argv);
  return opt;
}

ostream &Options::display(ostream &os) {
  os << setw(20) << left << "p data file:" << setw(30) << right << instance_file
     << endl
     << setw(20) << left << "p seed:" << setw(30) << right << seed << endl
     << setw(20) << left << "p sample:" << setw(30) << right << sample << endl
     << setw(20) << left << "p verbosity:" << setw(30) << right
     << (verbosity == SILENT
             ? "silent"
             : (verbosity == QUIET
                    ? "quiet"
                    : (verbosity == NORMAL ? "normal" : "yacking")))
     << endl
     << setw(20) << left << "p class policy:" << setw(30) << right
     << (class_policy == NEGATIVE
             ? "negative"
             : (class_policy == POSITIVE
                    ? "positive"
                    : (class_policy == ALTERNATE
                           ? "alternate"
                           : (class_policy == UNIFORM ? "uniform" : "biased"))))
     << endl
     << setw(20) << left << "p example policy:" << setw(30) << right
     << (example_policy == FIRST ? "first" : "random") << endl
     << setw(20) << left << "p verified:" << setw(30) << right
     << (verified ? "yes" : "no") << endl;
  return os;
}
