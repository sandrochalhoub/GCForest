
#include <iostream>
#include <map>
#include <vector>

#include "typedef.hpp"
#include <boost/algorithm/string/trim.hpp>
// #include <boost/dynamic_bitset.hpp>
#include <boost/lexical_cast.hpp>

// #include "WeightedDataset.hpp"

#ifndef _PRIMER_TYPEDDATASET_HPP
#define _PRIMER_TYPEDDATASET_HPP

using namespace boost;
using namespace std;

typedef int dtype;
// typedef dynamic_bitset<> word;

#define INTEGER 0
#define FLOAT 1
#define SYMBOL 2

static const instance nothing{instance()};

namespace blossom {

template <typename T> bool is(const std::string &someString) {
  using boost::lexical_cast;
  using boost::bad_lexical_cast;

  try {
    boost::lexical_cast<T>(someString);
  } catch (bad_lexical_cast &) {
    return false;
  }

  return true;
}

// concatenate w1 and w2
instance concatenate(const instance &w1, const instance &w2) {
  instance wa{w1};
  instance wb{w2};
  auto sz{w1.size() + w2.size()};
  wa.resize(sz);
  wb.resize(sz);
  wb <<= w1.size();
  wa |= wb;
  return wa;
}

// encoding from type T to binary instances
template <typename T> class Encoding {

public:
  virtual ~Encoding() {}

  // encode the values of the iterator
  virtual void encode(typename std::vector<T>::iterator beg,
                      typename std::vector<T>::iterator end) = 0;

  // returns the instance associated to value x
  virtual const instance &getEncoding(T &x) const = 0;

  virtual size_t size() const = 0;

  virtual const string getType() const = 0;

  // returns (in string format) the test corresponding to x[i]==v
  virtual const string getLabel(const int i, const int v) const = 0;
  virtual const string getLabel(const int i) const {
    return this->getLabel(i, 1);
  }

  //
  // const bool test()
};

// encoding from type T to binary instances
template <typename T> class TrivialEncoding : public Encoding<T> {

protected:
  vector<T> value_set;
  vector<instance> lit;

public:
  // virtual ~TrivialEncoding() {}

  virtual size_t size() const { return 1; }

  // encode the values of the iterator
  virtual void encode(typename std::vector<T>::iterator beg,
                      typename std::vector<T>::iterator end) {
    assert(end - beg == 2);
    for (auto it{beg}; it != end; ++it)
      value_set.push_back(*it);
    lit.push_back(dynamic_bitset<>(1, false));
    lit.push_back(dynamic_bitset<>(1, true));
  }

  // returns the instance associated to value x
  virtual const instance &getEncoding(T &x) const {
    return lit[(x == value_set[1])];
  }

  virtual const string getType() const {
    std::stringstream ss;
    ss << "trivial " << value_set[0] << " " << value_set[1];
    return ss.str();
  }

  // returns (in string format) the test x[i]
  virtual const string getLabel(const int i, const int v) const {
    assert(i == 0);
    std::stringstream ss;
    ss << "=" << value_set[v];
    return ss.str();
  }

  virtual const string getLabel(const int i) const { return ""; }

  //
  // const bool test()
};

// encoding from type T to binary instances
template <typename T> class ClassicEncoding : public Encoding<T> {

protected:
  map<T, instance> encoding_map;
  vector<T> value_set;

public:
  // virtual ~ClassicEncoding() {}

  // encode the values of the iterator
  // template <typename RandomIt>
  virtual void encode(typename std::vector<T>::iterator beg,
                      typename std::vector<T>::iterator end) {
    for (auto it{beg}; it != end; ++it)
      value_set.push_back(*it);
  }

  // returns the instance associated to value x
  const instance &getEncoding(T &x) const {
    auto it = encoding_map.find(x);
    if (it != encoding_map.end()) {
      return it->second;
    }
    return nothing;
  }
};

template <typename T> class BinaryDirect : public ClassicEncoding<T> {

public:
  bool is_signed;
  size_t encoding_size;

  virtual size_t size() const { return encoding_size; }

  // encode the values of the iterator
  // template <typename RandomIt>
  void encode(typename std::vector<T>::iterator beg,
              typename std::vector<T>::iterator end) {

    ClassicEncoding<T>::encode(beg, end);

    auto vb{ClassicEncoding<T>::value_set.begin()};
    auto ve{ClassicEncoding<T>::value_set.end()};
    auto minv{*vb};
    auto maxv{*(ve - 1)};

    is_signed = (minv < 0 && maxv > 0);

    encoding_size = is_signed + max((minv < 0 ? ceil(log2(abs(minv) + 1)) : 0),
                                    (maxv > 0 ? ceil(log2(maxv + 1)) : 0));

    for (auto i{vb}; i != ve; ++i) {
      dynamic_bitset<> e;
      if (is_signed)
        e.resize(encoding_size, abs(*i * 2) + (minv < 0));
      else
        e.resize(encoding_size, *i);
      ClassicEncoding<T>::encoding_map[*i] = e;
    }
  }

  virtual const string getType() const { return "binary-direct"; }

  // returns (in string format) the test x[i]
  const string getLabel(const int i, const int v) const {
    if (i == 0)
      return "<0";
    std::stringstream ss;
    ss << "&2^" << i << (v ? "!=0" : "=0");
    return ss.str();
  }
};

template <typename T> class BinaryScaled : public ClassicEncoding<T> {

public:
  size_t encoding_size;

  virtual size_t size() const { return encoding_size; }

  // encode the values of the iterator
  // template <typename RandomIt>
  void encode(typename std::vector<T>::iterator beg,
              typename std::vector<T>::iterator end) {

    ClassicEncoding<T>::encode(beg, end);

    auto vb{ClassicEncoding<T>::value_set.begin()};
    auto ve{ClassicEncoding<T>::value_set.end()};
    auto minv{*vb};
    auto maxv{*(ve - 1)};

    encoding_size = ceil(log2(vb - ve));

    for (auto i{vb}; i != ve; ++i) {
      dynamic_bitset<> e;
      e.resize(encoding_size, i - vb);
      ClassicEncoding<T>::encoding_map[*i] = e;
    }
  }

  virtual const string getType() const { return "binary-scaled"; }

  // returns (in string format) the test x[i]
  const string getLabel(const int i, const int v) const { return "&=?"; }
};

template <typename T> class Order : public ClassicEncoding<T> {

private:
  size_t num_examples;
  const string &feature_name;

public:
  Order(const size_t n, const string &f)
      : ClassicEncoding<T>(), num_examples(n), feature_name(f) {}

  virtual size_t size() const {
    return ClassicEncoding<T>::value_set.size() - 1;
  }

  // encode the values of the iterator
  // template <typename RandomIt>
  void encode(typename std::vector<T>::iterator beg,
              typename std::vector<T>::iterator end) {

    ClassicEncoding<T>::encode(beg, end);

    // cout << "unary order encode\n";
    // for (auto v : ClassicEncoding<T>::value_set)
    //   cout << " " << v;
    // cout << endl;

    // cout << ClassicEncoding<T>::value_set.size() << " " << (num_examples) ;

    if (ClassicEncoding<T>::value_set.size() < sqrt(num_examples) or ClassicEncoding<T>::value_set.size() < 10) {
      // cout << " full\n";

      full_encoding();
    } else {

      size_t num_intervals{static_cast<size_t>(
          log(static_cast<double>(ClassicEncoding<T>::value_set.size())))};
      if (num_intervals < 1)
        num_intervals = 1;

      cout << "c possible precision loss when binarizing feature "
           << feature_name << " (" << ClassicEncoding<T>::value_set.size()
           << " distinct values -> " << num_intervals << " intervals)"
           << "\n";

      reduced_encoding(num_intervals);
    }
  }

  void full_encoding() {

    auto vb{ClassicEncoding<T>::value_set.begin()};
    auto ve{ClassicEncoding<T>::value_set.end() - 1};

    for (auto i{vb}; i <= ve; ++i) {
      dynamic_bitset<> e;
      e.resize(i - vb, false);
      e.resize(ve - vb, true);
      ClassicEncoding<T>::encoding_map[*i] = e;
    }
  }

  void reduced_encoding(const size_t num_intervals) {

    vector<size_t> boundary;
    size_t i_size{ClassicEncoding<T>::value_set.size() / num_intervals};
    size_t next_boundary{i_size};
    while (next_boundary < ClassicEncoding<T>::value_set.size()) {
      boundary.push_back(next_boundary);
      next_boundary += i_size;
    }

    auto vb{ClassicEncoding<T>::value_set.begin()};
    auto ve{ClassicEncoding<T>::value_set.end() - 1};

    size_t b{0};
    for (auto i{vb}; i <= ve; ++i) {
      dynamic_bitset<> e;

      while (b < boundary.size() and
             ClassicEncoding<T>::value_set[boundary[b]] < *i)
        ++b;

      e.resize(b, false);
      e.resize(boundary.size(), true);
      ClassicEncoding<T>::encoding_map[*i] = e;
    }
  }

  virtual const string getType() const { return "order"; }

  // returns (in string format) the test x[i]
  const string getLabel(const int i, const int v) const {
    std::stringstream ss;
    ss << (v ? "<=" : ">") << ClassicEncoding<T>::value_set[i];
    return ss.str();
  }
};

template <typename T> class Interval : public ClassicEncoding<T> {

// private:
//   size_t num_intervals;

public:
  // Interval(const size_t n) : ClassicEncoding<T>(), num_intervals(n) {}

  virtual size_t size() const {
    return ClassicEncoding<T>::value_set.size() - 1;
  }

  // encode the values of the iterator
  // template <typename RandomIt>
  void encode(typename std::vector<T>::iterator beg,
              typename std::vector<T>::iterator end) {

    ClassicEncoding<T>::encode(beg, end);

    // for (auto v{ClassicEncoding<T>::value_set.begin() + 1};
    //      v < ClassicEncoding<T>::value_set.end(); ++v) {
    //   assert(*(v - 1) < *v);
    // }

    size_t num_intervals{static_cast<size_t>(
        log(static_cast<double>(ClassicEncoding<T>::value_set.size())))};

    if (num_intervals < 1)
      num_intervals = 1;

    vector<size_t> boundary;
    size_t i_size{ClassicEncoding<T>::value_set.size() / num_intervals};
    size_t next_boundary{i_size};
    while (next_boundary < ClassicEncoding<T>::value_set.size()) {
      boundary.push_back(next_boundary);
      next_boundary += i_size;
    }

    // for(auto v : boundary) {
    // 	cout << v << ": " << ClassicEncoding<T>::value_set[v] << endl;
    // }
    // cout << ClassicEncoding<T>::value_set.size() << endl;

    // exit(1);

    // cout << "unary order encode\n";
    // for (auto v : ClassicEncoding<T>::value_set)
    //   cout << " " << v;
    // cout << endl;

    auto vb{ClassicEncoding<T>::value_set.begin()};
    auto ve{ClassicEncoding<T>::value_set.end() - 1};

    // auto b{boundary.begin()};
    size_t b{0};
    for (auto i{vb}; i <= ve; ++i) {
      dynamic_bitset<> e;

      while (b < boundary.size() and
             ClassicEncoding<T>::value_set[boundary[b]] < *i)
        ++b;

      e.resize(b, false);
      e.resize(boundary.size(), true);
      ClassicEncoding<T>::encoding_map[*i] = e;
    }
  }

  virtual const string getType() const { return "order"; }

  // returns (in string format) the test x[i]
  const string getLabel(const int i, const int v) const {
    std::stringstream ss;
    ss << (v ? "<=" : ">") << ClassicEncoding<T>::value_set[i];
    return ss.str();
  }
};

template <typename T> class Direct : public ClassicEncoding<T> {

public:
  virtual size_t size() const { return ClassicEncoding<T>::value_set.size(); }

  // encode the values of the iterator
  // template <typename RandomIt>
  void encode(typename std::vector<T>::iterator beg,
              typename std::vector<T>::iterator end) {

    ClassicEncoding<T>::encode(beg, end);

    // cout << "unary direct encode\n";
    // for (auto v : ClassicEncoding<T>::value_set)
    //   cout << " " << v;
    // cout << endl;

    auto vb{ClassicEncoding<T>::value_set.begin()};
    auto ve{ClassicEncoding<T>::value_set.end()};

    // last element has no literal -> 0..0 means not equal to any of the n-1
    // first elements
    for (auto i{vb}; i < ve; ++i) {
      dynamic_bitset<> e;
      e.resize(ve - vb, false);
      e.set(i - vb);
      ClassicEncoding<T>::encoding_map[*i] = e;
    }
  }

  virtual const string getType() const { return "direct"; }

  // returns (in string format) the test x[i]
  const string getLabel(const int i, const int v) const {
    std::stringstream ss;
    ss << (v ? "=" : "!=") << ClassicEncoding<T>::value_set[i];
    return ss.str();
  }
};

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

  std::vector<Encoding<int> *> int_encoder;
  std::vector<Encoding<float> *> float_encoder;
  std::vector<Encoding<std::string> *> symb_encoder;

  std::vector<std::vector<int>> int_value;
  std::vector<std::vector<float>> float_value;
  std::vector<std::vector<std::string>> symb_value;

  std::vector<string> label;
  string max_label;
  string min_label;
  //@}

private:
  std::vector<std::string> str_buffer;
  std::vector<int> int_buffer;
  std::vector<float> float_buffer;

  // size_t threshold{3};

  template <typename T>
  void computeSet(const std::vector<T> &elts, std::vector<T> &set) {

    set = elts;
    sort(set.begin(), set.end());
    set.erase(unique(set.begin(), set.end()), set.end());
  }

public:
  /*!@name Constructors*/
  //@{
  explicit TypedDataSet() {}
  ~TypedDataSet() {
    while (not int_encoder.empty()) {
      delete int_encoder.back();
      int_encoder.pop_back();
    }
    while (not float_encoder.empty()) {
      delete float_encoder.back();
      float_encoder.pop_back();
    }
    while (not symb_encoder.empty()) {
      delete symb_encoder.back();
      symb_encoder.pop_back();
    }
  }
  //@}

  /*!@name Accessors*/
  //@{
  size_t size() const { return label.size(); }
  size_t numFeature() const { return feature_label.size(); }
  bool typed() const { return feature_type.size() == numFeature(); }
  template <typename RandomIt>
  void setFeatures(RandomIt beg, RandomIt end, const int target) {
    auto width{end - beg};
    auto n_feature = (width - 1);
    auto column{(width + target) % width};

    feature_label.reserve(n_feature);
    for (auto f{beg}; f != end; ++f) {
      if (f - beg != column) {
        string feat{*f};
        boost::algorithm::trim(feat);
        addFeature(feat);
      }
    }
  }
  void addFeature(string &f) { feature_label.push_back(f); }

  template <typename RandomIt>
  void addExample(RandomIt beg, RandomIt end, const int target) {
    auto n_feature = (end - beg - 1);
    // assert(n_feature = numFeature());
    while (n_feature > numFeature()) {
      string s("f" + to_string(numFeature() + 1));
      addFeature(s);
    }

    auto width{end - beg};
    auto column{(width + target) % width};

    if (!typed())
      for (auto f{beg}; f != end; ++f) {
        if (f - beg != column)
          typeFeature(*f);
      }

    int j{0};
    for (auto f{beg}; f != end; ++f) {
      if (f - beg == column)
        continue;

      // auto j{f - beg};
      std::stringstream convert(*f);
      switch (feature_type[j]) {
      case INTEGER:
        int d;
        convert >> d;
        int_value[feature_rank[j]].push_back(d);
        break;
      case FLOAT:
        float z;
        convert >> z;
        float_value[feature_rank[j]].push_back(z);
        break;
      case SYMBOL:
        symb_value[feature_rank[j]].push_back(*f);
        break;
      }

      ++j;
    }

    auto l{*(beg + column)};

    boost::algorithm::trim(l);
    label.push_back(l);
  }

  // guess the type of the k-th feature, given a value v
  void typeFeature(string &s) {
    if (is<int>(s)) {
      feature_type.push_back(INTEGER);
      feature_rank.push_back(int_value.size());
      int_value.resize(int_value.size() + 1);
    } else if (is<float>(s)) {
      feature_type.push_back(FLOAT);
      feature_rank.push_back(float_value.size());
      float_value.resize(float_value.size() + 1);
    } else {
      feature_type.push_back(SYMBOL);
      feature_rank.push_back(symb_value.size());
      symb_value.resize(symb_value.size() + 1);
    }
  }

  // void binarize(DataSet &bin) {
  template <class Dataset> void binarize(Dataset &bin) {
    auto int_symbolic{0};
    for (auto f{0}; f < numFeature(); ++f)
      if (feature_type[f] == SYMBOL)
        ++int_symbolic;

    for (auto f{0}; f < numFeature(); ++f) {
      if (feature_type[f] == INTEGER) {

        computeSet(int_value[feature_rank[f]], int_buffer);

        Encoding<int> *enc;
        if (int_buffer.size() == 2) {
          enc = new TrivialEncoding<int>();
          enc->encode(int_buffer.begin(), int_buffer.end());
          int_encoder.push_back(enc);
        }
        // else if(int_buffer.size() > threshold and log2(*(int_buffer.begin()))
        // + log2(*(int_buffer.rbegin())) + 1 < int_buffer.size()) {
        //           enc = new BinaryDirect<int>();
        //           enc->encode(int_buffer.begin(), int_buffer.end());
        //           int_encoder.push_back(enc);
        //       	}
        else {
          enc = new Order<int>(int_value[feature_rank[f]].size(),
                               feature_label[f]);
          enc->encode(int_buffer.begin(), int_buffer.end());
          int_encoder.push_back(enc);
        }

        // cout << "int (" << feature_rank[f] << "/" << int_encoder.size() <<
        // "):";
        // for (auto v : int_buffer)
        //   cout << "\n" << v << " -> " << enc->getEncoding(v);
        // cout << endl;
        // cout << endl;

      } else if (feature_type[f] == FLOAT) {

        // cout << "compute set\n";

        computeSet(float_value[feature_rank[f]], float_buffer);

        // cout << feature_label[f] << " " << float_buffer.size() << endl;
        //
        // for(auto x : float_buffer)
        // 	cout << " " << x;
        // cout << endl;
        //
        // exit(1);

        // cout << "constructor\n";

        Encoding<float> *enc = new Order<float>(
            float_value[feature_rank[f]].size(), feature_label[f]);

        // cout << "encode\n";

        enc->encode(float_buffer.begin(), float_buffer.end());

        // cout << "push enc\n";

        float_encoder.push_back(enc);

        //         cout << "float (" << feature_rank[f] << "/" <<
        //         float_encoder.size()
        //              << "):";
        //         for (auto v : float_buffer)
        //           cout << "\n" << v << " -> " << enc->getEncoding(v);
        //         cout << endl;
        //         cout << endl;
        //
        // exit(1);

      } else if (feature_type[f] == SYMBOL) {

        computeSet(symb_value[feature_rank[f]], str_buffer);

        Encoding<std::string> *enc;
        if (str_buffer.size() > 2) {
          enc = new Direct<std::string>();
          enc->encode(str_buffer.begin(), str_buffer.end());
          symb_encoder.push_back(enc);
        } else {
          enc = new TrivialEncoding<std::string>();
          enc->encode(str_buffer.begin(), str_buffer.end());
          symb_encoder.push_back(enc);
        }

        // cout << "str (" << feature_rank[f] << "/" << symb_encoder.size()
        //      << "):";
        // for (auto v : str_buffer)
        //   cout << "\n" << v << " -> " << enc->getEncoding(v);
        // cout << endl;
        // cout << endl;
      }
    }

    min_label = *std::min_element(label.begin(), label.end());
    max_label = *std::max_element(label.begin(), label.end());

    auto bin_feature_count{0};
    for (auto i{0}; i < size(); ++i) {
      instance binex;
      for (auto f{0}; f < numFeature(); ++f) {
        auto r{feature_rank[f]};
        if (feature_type[f] == INTEGER) {
          binex =
              concatenate(binex, int_encoder[r]->getEncoding(int_value[r][i]));
        } else if (feature_type[f] == FLOAT) {
          binex = concatenate(binex,
                              float_encoder[r]->getEncoding(float_value[r][i]));
        } else if (feature_type[f] == SYMBOL) {
          binex = concatenate(binex,
                              symb_encoder[r]->getEncoding(symb_value[r][i]));
        }
        auto ref{bin_feature_count};
        while (bin_feature_count < binex.size()) {
          string binf{feature_label[f]};
          if (feature_type[f] == INTEGER) {
            binf += int_encoder[r]->getLabel(bin_feature_count++ - ref);
          } else if (feature_type[f] == FLOAT) {
            binf += float_encoder[r]->getLabel(bin_feature_count++ - ref);
          } else {
            binf += symb_encoder[r]->getLabel(bin_feature_count++ - ref);
          }

          // bin.addFeature(binf);
        }
      }

      // instance db;
      // bin.duplicate_format(binex, db);
      // bin.add(db, label[i] != min_label);
      bin.addBitsetExample(binex, label[i] != min_label);

      // cout << binex << endl;
    }

    // cout << bin.example_count() << endl;
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
        case INTEGER:
          os << " " << int_value[r][e];
          break;
        case FLOAT:
          os << " " << float_value[r][e];
          break;
        case SYMBOL:
          os << " " << symb_value[r][e];
          break;
        }
      }
      os << ": " << label[e] << endl;
    }

    return os;
  }

  std::ostream &writeMapping(std::ostream &os) const {
    os << "# label 2 " << min_label << " " << max_label;
    for (int f{0}; f < feature_label.size(); ++f) {
      os << "\n# " << feature_label[f];
      dtype t{feature_type[f]};
      int r{feature_rank[f]};
      switch (t) {
      case INTEGER:
        os << " " << int_encoder[r]->getType();
        if (int_encoder[r]->size() > 1)
          os << " " << int_encoder[r]->size();
        for (int j{0}; j < int_encoder[r]->size(); ++j)
          os << " " << int_encoder[r]->getLabel(j);
        break;
      case FLOAT:
        os << " " << float_encoder[r]->getType();
        if (float_encoder[r]->size() > 1)
          os << " " << float_encoder[r]->size();
        for (int j{0}; j < float_encoder[r]->size(); ++j)
          os << " " << float_encoder[r]->getLabel(j);
        break;
      case SYMBOL:
        os << " " << symb_encoder[r]->getType();
        if (symb_encoder[r]->size() > 1)
          os << " " << symb_encoder[r]->size();
        for (int j{0}; j < symb_encoder[r]->size(); ++j)
          os << " " << symb_encoder[r]->getLabel(j);
        break;
      }
    }
    os << endl;
    return os;
  }
};

std::ostream &operator<<(std::ostream &os, const TypedDataSet &x) {
  return x.display(os);
}
}

#endif // _PRIMER_TYPEDDATASET_HPP
