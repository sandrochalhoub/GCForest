
#include <iostream>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include "SparseSet.hpp"

#ifndef _MINISCHEDULER_DATASET_HPP
#define _MINISCHEDULER_DATASET_HPP

using namespace boost;

namespace primer {
/**********************************************
* DataSet
**********************************************/
/// Representation of a list of examples
class DataSet {

  // private:
  //   dynamic_bitset<> positive_mask;
  //   dynamic_bitset<> negative_mask;

private:
  string pretty(const int f) const {
    return (f < (numFeature() / 2) ? "" : "¬") +
           to_string(f % (numFeature() / 2));
  }

public:
  /*!@name Parameters*/
  //@{
  /// list of values
  std::vector<std::string> feature_label;

  // we store the example followed by its negation
  // irrelevant bits are 0 in both copies
  SparseSet example[2];
  SparseSet explanations[2];
  std::vector<dynamic_bitset<>> X;
  //@}

public:
  /*!@name Constructors*/
  //@{
  explicit DataSet() {}
  explicit DataSet(const int nfeatures) {
    for (auto i{1}; i <= nfeatures; ++i)
      feature_label.push_back("f_" + std::to_string(i));
  }
  void reserve(const size_t n) {
    X.reserve(n);
    example[1].reserve(n);
    example[0].reserve(n);
  }

  /*!@name Accessors*/
  //@{
  dynamic_bitset<> &operator[](const size_t idx) { return X[idx]; }
  void addFeature(string &f) { feature_label.push_back(f); }
  void add(dynamic_bitset<> &x, const bool y) {
    example[y].safe_add(X.size());
    X.push_back(x);
  }
  size_t numFeature() const { return feature_label.size(); }
  size_t size() const { return X.size(); }
  size_t count() const { return example[0].count() + example[1].count(); }
  int NOT(const int f) const { return (f + numFeature() / 2) % numFeature(); }
  dynamic_bitset<> NOT(dynamic_bitset<> &e) const {
    dynamic_bitset<> not_e = e;
    not_e >>= numFeature() / 2;
    not_e |= (e << numFeature() / 2);
    return not_e;
  }
  //@}

  /*!@name List Manipulation*/
  //@{
  // keeps only n positive examples, chosen randomly with a uniform distribution
  template <class random>
  void uniform_sample(const int c, const size_t n, random generator) {
    while (example[c].count() > n) {
      example[c].remove_back(example[c][generator() % example[c].count()]);
    }
  }
  //@}

  /*!@name Miscellaneous*/
  //@{
  std::ostream &display(std::ostream &os) const {

    os << "features:";
    // for (auto l : feature_label)
    for (auto i{0}; i < numFeature() / 2; ++i)
      os << " " << feature_label[i];
    os << endl;

    // for (auto i : example[1])
    //   os << X[i] << ": +" << std::endl;
    // for (auto i : example[0])
    //   os << X[i] << ": -" << std::endl;

    os << "POSITIVE EXAMPLES:\n";
    for (auto i : example[1]) {
      cout << i << ": " ;//<< X[i] << " ";
      display_example(os, X[i]);
      os << endl;
    }
    os << "NEGATIVE EXAMPLES:\n";
    for (auto i : example[0]) {
      cout << i << ": " ;//<< X[i] << " ";
      display_example(os, X[i]);
      os << endl;
    }
    return os;
  }

  std::ostream &display_example(std::ostream &os,
                                const dynamic_bitset<> e) const {

    auto flag{false};
    auto nxt{false};
    int last;
    size_t n{e.count()};
    for (auto f{0}; f < numFeature(); ++f) {
      if (e[f]) {
        if (f < numFeature() - 1 and e[f])
          nxt = true;

        if (!--n) {
          if (flag)
            os << ",";
          os << pretty(f); // feature_label[f];
          break;
        }

        if (!nxt or f != last + 1 or f == numFeature()/2) {
          if (flag)
            os << ",";
          os << pretty(f); // feature_label[f];
          flag = true;
        } else if (flag) {
          os << "..";
          flag = false;
        }

        last = f;
      }
    }

    return os;
  }

  // the set of true features of x1 that are false in x2
  // we cannot do set difference because features might neighter be true nor
  // false
  void getContradictingFeatures(dynamic_bitset<> &x1, dynamic_bitset<> &x2,
                                dynamic_bitset<> &buffer) {
    buffer = NOT(x2);
    // buffer = x2;
    // buffer >>= nFeature() / 2;
    // buffer |= (x2 << numFeature() / 2);

    buffer &= x1;
  }

  void addExplanation(dynamic_bitset<> &impl, const bool y, const int limit,
                      vector<int> &entailed) {
    entailed.clear();
    // remove all explained examples
    for (auto e : example[y]) {
      if (e > limit)
        break;

      if (impl.is_subset_of(X[e]))
        entailed.push_back(e);
    }
    for (auto e : entailed)
      example[y].remove_front(e);

    // add the explanation instead
    add(impl, y);
  }

  void computeDecisionSet(Options opt) {
    auto c{0};

    dynamic_bitset<> contradicting_features;
    dynamic_bitset<> candidates;
    dynamic_bitset<> implicant;

    vector<int> removed;

    contradicting_features.resize(numFeature());

    auto last_example{X.size() - 1};

    while (true) {

      // alternate between positive and negative examples, unless there are no
      // remaining example of one class
      c = 1 - c;
      auto i{example[c].front()};
      if (i > last_example) {
        c = 1 - c;
        i = example[c].front();
        if (i > last_example)
          break;
      }

      // now X[i] is the first remaining example of class c
      if (opt.verbosity >= Options::SOLVERINFO) {
        cout << "compute a rule from the " << (c ? "positive" : "negative")
             << " example " << i << ":";
        display_example(cout, X[i]);
        cout << endl;
      }

      // implicant is empty
      implicant.clear();
      implicant.resize(numFeature(), false);

      // cnadidates contains all features
      candidates.clear();
      candidates.resize(numFeature(), true);

      // example[1 - c] contains both examples and explanations
      for (auto j : example[1 - c]) {

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
          candidates.resize(numFeature(), true);
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
        cout << " -> ";
        display_example(cout, implicant);
        cout << " (" << implicant.count() << ")" << endl << endl;
      }

      // add the explanation to example[c] and remove all the examples that are
      // entailed (they're into removed)
      addExplanation(implicant, c, last_example, removed);

      if (opt.verbosity >= Options::SOLVERINFO)
        for (auto e : removed) {
          cout << " - remove ";
          display_example(cout, X[e]);
          cout << endl;
        }
    }
  }

  void verify() {
    // check that every example is entailed by an explanation of its class and
    // that no explanation entails an example of the other class

    dynamic_bitset<> not_covered[2];
    for (auto c{0}; c < 2; ++c) {
      not_covered[c].resize(X.size(), false);
      for (auto i{-example[c].start()}; i < 0; ++i)
        not_covered[c].set(example[c][i]);
    }

    for (auto c{0}; c < 2; ++c)
      for (auto explanation : example[c]) {

        for (auto i{-example[c].start()}; i < 0; ++i)
          if (X[explanation].is_subset_of(X[example[c][i]]))
            not_covered[c].reset(example[c][i]);

        for (auto i{-example[1 - c].start()}; i < 0; ++i)
          if (X[explanation].is_subset_of(X[example[c][i]])) {
            cerr << "rule " << explanation << ": " << X[explanation] << " = ";
            display_example(cerr, X[explanation]);
            cerr << " for class " << c << " entails example " << example[c][i]
                 << ": " << X[example[c][i]] << " = ";
            display_example(cerr, X[example[c][i]]);
            cerr << " of class " << (1 - c) << "!!\n";
            exit(1);
          }
      }

    for (auto c{0}; c < 2; ++c) {
      if (!not_covered[c].none()) {
        cerr << "examples " << not_covered[c] << " are covered by no rule!!\n";
        exit(1);
      }
    }
  }
};

std::ostream &operator<<(std::ostream &os, const DataSet &x) {
  return x.display(os);
}

}

#endif // _MINISCHEDULER_DATASET_HPP
