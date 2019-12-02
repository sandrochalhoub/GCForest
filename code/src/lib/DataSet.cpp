
#include <cmath>

#include <boost/range/adaptor/reversed.hpp>

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

void DataSet::duplicate_format(const instance& from, instance& to) const {
	
	if(from.size() != numFeature()) {
		cout << from.size() << " != " << numFeature() << endl;
	}
	
	assert(from.size() == numFeature());
	
	to.clear();
	to.resize(numFeature());
	to |= from;
	to.resize(2*numFeature(), true);
	auto aux{to};
	aux <<= numFeature();
	aux.flip();
	to &= aux;
}

instance &DataSet::operator[](const size_t idx) { return X[idx]; }

void DataSet::addFeature(string &f) { feature_label.push_back(f); }

void DataSet::add(instance& x, const bool y) {

  example[y].safe_add(X.size());
  X.push_back(x);
}

size_t DataSet::numFeature() const { return feature_label.size(); }
size_t DataSet::size() const { return X.size(); }
size_t DataSet::positiveCount() const { return example[1].count(); }
size_t DataSet::negativeCount() const { return example[0].count(); }
size_t DataSet::count() const { return positiveCount() + negativeCount(); }
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

void DataSet::computeProbabilities() {
  double very_low{log(1.0 / static_cast<double>(1000 * size()))};

  // if(feature_count[0].empty() or feature_count[1].empty())
  computeEntropies();

	
  for (auto y{0}; y < 2; ++y) {
		
		// cout << "class " << y << endl;
		
		
    feature_probability[y].resize(2 * numFeature());
    auto total{static_cast<double>(example[y].count())};

    for (auto f{0}; f < 2 * numFeature(); ++f) {
			
			// cout << " -feature " << pretty(f) << ": (" << feature_count[y][f] << "/" << total << ") = "  ;

      if (feature_count[y][f] == 0) {
        feature_probability[y][f] = very_low;
				// cout << "0\n";
      } else {
        feature_probability[y][f] =
            log(static_cast<double>(feature_count[y][f]) / total);
				// cout << (static_cast<double>(feature_count[y][f]) / total) << endl;
			}
			
    }

    // cout << y << ":";
    // for (auto f{0}; f < numFeature(); ++f)
    //   cout << " " << setw(5) << setprecision(3)
    //        << (static_cast<double>(feature_count[y][f]) / total);
    // cout << endl << "  ";
    // for (auto f{numFeature()}; f < 2 * numFeature(); ++f)
    //   cout << " " << setw(5) << setprecision(3)
    //        << (static_cast<double>(feature_count[y][f]) / total);
    // cout << endl;
    //
    // cout << y << ":";
    // for (auto f{0}; f < numFeature(); ++f)
    //   cout << " " << setw(5) << setprecision(3) << feature_probability[y][f];
    // cout << endl << "  ";
    // for (auto f{numFeature()}; f < 2 * numFeature(); ++f)
    //   cout << " " << setw(5) << setprecision(3) << feature_probability[y][f];
    // cout << endl;
  }

  example_probability.resize(size());
  for (auto y{0}; y < 2; ++y)
    for (auto x : example[y]) {
      double proba_x{0};
      for (auto f{0}; f < numFeature(); ++f) {
        auto not_f{f + numFeature()};
        if (X[x][f] == X[x][not_f])
          --proba_x; //
        else if (X[x][f])
          proba_x += feature_probability[y][f];
        else
          proba_x += feature_probability[y][not_f];
      }

      example_probability[x] = proba_x;
    }
}

int DataSet::argMinEntropy(instance &candidate) const {
  for (auto f : feature_order)
    if (candidate[f])
      return f;
    else if (candidate[f + numFeature()])
      return f + numFeature();
  assert(false);
}

int DataSet::argMaxEntropy(instance &candidate) const {
  for (auto f : boost::adaptors::reverse(feature_order))
    if (candidate[f])
      return f;
    else if (candidate[f + numFeature()])
      return f + numFeature();
  assert(false);
}

// int DataSet::argMinProbability(SparseSet &candidate, const int limit) const {
//   return *std::min_element(
//       candidate.begin(), candidate.begin() + limit, [&](const int a, const int b) {
//         return example_probability[a] < example_probability[b];
//       });
// }
//
// int DataSet::argMaxProbability(SparseSet &candidate, const int limit) const {
//   return *std::min_element(
//       candidate.begin(), candidate.begin() + limit, [&](const int a, const int b) {
//         return example_probability[a] > example_probability[b];
//       });
// }

void DataSet::computeEntropies() {

  feature_count[0].resize(2 * numFeature());
  feature_count[1].resize(2 * numFeature());

  entropy_rank.resize(numFeature());
  entropy_value.resize(numFeature());
  feature_order.clear();
  for (auto f{0}; f < numFeature(); ++f) {
    entropy_value[f] = entropy(f);
    feature_order.push_back(f);
  }

  sort(feature_order.begin(), feature_order.end(),
       [=](const int f1, const int f2) {
         return entropy_value[f1] < entropy_value[f2];
       });

  int r{0};
  for (auto f : feature_order) {
    entropy_rank[f] = r++;

    // cout << setw(3) << f << ": " << setw(10) << setprecision(5) <<
    // entropy_value[f] << endl;
  }
}

double DataSet::entropy(const int feature) {

  double feature_entropy{0};

  int not_feature = (feature + numFeature());
  int truef[2] = {not_feature, feature};

  for (auto y{0}; y < 2; ++y) {
    feature_count[y][feature] = 0;
    feature_count[y][not_feature] = 0;
  }

  for (auto y{0}; y < 2; ++y)
    for (auto e : example[y]) {
      if (X[e][feature] != X[e][not_feature])
        ++feature_count[y][truef[X[e][feature]]];
      else {
      	// irrelevant ~= equiprobable
        feature_count[y][feature] += .5;
				feature_count[y][not_feature] += .5;
			}
    }

  // double entropy{0};
  double total_size{
      static_cast<double>(example[0].count() + example[1].size())};

  for (auto x{0}; x < 2; ++x) {

    // double val_size{static_cast<double>(feature_count[0][truef[x]] +
    //                                     feature_count[1][truef[x]])};

    double val_size{feature_count[0][truef[x]] + feature_count[1][truef[x]]};

    // Pr(Y=y|X=x) = (count[y][x] / val_size)

    // H(Y|X=x) = \sum_y (Pr(Y=y|X=x) log2 Pr(Y=y|X=x))
    double entropy_x{0};
    for (auto y{0}; y < 2; ++y) {
      if (feature_count[y][truef[x]] != 0 and
          feature_count[y][truef[x]] != val_size) {
        entropy_x -= (feature_count[y][truef[x]] / val_size) *
                     std::log2(feature_count[y][truef[x]] / val_size);
        // cout << " + " << -(count[c][val] / val_size) *
        // std::log(count[c][val] / val_size);
      }
      // else cout << " + 0";
    }

    // cout << " = " << (entropy_val * val_size / total_size) << endl;

    // Pr(X=x) = val_size / total_size

    // H(Y|X) = \sum_x Pr(X=x) H(Y|X=x)
    feature_entropy += (entropy_x * val_size / total_size);
  }

  return feature_entropy;
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

std::ostream &DataSet::write(std::ostream &os, string &delimiter,
                             string &wildcard, const bool matrix,
                             const bool header) const {

  SparseSet relevant_feature(numFeature());
  relevant_feature.fill();

  for (auto i{0}; i < numFeature(); ++i) {
    auto irrelevant{true};
    for (auto c{0}; irrelevant and c < 2; ++c)
      for (auto j : example[c])
        if (X[j][i] != X[j][i + numFeature()]) {
          irrelevant = false;
          break;
        }
    if (irrelevant)
      relevant_feature.remove_back(i);
  }

  if (header) {
    for (auto i : relevant_feature)
      os << i << delimiter << " ";
    os << "label" << endl;
  }

  for (auto c{0}; c < 2; ++c)
    for (auto j : example[c]) {
      for (auto i : relevant_feature) {
        if (X[j][i] == X[j][i + numFeature()]) {
          if (matrix)
            os << "*" << delimiter << " ";
        } else {
          if (matrix)
            os << X[j][i] << delimiter << " ";
          else
            os << (X[j][i] ? (i + 1) : -(i + 1)) << delimiter << " ";
        }
      }
      os << c << endl;
    }

  return os;
}

// std::ostream &DataSet::toCsv(std::ostream &os) const {
//   for (auto i{0}; i < numFeature(); ++i)
//     os << feature_label[i] << ",";
//   os << "label" << endl;
//   for (auto c{0}; c < 2; ++c)
//     for (auto j : example[c]) {
//       for (auto i{0}; i < numFeature(); ++i)
//         os << X[j][i] << ",";
//       os << c << endl;
//     }
//
//   return os;
// }

// std::ostream &DataSet::toTxt(std::ostream &os) const {
//   //     for (auto i{0}; i < numFeature(); ++i)
//   //       os << feature_label[i] << " " ;
//   // os << "label" << endl;
//
//   SparseSet relevant_feature(numFeature());
//   relevant_feature.fill();
//
//   for (auto i{0}; i < numFeature(); ++i) {
//     auto irrelevant{true};
//     for (auto c{0}; irrelevant and c < 2; ++c)
//       for (auto j : example[c])
//         if (X[j][i] != X[j][i + numFeature()]) {
//           irrelevant = false;
//           break;
//         }
//     if (irrelevant)
//       relevant_feature.remove_back(i);
//   }
//
//   for (auto c{0}; c < 2; ++c)
//     for (auto j : example[c]) {
//       for (auto i : relevant_feature)
//         if (X[j][i] == X[j][i + numFeature()])
//           os << "* ";
//         else
//           os << X[j][i] << " ";
//       os << c << endl;
//     }
//
//   return os;
// }

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
    displayExample(os, X[i]);
    if (example_probability.size() > i)
      os << " e^" << example_probability[i];
    os << endl;
  }
  os << "NEGATIVE EXAMPLES:\n";
  for (auto i : example[0]) {
    cout << i << ": "; //<< X[i] << " ";
    displayExample(os, X[i]);
    if (example_probability.size() > i)
      os << " e^" << example_probability[i];
    os << endl;
  }
  return os;
}

std::ostream &DataSet::displayExample(std::ostream &os,
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
          displayExample(cerr, X[explanation]);
          cerr << " for class " << c << " entails example " << example[c][i]
               << ": " << X[example[c][i]] << " = ";
          displayExample(cerr, X[example[c][i]]);
          cerr << " of class " << (1 - c) << "!!\n";
          exit(1);
        }

      for (auto counter_example : example[1 - c]) {
        contradictions = (not_expl & X[counter_example]);
        if (contradictions.none()) {
          cerr << explanation << ": ";
          displayExample(cerr, X[explanation]);
          cerr << " does not contradict " << counter_example << ": ";
          displayExample(cerr, X[counter_example]);
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

std::ostream &operator<<(std::ostream &os, const DataSet &x) {
  return x.display(os);
}
}