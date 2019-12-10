
#include <iostream>
#include <map>
#include <vector>

#include <boost/algorithm/string/trim.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/lexical_cast.hpp>

#include "DataSet.hpp"

#ifndef _PRIMER_TYPEDDATASET_HPP
#define _PRIMER_TYPEDDATASET_HPP

using namespace boost;


typedef int dtype;
typedef dynamic_bitset<> word;

#define INTEGER 0
#define FLOAT 0
#define SYMBOL 1

static const word nothing{word()};

namespace primer {

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

// encoding from type T to binary words
template <typename T> class Encoding {

public:
// encode the values of the iterator
virtual void encode(typename std::vector<T>::iterator beg,
typename std::vector<T>::iterator end) = 0;

// returns the word associated to value x
virtual const word &getEncoding(T &x) const = 0;

// returns (in string format) the test x[i]
virtual const string getLabel(const int i) const = 0;

//
// const bool test()
};

// encoding from type T to binary words
template <typename T> class TrivialEncoding : public Encoding<T> {

protected:
vector<T> value_set;
vector<word> lit;

public:
// encode the values of the iterator
virtual void encode(typename std::vector<T>::iterator beg,
typename std::vector<T>::iterator end) {
assert(end - beg == 2);
for (auto it{beg}; it != end; ++it)
value_set.push_back(*it);
lit.push_back(dynamic_bitset<>(1, false));
lit.push_back(dynamic_bitset<>(1, true));
}

// returns the word associated to value x
virtual const word &getEncoding(T &x) const {
return lit[(x == value_set[1])];
}

// returns (in string format) the test x[i]
virtual const string getLabel(const int i) const {
return "";
}

//
// const bool test()
};

// encoding from type T to binary words
template <typename T> class ClassicEncoding : public Encoding<T> {

protected:
map<T, word> encoding_map;
vector<T> value_set;

public:
// encode the values of the iterator
// template <typename RandomIt>
virtual void encode(typename std::vector<T>::iterator beg,
typename std::vector<T>::iterator end) {
for (auto it{beg}; it != end; ++it)
value_set.push_back(*it);
}

// returns the word associated to value x
const word &getEncoding(T &x) const {
auto it = encoding_map.find(x);
if (it != encoding_map.end()) {
return it->second;
}
return nothing;
}
};

template <typename T> 
class BinaryDirect : public ClassicEncoding<T> {

public:
// encode the values of the iterator
// template <typename RandomIt>
void encode(typename std::vector<T>::iterator beg,
typename std::vector<T>::iterator end) {

ClassicEncoding<T>::encode(beg, end);

auto vb{ClassicEncoding<T>::value_set.begin()};
auto ve{ClassicEncoding<T>::value_set.end()};
auto minv{*vb};
auto maxv{*(ve - 1)};

auto is_signed{(minv < 0 && maxv > 0)};

auto encoding_size{is_signed +
max((minv < 0 ? ceil(log2(abs(minv) + 1)) : 0),
(maxv > 0 ? ceil(log2(maxv + 1)) : 0))};

for (auto i{vb}; i != ve; ++i) {
dynamic_bitset<> e;
if (is_signed)
e.resize(encoding_size, abs(*i * 2) + is_signed);
else
e.resize(encoding_size, *i);
ClassicEncoding<T>::encoding_map[*i] = e;
}
}

// returns (in string format) the test x[i]
const string getLabel(const int i) const {
return ":2^" + to_string(ClassicEncoding<T>::value_set[i]);
}
};

template <typename T> class BinaryScaled : public ClassicEncoding<T> {

public:
// encode the values of the iterator
// template <typename RandomIt>
void encode(typename std::vector<T>::iterator beg,
typename std::vector<T>::iterator end) {

ClassicEncoding<T>::encode(beg, end);

auto vb{ClassicEncoding<T>::value_set.begin()};
auto ve{ClassicEncoding<T>::value_set.end()};
auto minv{*vb};
auto maxv{*(ve - 1)};

auto encoding_size{ceil(log2(vb - ve))};

for (auto i{vb}; i != ve; ++i) {
dynamic_bitset<> e;
e.resize(encoding_size, i - vb);
ClassicEncoding<T>::encoding_map[*i] = e;
}
}

// returns (in string format) the test x[i]
const string getLabel(const int i) const { return "&=?"; }
};

template <typename T> 
class Order : public ClassicEncoding<T> {

public:
// encode the values of the iterator
// template <typename RandomIt>
void encode(typename std::vector<T>::iterator beg,
typename std::vector<T>::iterator end) {

ClassicEncoding<T>::encode(beg, end);

cout << "unary order encode\n";
for (auto v : ClassicEncoding<T>::value_set)
cout << " " << v;
cout << endl;

auto vb{ClassicEncoding<T>::value_set.begin()};
auto ve{ClassicEncoding<T>::value_set.end() - 1};

for (auto i{vb}; i <= ve; ++i) {
dynamic_bitset<> e;
e.resize(i - vb, false);
e.resize(ve - vb, true);
ClassicEncoding<T>::encoding_map[*i] = e;
}
}

// returns (in string format) the test x[i]
const string getLabel(const int i) const {
return "<=" + to_string(ClassicEncoding<T>::value_set[i]);
}
};

template <typename T> class Direct : public ClassicEncoding<T> {

public:
// encode the values of the iterator
// template <typename RandomIt>
void encode(typename std::vector<T>::iterator beg,
typename std::vector<T>::iterator end) {

ClassicEncoding<T>::encode(beg, end);

cout << "unary direct encode\n";
for (auto v : ClassicEncoding<T>::value_set)
cout << " " << v;
cout << endl;

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

// returns (in string format) the test x[i]
const string getLabel(const int i) const {
std::stringstream ss;
ss << "=" << ClassicEncoding<T>::value_set[i];
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
//@}

private:
std::vector<std::string> str_buffer;
std::vector<int> int_buffer;
std::vector<float> float_buffer;

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
//@}

/*!@name Accessors*/
//@{
size_t size() const { return label.size(); }
size_t numFeature() const { return feature_label.size(); }
bool typed() const { return feature_type.size() == numFeature(); }
template <typename RandomIt> void setFeatures(RandomIt beg, RandomIt end) {
auto n_feature = (end - beg);
feature_label.reserve(n_feature);
for (auto f{beg}; f != end; ++f) {
string feat{*f};
boost::algorithm::trim(feat);
addFeature(feat);
}
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
case INTEGER:
int d;
convert >> d;
int_value[feature_rank[j]].push_back(d);
break;
case FLOAT:
float d;
convert >> d;
float_value[feature_rank[j]].push_back(d);
break;
case SYMBOL:
symb_value[feature_rank[j]].push_back(*f);
break;
}
}

boost::algorithm::trim(l);
label.push_back(l);
}

// guess the type of the k-th feature, given a value v
void typeFeature(string &s) {

// auto numeric{!s.empty() and std::find_if(s.begin(), s.end(), [](char c) {
//                               return !std::isdigit(c);
//                             }) == s.end()};

// boost::algorithm::trim(s);

auto numeric{is<int>(s)};

if (is<int>(s)) {
feature_type.push_back(INTEGER);
feature_rank.push_back(int_value.size());
int_value.resize(int_value.size() + 1);
} else if(is<float>(s)){
feature_type.push_back(FLOAT);
feature_rank.push_back(float_value.size());
float_value.resize(float_value.size() + 1);
} else {
feature_type.push_back(SYMBOL);
feature_rank.push_back(symb_value.size());
symb_value.resize(symb_value.size() + 1);
}
}



void binarize(DataSet &bin) {
auto int_symbolic{0};
for (auto f{0}; f < numFeature(); ++f)
if (feature_type[f] == SYMBOL)
++int_symbolic;

for (auto f{0}; f < numFeature(); ++f) {
if (feature_type[f] == INTEGER) {

computeSet(int_value[feature_rank[f]], int_buffer);

Encoding<int> *enc;
if (int_buffer.size() > 2) {
enc = new Order<int>();
enc->encode(int_buffer.begin(), int_buffer.end());
int_encoder.push_back(enc);
} else {
enc = new TrivialEncoding<int>();
enc->encode(int_buffer.begin(), int_buffer.end());
int_encoder.push_back(enc);
}

cout << "int (" << feature_rank[f] << "/" << int_encoder.size()
<< "):";
for (auto v : int_buffer)
cout << "\n" << v << " -> " << enc->getEncoding(v);
cout << endl;
cout << endl;

} else if (feature_type[f] == INTEGER) {

computeSet(float_value[feature_rank[f]], float_buffer);

Encoding<float> *enc = new TrivialEncoding<float>();
enc->encode(float_buffer.begin(), float_buffer.end());
float_encoder.push_back(enc);

cout << "float (" << feature_rank[f] << "/" << float_encoder.size()
<< "):";
for (auto v : float_buffer)
cout << "\n" << v << " -> " << enc->getEncoding(v);
cout << endl;
cout << endl;

} else if (feature_type[f] == SYMBOL) {

computeSet(symb_value[feature_rank[f]], str_buffer);

Encoding<std::string> *enc;	
if (str_buffer.size() > 2) {
enc = new Direct<std::string>();
enc->encode(str_buffer.begin(), str_buffer.end());
symb_encoder.push_back(enc);
} else {
enc =
new TrivialEncoding<std::string>();
enc->encode(str_buffer.begin(), str_buffer.end());
symb_encoder.push_back(enc);
}

cout << "str (" << feature_rank[f] << "/" << symb_encoder.size()
<< "):";
for (auto v : str_buffer)
cout << "\n" << v << " -> " << enc->getEncoding(v);
cout << endl;
cout << endl;

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
if (feature_type[f] == INTEGER) {
// binex = concatenate(binex, int_encoding[r][int_value[r][i]]);
binex = concatenate(
binex, int_encoder[r]->getEncoding(
int_value[r][i])); // [int_value[r][i]]);
} else if (feature_type[f] == SYMBOL) {
// binex = concatenate(binex, str_encoding[r][symb_value[r][i]]);
binex = concatenate(
binex, symb_encoder[r]->getEncoding(symb_value[r][i]));
}
auto ref{bin_feature_count};
while (bin_feature_count < binex.size()) {
string binf{feature_label[f]};
if (feature_type[f] == INTEGER) {
binf += int_encoder[r]->getLabel(bin_feature_count++ - ref);
} else {
binf += symb_encoder[r]->getLabel(bin_feature_count++ - ref);
}

// std::to_string(binex.size() - bin_feature_count++)};
bin.addFeature(binf);
}
}

cout << binex << " " << (label[i] == label[0]) << endl;

// binex.resize(2 * binex.size());
word db;
bin.duplicate_format(binex, db);
bin.add(db, label[i] == label[0]);
}

// cout << bin << endl;
//
// cout << bin.numFeature() << endl;
//
// cout << bin.featureName(0) << " " << bin.featureName(bin.numFeature())
//      << endl;
// exit(1);
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
case SYMBOL:
os << " " << symb_value[r][e];
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
