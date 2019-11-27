
#include <iostream>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include "CmdLine.hpp"
#include "SparseSet.hpp"

#ifndef _PRIMER_DATASET_HPP
#define _PRIMER_DATASET_HPP

using namespace boost;
using namespace std;

typedef dynamic_bitset<> instance;

namespace primer {

/**********************************************
* DataSet
**********************************************/
/// Representation of a list of examples
class DataSet {

private:
  string pretty(const int f) const;

public:
  /*!@name Parameters*/
  //@{
  /// list of features
  std::vector<std::string> feature_label;

  // we store the example followed by its negation
  // irrelevant bits are 0 in both copies
	// X is a vector containing all examples and explanations/cubes
  std::vector<instance> X;

	// example[0] (resp. example[1]) is a set of indices of negative (resp. positive) examples in X
  SparseSet example[2];
	
	// explanations[0] (resp. explanations[1]) is a set of indices of negative (resp. positive) explanations in X
  SparseSet explanations[2];


  std::vector<int> literal_count;
  //@}

public:
  /*!@name Constructors*/
  //@{
  explicit DataSet();
  explicit DataSet(const int n_feature);

  void reserve(const size_t n);
  //@}

  /*!@name Accessors*/
  //@{
  instance &operator[](const size_t idx);

  template <typename RandomIt> void setFeatures(RandomIt beg, RandomIt end);
  void addFeature(string &f);
  void add(instance x_, const bool y);
  void push(instance x_, const bool y);

  template <typename RandomIt> void add(RandomIt beg, RandomIt end);

  size_t numFeature() const;
  size_t size() const;
  size_t count() const;
  size_t volume() const;

  instance NOT(instance &e) const;

  double conditional_entropy(const int feature);
  //@}

  /*!@name List Manipulation*/
  //@{
  // keeps only n positive examples, chosen randomly with a uniform distribution
  template <class random>
  void uniform_sample(const int c, const size_t n, random generator);
  //@}

  /*!@name Business*/
  //@{
  // look for examples that are both positive and negative and remove them
  void filter();

  // the set of true features of x1 that are false in x2
  // we cannot do set difference because features might neighter be true nor
  // false
  void getContradictingFeatures(instance &x1, instance &x2, instance &buffer);

  void addExplanation(instance &impl, const bool y, const int limit,
                      vector<int> &entailed);

  template <typename R>
  void computeDecisionSet(Options &opt, R &random_generator);

  void close();

  bool classify(instance &x);
  //@}

  /*!@name Miscellaneous*/
  //@{
  std::ostream &to_csv(std::ostream &os) const;

  std::ostream &to_txt(std::ostream &os) const;

  std::ostream &display(std::ostream &os) const;

  std::ostream &display_example(std::ostream &os, const instance e) const;
  //@}

  /*!@name Verification*/
  //@{
  void verify();
  //@}
};

template <typename RandomIt>
void DataSet::setFeatures(RandomIt beg, RandomIt end) {
  auto n_feature = (end - beg);
  feature_label.reserve(n_feature);
  for (auto f{beg}; f != end; ++f)
    addFeature(*f);
}

template <typename RandomIt> void DataSet::add(RandomIt beg, RandomIt end) {
  instance x;
  x.resize(numFeature(), false);
  x.resize(2 * numFeature(), true);
  for (auto s{beg}; s != end - 1; ++s) {
    std::stringstream convert(*s);

    bool v{false};
    convert >> v;

    if (v) {
      x.set(s - beg);
      x.reset(s - beg + numFeature());
    }
  }

  std::stringstream convert(*(end - 1));

  bool y{false};
  convert >> y;

  example[y].safe_add(X.size());
  X.push_back(x);
  // add(x, y);
  }

  template <class random>
  void DataSet::uniform_sample(const int c, const size_t n, random generator) {
    while (example[c].count() > n) {
      example[c].remove_back(example[c][generator() % example[c].count()]);
    }
  }

  template <typename R>
  void DataSet::computeDecisionSet(Options &opt, R &random_generator) {
    auto c{0};

    // verify();

    instance contradicting_features;
    instance candidates;
    instance implicant;

    vector<int> removed;

    contradicting_features.resize(2 * numFeature());

    auto last_example{X.size() - 1};

    size_t num_original[2];
    for (auto i{0}; i < 2; ++i)
      num_original[i] = example[i].count();

    while (true) {

      if (num_original[0] + num_original[1] == 0)
        break;
      if (num_original[1] == 0)
        c = 0;
      else if (num_original[0] == 0)
        c = 1;
      else {
        if (opt.class_policy == Options::BIASED)
          c = ((random_generator() % (num_original[0] + num_original[1])) >
               num_original[0]);
        else if (opt.class_policy == Options::UNIFORM)
          c = random_generator() % 2;
        else if (opt.class_policy == Options::POSITIVE)
          c = 1;
        else if (opt.class_policy == Options::NEGATIVE)
          c = 0;
        else
          c = 1 - c;
      }

      auto i{0};

      if (opt.example_policy == Options::RANDOM)
        i = example[c].any(num_original[c], random_generator);
      else
        i = example[c].front();

      assert(i <= last_example);

      // now X[i] is the first remaining example of class c
      if (opt.verbosity >= Options::SOLVERINFO) {
        cout << "compute a rule from the " << (c ? "positive" : "negative")
             << " example " << i << ":";
        display_example(cout, X[i]);
        cout << endl;
      }

      // implicant is empty
      implicant.clear();
      implicant.resize(2 * numFeature(), false);

      // cnadidates contains all features
      candidates.clear();
      candidates.resize(2 * numFeature(), true);

      // example[1 - c] contains both examples and explanations
      // for (auto j : example[1 - c]) {

      // cout << example[1 - c] << endl;
      for (auto jptr{example[1 - c].rbegin()}; jptr != example[1 - c].rend();
           ++jptr) {
        auto j{*jptr};
        // cout << " " << j << endl;

        // there is already a feature of the explanation that contradicts X[j],
        // so no need to take X[j] into account
        getContradictingFeatures(X[j], implicant, contradicting_features);
        if (!contradicting_features.none()) {
          if (opt.verbosity >= Options::SOLVERINFO) {
            cout << "skip " << j << " = ";
            display_example(cout, X[j]);
            cout << " b/c it is already covered\n";
          }
          continue;
        }

        // X[j] satisfies the current explanation, so we need to add at least
        // one contradicting feature among:
        getContradictingFeatures(X[i], X[j], contradicting_features);

        if (opt.verbosity >= Options::SOLVERINFO) {
          cout << j << ": ";
          display_example(cout, X[i]);
          cout << " \\ " << setw(4) << j << ":";
          display_example(cout, X[j]);
          cout << " = ";
          display_example(cout, contradicting_features);
        }

        // this should not happen
        if (contradicting_features.none()) {
          if (opt.verbosity >= Options::SOLVERINFO)
            cout << " inconsistent example\?\?!\n";
          continue;
        }

        // check if the contradicting features intersect the current set of
        // candidates
        if (!candidates.intersects(contradicting_features)) {

          // if not, then we need to add one of the contradicted literals in the
          // explanation and start with a fresh set of candidates
          implicant.set(candidates.find_first());
          candidates.clear();
          candidates.resize(2 * numFeature(), true);
        }
        // make sure that the explanation will contradict X[j]
        candidates &= contradicting_features;

        if (opt.verbosity >= Options::SOLVERINFO) {
          cout << " -> ";
          display_example(cout, candidates);
          cout << " ";
          display_example(cout, implicant);
          cout << endl;
        }
      }

      // make sure that the explanation contradicts the last batch of examples
      // from 1-c
      implicant.set(candidates.find_first());

      if (opt.verbosity >= Options::YACKING) {
        cout << " -> add " << X.size() << ": ";
        display_example(cout, implicant);
        cout << " (" << implicant.count() << "/" << implicant.size() << ")"
             << endl
             << endl;
      }

      // add the explanation to example[c] and remove all the examples that are
      // entailed (they're into removed)
      addExplanation(implicant, c, last_example, removed);

      assert(removed.size() <= num_original[c]);
      num_original[c] -= removed.size();

      if (opt.verbosity >= Options::SOLVERINFO)
        for (auto e : removed) {
          cout << " - remove " << e << ": ";
          display_example(cout, X[e]);
          cout << endl;
        }
    }
}

std::ostream &operator<<(std::ostream &os, const DataSet &x);
}

#endif // _PRIMER_DATASET_HPP
