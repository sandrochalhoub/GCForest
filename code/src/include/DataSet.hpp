
#include <iostream>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include "SparseSet.hpp"

#ifndef _MINISCHEDULER_DATASET_HPP
#define _MINISCHEDULER_DATASET_HPP

using namespace boost;

typedef dynamic_bitset<> instance;

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
    return (f < numFeature() ? "" : "¬") + to_string(f % numFeature());
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
  std::vector<instance> X;
  std::vector<int> literal_count;
  //@}

public:
  /*!@name Constructors*/
  //@{
  explicit DataSet() {}
  explicit DataSet(const int n_feature) {
    for (auto i{1}; i <= n_feature; ++i) {
      string f("f_" + std::to_string(i));
      addFeature(f);
    }
  }

  void reserve(const size_t n) {
    X.reserve(n);
    example[1].reserve(n);
    example[0].reserve(n);
  }
  //@}

  /*!@name Accessors*/
  //@{
  instance &operator[](const size_t idx) { return X[idx]; }

  template <typename RandomIt> void setFeatures(RandomIt beg, RandomIt end) {
    auto n_feature = (end - beg);
    feature_label.reserve(n_feature);
    for (auto f{beg}; f != end; ++f)
      addFeature(*f);
  }
  void addFeature(string &f) { feature_label.push_back(f); }

  template <typename RandomIt>
  void add(RandomIt beg, RandomIt end, const bool y) {
    instance x;
    x.resize(numFeature(), false);
    x.resize(2 * numFeature(), true);
    for (auto v{beg}; v != end; ++v) {
      if (*v) {
        x.set(v - beg);
        x.reset(v - beg + numFeature());
      }
    }
    add(x, y);
  }
  void add(instance &x, const bool y) {
    example[y].safe_add(X.size());
    X.push_back(x);
  }

  size_t numFeature() const { return feature_label.size(); }
  size_t size() const { return X.size(); }
  size_t count() const { return example[0].count() + example[1].count(); }
  size_t volume() const {
    auto v{0};
    for (auto c{0}; c < 2; ++c)
      for (auto e : example[c])
        v += X[e].count();
    return v;
  }

  instance NOT(instance &e) const {
    instance not_e = e;
    not_e >>= numFeature();
    not_e |= (e << numFeature());
    return not_e;
  }

  double entropy(const int feature) {
    auto not_feature = (feature + numFeature());

    int npos[2] = {0, 0};
    int nneg[2] = {0, 0};
    int* count[2] = {nneg, npos};
    for (auto c{0}; c < 2; ++c) {
			int gr0{0};
			int gr1{0};
			
      for (auto e : example[c]) {
				
				// cout << X[e] << endl;
				// for(auto f=0; f<numFeature(); ++f)
				// 	cout << X[e][f] ;
				// cout << endl;
				// exit(1);
				
				
				
        if (X[e][feature] != X[e][not_feature]) {
          ++count[c][X[e][feature]];
					if(X[e][not_feature])
						gr1++;
					else
						gr0++;
				}
      }
			cout << (c ? " positive: " : " negative: ") << gr0 << "/" << gr1 << endl;
		}
		
		for (auto v{0}; v < 2; ++v)	
			for (auto c{0}; c < 2; ++c)
				cout << feature << "=" << v << ": " << count[c][v] << (c ? " positive" : " negative") << " examples\n";
			
			return 0;
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

  /*!@name Business*/
  //@{
  // look for examples that are both positive and negative and remove them
  void filter() {
		vector<int> to_remove;
    for (auto neg : example[0])
      for (auto pos : example[1])
        if (X[neg] == X[pos]) {
					to_remove.push_back(neg);
					to_remove.push_back(pos);
        }
		for(auto e{to_remove.begin()}; e!=to_remove.end(); e+=2) {
			for(int i{0}; i<2; ++i)
				example[i].remove_back(*(e+i));
		}
  }

  // the set of true features of x1 that are false in x2
  // we cannot do set difference because features might neighter be true nor
  // false
  void getContradictingFeatures(instance &x1, instance &x2, instance &buffer) {
    buffer = NOT(x2);
    // buffer = x2;
    // buffer >>= nFeature() / 2;
    // buffer |= (x2 << numFeature() / 2);

    buffer &= x1;
  }

  void addExplanation(instance &impl, const bool y, const int limit,
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

	template< typename R >
  void computeDecisionSet(Options& opt, R& random_generator) {
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
        cout << " (" << implicant.count() << ")" << endl << endl;
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

  void close() {
    literal_count.reserve(size());
    for (auto c{0}; c < 2; ++c)
      for (auto e : example[c])
        literal_count[e] = X[e].count();
  }

  bool classify(instance &x) {
    for (auto c{0}; c < 2; ++c)
      for (auto e : example[c])
        if (X[e].is_subset_of(x))
          return c;

    instance buffer;
    double proba[2];

    // #contradictions / #number literal = probability that x is not of class c

    for (auto c{0}; c < 2; ++c)
      for (auto e : example[c]) {
        buffer = X[e];
        buffer -= x;
        auto nl{buffer.count()};
        proba[c] *=
            (static_cast<double>(nl) / static_cast<double>(literal_count[e]));
      }

    return proba[1] > proba[0];
  }
  //@}

  /*!@name Miscellaneous*/
  //@{
  std::ostream &display(std::ostream &os) const {

    os << "features:";
    // for (auto l : feature_label)
    for (auto i{0}; i < numFeature(); ++i)
      os << " " << feature_label[i];
    os << endl;

    // for (auto i : example[1])
    //   os << X[i] << ": +" << std::endl;
    // for (auto i : example[0])
    //   os << X[i] << ": -" << std::endl;

    os << "POSITIVE EXAMPLES:\n";
    for (auto i : example[1]) {
      cout << i << ": "; //<< X[i] << " ";
      display_example(os, X[i]);
      os << endl;
    }
    os << "NEGATIVE EXAMPLES:\n";
    for (auto i : example[0]) {
      cout << i << ": "; //<< X[i] << " ";
      display_example(os, X[i]);
      os << endl;
    }
    return os;
  }

  std::ostream &display_example(std::ostream &os, const instance e) const {

    auto flag{false};
    auto nxt{false};
    int last;
    size_t n{e.count()};

    os << "(";
    for (auto f{0}; f < 2 * numFeature(); ++f) {
      if (e[f]) {

        // os << "[" << f << "]";

        if (!--n) {
          if (flag)
            os << ",";
          os << pretty(f) << ")"; // feature_label[f];
          break;
        }

        if (f < 2 * numFeature() - 1 and e[f + 1])
          nxt = true;
        else
          nxt = false;

        if (!nxt or f != last + 1 or f == numFeature()) {
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
  //@}

  /*!@name Verification*/
  //@{
  void verify() {
    // check that every example is entailed by an explanation of its class and
    // that no explanation entails an example of the other class

    instance not_covered[2];
    for (auto c{0}; c < 2; ++c) {
      not_covered[c].resize(X.size(), false);
      for (auto i{-example[c].start()}; i < 0; ++i)
        not_covered[c].set(example[c][i]);
    }

    instance contradictions, not_expl;
    for (auto c{0}; c < 2; ++c)
      for (auto explanation : example[c]) {
        not_expl = NOT(X[explanation]);

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

        for (auto counter_example : example[1 - c]) {
          contradictions = (not_expl & X[counter_example]);
          if (contradictions.none()) {
            cerr << explanation << ": ";
            display_example(cerr, X[explanation]);
            cerr << " does not contradict " << counter_example << ": ";
            display_example(cerr, X[counter_example]);
            cerr << endl;
            exit(1);
          }
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
