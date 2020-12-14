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

  void addBitsetExample(instance& x, const bool y);

  // template <class Algo> void toInc(Algo &algo);
	template <class Algo> void setup(Algo &algo);
  void preprocess(const bool verbose=false);

  size_t input_count(const bool c) const { return data[c].size(); }
  size_t input_example_count() const { return input_count(0) + input_count(1); }
	
  size_t count(const bool c) const { return examples[c].size(); }
  size_t example_count() const { return count(0) + count(1); }

  void printDatasetToTextFile(ostream &outfile, const bool first = true) const;
  void printDatasetToCSVFile(ostream &outfile, const string &delimiter = ",",
                             const bool first = false) const;

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

template <typename E_t>
template <class Algo>
inline void WeightedDataset<E_t>::setup(Algo &algo) {
  algo.clearExamples();

  for (int y = 0; y < 2; ++y)
    for (auto j : examples[y])
      algo.addBitsetExample(data[y][j], y, weight[y][j]);

  algo.setErrorOffset(suppression_count);
}

template <typename E_t> inline void WeightedDataset<E_t>::preprocess(const bool verbose) {

	auto t{cpu_time()};

  suppression_count = 0;
  // unsigned long dup_count = 0; // for statistics

  for (int y = 0; y < 2; ++y)
    std::sort(data[y].begin(), data[y].end());
	
  if (verbose)
    cout << "d sorttime=" << cpu_time() - t << endl;

  vector<instance>::iterator x[2] = {data[0].begin(), data[1].begin()};
  vector<instance>::iterator end[2] = {data[0].end(), data[1].end()};

  // int wght[2] = {1, 1};

  int i[2] = {0, 0};
  E_t wght[2] = {weight[0][i[0]], weight[1][i[1]]};

  while (x[0] != end[0] and x[1] != end[1]) {

    for (int y = 0; y < 2; ++y)
      while (x[y] != (end[y] - 1) and *(x[y]) == *(x[y] + 1)) {
        examples[y].remove_back(i[y]);
        ++x[y];
        ++i[y];
        wght[y] += weight[y][i[y]];
      }

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
        examples[0].remove_back(i[0]);
        suppression_count += wght[0];
      } else if (wght[0] > wght[1]) {
        weight[0][i[0]] = wght[0] - wght[1];
        examples[1].remove_back(i[1]);
        suppression_count += wght[1];
      } else {
        suppression_count += wght[1];
        examples[0].remove_back(i[0]);
        examples[1].remove_back(i[1]);
      }
      for (int y = 0; y < 2; ++y) {
        ++x[y];
        ++i[y];
        wght[y] = weight[y][i[y]];
      }
    }
  }

  for (int y = 0; y < 2; ++y) {
    wght[y] = 0;

    for (; x[y] != end[y]; ++x[y]) {
      assert(x[1 - y] == end[1 - y]);

      wght[y] += weight[y][i[y]];

      if (x[y] == end[y] - 1 or *x[y] != *(x[y] + 1)) {
        wght[y] = 0;
      } else {
        examples[y].remove_back(i[y]);
      }
    }
  }
	
	
	auto dup_count{input_count(0) + input_count(1) - count(0) - count(1) - 2 * suppression_count};
  if (verbose)
    std::cout << "d duplicate=" << dup_count << " suppressed=" << suppression_count
              << " ratio=" << float(dup_count + 2 * suppression_count) / input_example_count()
              << " count=" << input_example_count() << " negative=" << count(0)
              << " positive=" << count(1)
              << " final_count=" << example_count()
              << "\nd preprocesstime=" << cpu_time() - t << endl;

  // cout << suppression_count << endl;
}

}

#endif // _BLOSSOM_WEIGHTEDDATASET_HPP