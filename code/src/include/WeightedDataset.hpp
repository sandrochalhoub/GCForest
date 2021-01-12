#ifndef _BLOSSOM_WEIGHTEDDATASET_HPP
#define _BLOSSOM_WEIGHTEDDATASET_HPP

#include <vector>
#include <algorithm>

#include "typedef.hpp"
#include "CmdLine.hpp"
#include "SparseSet.hpp"
#include "utils.hpp"

using namespace std;

namespace blossom {

// template <typename E_t> class DatapointList;

template <typename E_t> class WeightedDataset {
public:
  WeightedDataset() = default;

  template <class rIter>
  void addExample(rIter beg_row, rIter end_row, const int target,
                  const E_t w = 1);

  void addExample(const vector<int> &x);

  void addBitsetExample(instance &x, const bool y);

  // template <class Algo> void toInc(Algo &algo);
  // template <class Algo> void setup(Algo &algo) const;
  void preprocess(const bool verbose = false);

  size_t input_count(const bool c) const { return data[c].size(); }
  size_t input_example_count() const { return input_count(0) + input_count(1); }

  size_t count(const bool c) const { return examples[c].size(); }
  size_t example_count() const { return count(0) + count(1); }

  size_t numFeature() const { return data[0].empty() ? 0 : data[0][0].size(); }

  template <class selector>
  void printDatasetToFile(ostream &outfile, const string &delimiter,
                          const string &endline, selector not_redundant,
                          const bool first = true,
                          const bool header = false) const;

  template <class selector>
  void printHeader(ostream &outfile, const string &delimiter,
                   const string &endline, const string &label,
                   selector not_redundant, const bool first = true) const;

  // void printDatasetToTextFile(ostream &outfile, const bool first =
  // true)
  // const;
  // template <class selector>
  // void printDatasetToTextFile(ostream &outfile, selector s,
  //                             const bool first) const;
  // void printDatasetToCSVFile(ostream &outfile, const string &delimiter
  // = ",",
  //                            const bool first = false) const;
  // template <class selector>
  // void printDatasetToCSVFile(ostream &outfile, const string &delimiter
  // = ",",
  //                            const bool first = false) const;

  class List {
  public:
    List(const WeightedDataset<E_t> &dataset, const int y)
        : _dataset(dataset), _y(y) {}

    // iterators on the indices of the datapoints with non-null weight
    std::vector<int>::const_iterator begin() const {
      return _dataset.examples[_y].begin();
    }
    std::vector<int>::const_iterator end() const {
      return _dataset.examples[_y].end();
    }

    // accessors
    const instance &operator[](const int x) const {
      return _dataset.data[_y][x];
    }
    E_t weight(const int x) const { return _dataset.weight[_y][x]; }

    bool contain(const int x) const { return _dataset.examples[_y].contain(x); }

    size_t size() const { return _dataset.examples[_y].count(); }

  private:
    const WeightedDataset<E_t> &_dataset;
    const int _y;
  };

  List operator[](const int y) const { return List(*this, y); }

  size_t numInconsistent() const { return suppression_count; }

private:
  vector<instance> data[2];
  vector<E_t> weight[2];
  SparseSet examples[2];

  size_t suppression_count{0};
};

template <typename E_t>
void WeightedDataset<E_t>::addBitsetExample(instance &x, const bool y) {
  data[y].push_back(x);
}

template <typename E_t>
inline void WeightedDataset<E_t>::addExample(const vector<int> &example) {
  return addExample(example.begin(), example.end(), -1, 1);
}

template <typename E_t>
template <class rIter>
inline void WeightedDataset<E_t>::addExample(rIter beg_row, rIter end_row,
                                             const int target, const E_t w) {

  auto width{end_row - beg_row};
  auto column{(width + target) % width};
  auto y{*(beg_row + column)};

  if (data[y].size() == data[y].capacity()) {
    data[y].reserve(2 * data[y].capacity() + 2 * (data[y].empty()));
    examples[y].reserve(data[y].capacity());
  }

  examples[y].add(data[y].size());

  data[y].resize(data[y].size() + 1);
  data[y].back().resize(width - 1);

  int f{0};
  for (auto x{beg_row}; x != end_row; ++x) {
    assert(*x == 0 or *x == 1);
    if (x - beg_row != column) {
      if (*x)
        data[y].back().set(f);
      ++f;
    }
  }

  weight[y].push_back(w);
}

// template <typename E_t>
// template <class Algo>
// void WeightedDataset<E_t>::setup(Algo &algo) const {
//   algo.clearExamples();
//
//   for (int y = 0; y < 2; ++y)
//     for (auto j : examples[y])
//       algo.addBitsetExample(data[y][j], y, weight[y][j]);
//
//   algo.setErrorOffset(suppression_count);
// }

template <typename E_t> inline void WeightedDataset<E_t>::preprocess(const bool verbose) {

  auto t{cpu_time()};

  suppression_count = 0;
  // unsigned long dup_count = 0; // for statistics

  for (int y = 0; y < 2; ++y) {
    std::sort(data[y].begin(), data[y].end());
  }

  if (verbose)
    cout << "d sorttime=" << cpu_time() - t << endl;

  vector<instance>::iterator x[2] = {data[0].begin(), data[1].begin()};
  vector<instance>::iterator end[2] = {data[0].end(), data[1].end()};

  // int wght[2] = {1, 1};

  int i[2] = {0, 0};
  E_t wght[2] = {weight[0][i[0]], weight[1][i[1]]};

  // cout << endl << setw(3) << data[0].size() << " " << setw(3) <<
  // data[1].size() << endl;

  while (x[0] != end[0] and x[1] != end[1]) {

    // cout << endl << setw(3) << i[0] << " " << setw(3) << i[1] << endl;

    for (int y = 0; y < 2; ++y)
      while (x[y] != (end[y] - 1) and *(x[y]) == *(x[y] + 1)) {
        // cout << "remove (" << y << ") " << i[y] << endl;
        examples[y].remove_back(i[y]);
        ++x[y];
        ++i[y];
        wght[y] += weight[y][i[y]];
      }

    // cout << setw(3) << i[0] << " " << setw(3) << i[1] << endl;

    if (*x[0] < *x[1]) {
      weight[0][i[0]] = wght[0];
      ++x[0];
      ++i[0];
      wght[0] = weight[0][i[0]];
    } else if (*x[0] > *x[1]) {
      weight[1][i[1]] = wght[1];
      ++x[1];
      ++i[1];
      wght[1] = weight[1][i[1]];
    } else {
      if (wght[0] < wght[1]) {
        weight[1][i[1]] = wght[1] - wght[0];

        // cout << "remove0 " << i[0] << endl;
        examples[0].remove_back(i[0]);
        suppression_count += wght[0];
      } else if (wght[0] > wght[1]) {
        weight[0][i[0]] = wght[0] - wght[1];

        // cout << "remove1 " << i[1] << endl;
        examples[1].remove_back(i[1]);
        suppression_count += wght[1];
      } else {
        suppression_count += wght[1];

        // cout << "remove " << i[0] << " and " << i[1] << endl;

        examples[0].remove_back(i[0]);
        examples[1].remove_back(i[1]);
      }
      for (int y = 0; y < 2; ++y) {
        ++x[y];
        ++i[y];

        assert(i[y] < weight[y].size());

        wght[y] = weight[y][i[y]];
      }
    }
  }

  for (int y = 0; y < 2; ++y) {

    // cout << "wght[y]: " << wght[y] << endl;

    wght[y] = 0;

    for (; x[y] != end[y]; ++x[y]) {
      assert(x[1 - y] == end[1 - y]);

      wght[y] += weight[y][i[y]];

      if (x[y] == end[y] - 1 or *x[y] != *(x[y] + 1)) {
        weight[y][i[y]] = wght[y];
        wght[y] = 0;
      } else {

        // cout << "remove end " << i[y] << endl;

        examples[y].remove_back(i[y]);
      }
      ++i[y];
    }
  }

  auto dup_count{input_count(0) + input_count(1) - count(0) - count(1) -
                 2 * suppression_count};
  if (verbose)
    std::cout << "d duplicate=" << dup_count
              << " suppressed=" << suppression_count << " ratio="
              << float(dup_count + 2 * suppression_count) /
                     input_example_count()
              << " count=" << input_example_count() << " negative=" << count(0)
              << " positive=" << count(1) << " final_count=" << example_count()
              << "\nd preprocesstime=" << cpu_time() - t << endl;

  // for(auto i : examples[0]) {
  // 	cout << i << " " << weight[0][i] << endl;
  // }

  // for(auto y{0}; y<2; ++y) {
  // 	for(auto i{0}; i<data[y].size(); ++i) {
  // 		if(not examples[y].contain(i))
  // 			continue;
  //
  // 		int j{i+1};
  // 		while(j < data[y].size() and not examples[y].contain(j)) {
  // 			++j;
  // 		}
  // 		if(j >= data[y].size())
  // 			break;
  //
  // 		if(data[y][i] == data[y][j]) {
  // 			cout << y << " " << i << " " << j << endl;
  // 		}
  // 		// cout << examples[y][i-1] << " " << examples[y][i] << " " <<
  // data[y][examples[y][i-1]] << endl;
  // 		// if(data[y][examples[y][i-1]] == data[y][examples[y][i]]) {
  // 		// 	cout << examples[y][i-1] << " " << examples[y][i] << " "
  // <<
  // data[y][examples[y][i]] << endl;
  // 		// }
  // 		assert(data[y][i] != data[y][j]);
  // 	}
  // }
  //   // cout << suppression_count << endl;
}

template <typename E_t>
template <class selector>
void WeightedDataset<E_t>::printHeader(
    ostream &outfile, const string &delimiter, const string &endline,
    const string &label, selector not_redundant, const bool first) const {
  if (first)
    outfile << label;
  for (auto x{0}; x < data[0][0].size(); ++x) {
    if (not_redundant(x)) {
      if (first)
        outfile << delimiter << "f" << (x + 1);
      else
        outfile << "f" << (x + 1) << delimiter;
    }
  }
  if (not first)
    outfile << label;
  outfile << endline;
}

template <typename E_t>
template <class selector>
void WeightedDataset<E_t>::printDatasetToFile(
    ostream &outfile, const string &delimiter, const string &endline,
    selector not_redundant, const bool first, const bool header) const {

  if (header) {
    printHeader(outfile, delimiter, string(endline + "\n"),
                (first ? string("label") : string("target")), not_redundant,
                first);
  }

  for (auto y{0}; y < 2; ++y) {
    for (auto x : examples[y]) {
      if (first)
        outfile << y;
      for (auto f{0}; f < data[y][x].size(); ++f) {
        if (not_redundant(f)) {
          if (first)
            outfile << delimiter << data[y][x][f];
          else
            outfile << data[y][x][f] << delimiter;
        }
      }
      if (not first)
        outfile << y;
      outfile << endline << endl;
    }
  }
}
}

#endif // _BLOSSOM_WEIGHTEDDATASET_HPP