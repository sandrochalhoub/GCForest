
#include <iostream>
#include <map>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include "DataSet.hpp"

#ifndef _PRIMER_TYPEDDATASET_HPP
#define _PRIMER_TYPEDDATASET_HPP

using namespace boost;


typedef int dtype;
typedef dynamic_bitset<> word;

#define NUMERIC 0
#define SYMBOLIC 1

namespace primer {

// concatenate w1 and w2
word concatenate(const word &w1, const word &w2) {
  word wa{w1};
  word wb{w2};
  auto sz{w1.size() + w2.size()};
  wa.resize(sz);
  wb.resize(sz);
  wb <<= w1.size();
  wa |= wb;
  return wa;
}

/**********************************************
* TypedDataSet
**********************************************/
/// Representation of a list of examples
class TypedDataSet {

public:
  /*!@name Parameters*/
  //@{
  /// list of values
  std::vector<std::string> feature_label;
  std::vector<dtype> feature_type;
  std::vector<int> feature_rank;

  std::vector<std::vector<double>> numeric_value;
  std::vector<std::vector<std::string>> symbolic_value;

  std::vector<string> label;
  //@}

public:
  /*!@name Constructors*/
  //@{
  explicit TypedDataSet() {}
  //@}

  /*!@name Accessors*/
  //@{
  size_t size() const { return label.size(); }
  size_t numFeature() const { return feature_label.size(); }
  bool typed() const { return feature_type.size() == numFeature(); }
  template <typename RandomIt> void setFeatures(RandomIt beg, RandomIt end) {
    auto n_feature = (end - beg);
    feature_label.reserve(n_feature);
    for (auto f{beg}; f != end; ++f)
      addFeature(*f);
  }
  void addFeature(string &f) { feature_label.push_back(f); }

  template <typename RandomIt>
  void addExample(RandomIt beg, RandomIt end, string &l) {
    auto n_feature = (end - beg);
    // assert(n_feature = numFeature());
    while (n_feature > numFeature()) {
      string s("f" + to_string(numFeature() + 1));
      addFeature(s);
    }

    if (!typed())
      for (auto f{beg}; f != end; ++f) {
        typeFeature(*f);
      }

    for (auto f{beg}; f != end; ++f) {
      auto j{f - beg};
      std::stringstream convert(*f);
      switch (feature_type[j]) {
      case NUMERIC:
        double d;
        convert >> d;
        numeric_value[feature_rank[j]].push_back(d);
        break;
      case SYMBOLIC:
        symbolic_value[feature_rank[j]].push_back(*f);
        break;
      }
    }

    label.push_back(l);
  }

  // guess the type of the k-th feature, given a value v
  void typeFeature(string &s) {

    auto numeric{!s.empty() and std::find_if(s.begin(), s.end(), [](char c) {
                                  return !std::isdigit(c);
                                }) == s.end()};

    if (numeric) {
      feature_type.push_back(NUMERIC);
      feature_rank.push_back(numeric_value.size());
      numeric_value.resize(numeric_value.size() + 1);
    } else {
      feature_type.push_back(SYMBOLIC);
      feature_rank.push_back(symbolic_value.size());
      symbolic_value.resize(symbolic_value.size() + 1);
    }
  }

  template <typename T, typename RandomIt>
  void unary_equal_encode(RandomIt beg, RandomIt end, map<T, word> &encoding,
                          const bool opt) {
    vector<T> value_set(beg, end);
    sort(value_set.begin(), value_set.end());
    value_set.erase(unique(value_set.begin(), value_set.end()),
                    value_set.end());

    for (auto i{0}; i < value_set.size(); ++i) {
      dynamic_bitset<> e(value_set.size() - opt);
      if (!opt or i)
        e.set(i - opt);
      encoding[value_set[i]] = e;
    }
  }

  template <typename T, typename RandomIt>
  void unary_order_encode(RandomIt beg, RandomIt end, map<T, word> &encoding,
                          const bool opt = true) {
    vector<T> value_set(beg, end);
    sort(value_set.begin(), value_set.end());
    value_set.erase(unique(value_set.begin(), value_set.end()),
                    value_set.end());

    for (auto i{0}; i < value_set.size(); ++i) {
      dynamic_bitset<> e;
      e.resize(i, true);
      e.resize(value_set.size() - opt, false);
      encoding[value_set[i]] = e;
    }
  }

  template <typename T, typename RandomIt>
  void binary_encode(RandomIt beg, RandomIt end, map<T, word> &encoding) {
    vector<T> value_set(beg, end);
    sort(value_set.begin(), value_set.end());
    value_set.erase(unique(value_set.begin(), value_set.end()),
                    value_set.end());

    auto encoding_size{std::ceil(std::log2(value_set.size()))};

    for (long int i{0}; i < value_set.size(); ++i) {
      dynamic_bitset<> e(encoding_size, i);
      encoding[value_set[i]] = e;
    }
}

void binarize(DataSet &bin) {
  auto num_symbolic{0};
  for (auto f{0}; f < numFeature(); ++f)
    if (feature_type[f] == SYMBOLIC)
      ++num_symbolic;

  map<double, word> num_encoding[numFeature() - num_symbolic];
  map<string, word> str_encoding[num_symbolic];

  for (auto f{0}; f < numFeature(); ++f) {
    if (feature_type[f] == NUMERIC) {

      // unary_equal_encode(numeric_value[feature_rank[f]].begin(),
      // numeric_value[feature_rank[f]].end(), encoding, true);
      // unary_order_encode(numeric_value[feature_rank[f]].begin(),
      // numeric_value[feature_rank[f]].end(), encoding, true);
      binary_encode(numeric_value[feature_rank[f]].begin(),
                    numeric_value[feature_rank[f]].end(),
                    num_encoding[feature_rank[f]]);

      // for (std::map<double,word>::iterator it=num_encoding[feature_rank[f]].begin();
      // it!=num_encoding[feature_rank[f]].end(); ++it)
      //  cout << it->first << " => " << it->second << '\n';
			
    } else if (feature_type[f] == SYMBOLIC) {

      unary_equal_encode(symbolic_value[feature_rank[f]].begin(),
                         symbolic_value[feature_rank[f]].end(),
                         str_encoding[feature_rank[f]], true);

      // for (std::map<string,word>::iterator it=str_encoding[feature_rank[f]].begin();
      // it!=str_encoding[feature_rank[f]].end(); ++it)
      //  cout << it->first << " => " << it->second << '\n';
			
    }
  }

  // vector<string> label_set();
  // sort(label_set.begin(), label_set.end());
  // label_set.erase(unique(label_set.begin(), label_set.end()),
  //                 label_set.end());
  //
  // 									assert(label_set.size()
  // ==
  // 2);
	
	// for (auto f{0}; f < numFeature(); ++f)
		// bin.addFeature(feature_label[f]);
	
	auto bin_feature_count{0};
  for (auto i{0}; i < size(); ++i) {
    word binex;
    for (auto f{0}; f < numFeature(); ++f) {
      auto r{feature_rank[f]};
      if (feature_type[f] == NUMERIC) {
        binex = concatenate(binex, num_encoding[r][numeric_value[r][i]]);
      } else if (feature_type[f] == SYMBOLIC) {
        binex = concatenate(binex, str_encoding[r][symbolic_value[r][i]]);
      }
			while(bin_feature_count < binex.size()) {
				string binf{feature_label[f] + std::to_string(binex.size() - bin_feature_count++)};
				bin.addFeature(binf);
			}
    }
    // cout << binex << endl;

		// binex.resize(2 * binex.size());
		word db;		
		bin.duplicate_format(binex, db);
    bin.add(db, label[i] == label[0]);
  }
}

std::ostream &display(std::ostream &os) const {

  os << "features:";
  // for (auto l : feature_label)
  for (auto i{0}; i < numFeature(); ++i)
    os << " " << feature_label[i];
  os << endl;

  for (auto e{0}; e < label.size(); ++e) {
    for (auto f{0}; f < numFeature(); ++f) {
      dtype t{feature_type[f]};
      int r{feature_rank[f]};
      switch (t) {
      case NUMERIC:
        os << " " << numeric_value[r][e];
        break;
      case SYMBOLIC:
        os << " " << symbolic_value[r][e];
        break;
      }
    }
    os << ": " << label[e] << endl;
  }

  return os;
}
};

std::ostream &operator<<(std::ostream &os, const TypedDataSet &x) {
  return x.display(os);
}

}

#endif // _PRIMER_TYPEDDATASET_HPP
