

#include "DataSet.hpp"



using namespace boost;

namespace primer {

std::string DataSet::pretty(const int f) const {
  return (f < numFeature() ? "" : "¬") + to_string(f % numFeature());
}

DataSet::DataSet() {}

DataSet::DataSet(const int n_feature) {
  for (auto i{1}; i <= n_feature; ++i) {
    string f("f_" + std::to_string(i));
    addFeature(f);
  }
}

void DataSet::reserve(const size_t n) {
  X.reserve(n);
  example[1].reserve(n);
  example[0].reserve(n);
}

instance &DataSet::operator[](const size_t idx) { return X[idx]; }

void DataSet::addFeature(string &f) { feature_label.push_back(f); }

// I AM NOT SURE WHAT WAS THE PURPOSE OF THAT
// void DataSet::add(instance x_, const bool y) {
//
//   while (x_.size() > numFeature()) {
//     string s("f" + to_string(numFeature() + 1));
//     addFeature(s);
//   }
//
//   instance x = x_;
//   instance xp = x_;
//
//   x.resize(2 * numFeature(), true);
//
//   xp.resize(2 * numFeature(), false);
//   xp <<= numFeature();
//
//   x.flip();
//
//   x |= xp;
//
//   example[y].safe_add(X.size());
//   X.push_back(x);
// }

void DataSet::add(instance x, const bool y) {

  example[y].safe_add(X.size());
  X.push_back(x);
}

size_t DataSet::numFeature() const { return feature_label.size(); }
size_t DataSet::size() const { return X.size(); }
size_t DataSet::positive_count() const { return example[1].count(); }
size_t DataSet::negative_count() const { return example[0].count(); }
size_t DataSet::count() const { return positive_count() + negative_count(); }
size_t DataSet::volume() const {
  auto v{0};
  for (auto c{0}; c < 2; ++c)
    for (auto e : example[c])
      v += X[e].count();
  return v;
}

instance DataSet::NOT(instance &e) const {
  instance not_e = e;
  not_e >>= numFeature();
  not_e |= (e << numFeature());
  return not_e;
}

double DataSet::conditional_entropy(const int feature) {

  auto not_feature = (feature + numFeature());

  // cout << pretty(feature) << " / " << pretty(not_feature) << endl;

  double npos[2] = {0, 0};
  double nneg[2] = {0, 0};
  double *count[2] = {nneg, npos};
  for (auto c{0}; c < 2; ++c)
    for (auto e : example[c])
      if (X[e][feature] != X[e][not_feature])
        ++count[c][X[e][feature]];

  // for (auto v{0}; v < 2; ++v)
  // 	for (auto c{0}; c < 2; ++c)
  // 		cout << feature << "=" << v << ": " << count[c][v] << (c ? "
  // positive"
  // : " negative") << " examples\n";

  double entropy{0};
  double total_size{
      static_cast<double>(example[0].count() + example[1].size())};

  for (auto val{0}; val < 2; ++val) {
    //
    // cout << "compute entropy of dataset w.r.t. " << pretty((val ? feature :
    // not_feature)) << " (" << count[0][val] << "/" << count[1][val] <<
    // "):\n";

    auto val_size{count[0][val] + count[1][val]};
    // auto ratio{val_size / total_size};
    double entropy_val{0};
    for (auto c{0}; c < 2; ++c) {
      if (count[c][val] != 0 and count[c][val] != val_size) {
        entropy_val -=
            (count[c][val] / val_size) * std::log(count[c][val] / val_size);
        // cout << " + " << -(count[c][val] / val_size) *
        // std::log(count[c][val] / val_size);
      }
      // else cout << " + 0";
    }

    // cout << " = " << (entropy_val * val_size / total_size) << endl;

    entropy += (entropy_val * val_size / total_size);
  }

  // cout << " ==> " << -entropy << endl;

  return entropy;
}

void DataSet::filter() {
  vector<int> to_remove;
  for (auto neg : example[0])
    for (auto pos : example[1])
      if (X[neg] == X[pos]) {
        to_remove.push_back(neg);
        to_remove.push_back(pos);
      }
  for (auto e{to_remove.begin()}; e != to_remove.end(); e += 2) {
    for (int i{0}; i < 2; ++i)
      example[i].remove_back(*(e + i));
  }
}

void DataSet::getContradictingFeatures(instance &x1, instance &x2,
                                       instance &buffer) {
  buffer = NOT(x2);
  // buffer = x2;
  // buffer >>= nFeature() / 2;
  // buffer |= (x2 << numFeature() / 2);

  buffer &= x1;
}

void DataSet::addExplanation(instance &impl, const bool y, const int limit,
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

void DataSet::close() {
  literal_count.reserve(size());
  for (auto c{0}; c < 2; ++c)
    for (auto e : example[c])
      literal_count[e] = X[e].count();
}

bool DataSet::classify(instance &x) {
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

std::ostream &DataSet::to_csv(std::ostream &os) const {
  for (auto i{0}; i < numFeature(); ++i)
    os << feature_label[i] << ",";
  os << "label" << endl;
  for (auto c{0}; c < 2; ++c)
    for (auto j : example[c]) {
      for (auto i{0}; i < numFeature(); ++i)
        os << X[j][i] << ",";
      os << c << endl;
    }

  return os;
}

std::ostream &DataSet::to_txt(std::ostream &os) const {
  //     for (auto i{0}; i < numFeature(); ++i)
  //       os << feature_label[i] << " " ;
  // os << "label" << endl;
  for (auto c{0}; c < 2; ++c)
    for (auto j : example[c]) {
      for (auto i{0}; i < numFeature(); ++i)
        os << X[j][i] << " ";
      os << c << endl;
    }

  return os;
}

std::ostream &DataSet::display(std::ostream &os) const {

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

std::ostream &DataSet::display_example(std::ostream &os,
                                       const instance e) const {

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

void DataSet::verify() {
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

std::ostream& operator<<(std::ostream &os, const DataSet &x) {
  return x.display(os);
}

}