#ifndef _PRIMER_WEIGHTEDDATASET_HPP
#define _PRIMER_WEIGHTEDDATASET_HPP

#include <vector>
#include <algorithm>

#include "CmdLine.hpp"
#include "typedef.hpp"
#include "utils.hpp"

using namespace std;

namespace blossom {

class WeightedDataset {
public:
  WeightedDataset() = default;

  template <class rIter>
  void addExample(rIter beg_row, rIter end_row, const int target);
	
  void addBitsetExample(instance& x, const bool y);

  template <class Algo> void toInc(Algo &algo);

  size_t example_count() const { return data[0].size() + data[1].size(); }

  void printDatasetToTextFile(ostream &outfile, const bool first = true) const;
  void printDatasetToCSVFile(ostream &outfile, const string &delimiter = ",",
                             const bool first = false) const;

private:
  std::vector<instance> data[2];
};

template <class rIter>
inline void WeightedDataset::addExample(rIter beg_row, rIter end_row,
                                        const int target) {

  auto width{end_row - beg_row};
  auto column{(width + target) % width};
  auto y{*(beg_row + column)};

  if (data[y].size() == data[y].capacity()) {
    data[y].reserve(2 * data[y].capacity());
  }
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

  // std::vector<int> example(beg_instance, end_instance);
  // data[y].push_back(example);
}

template <class Algo> inline void WeightedDataset::toInc(Algo &algo) {

	auto t{cpu_time()};
	if (algo.options.verbosity >= DTOptions::NORMAL)
  	cout << "d readtime=" << t << endl;

  if (not algo.options.preprocessing) {
    for (auto y{0}; y < 2; ++y) {
      for (auto &b : data[y]) {
        algo.addBitsetExample(b, y, 1);
      }
    }
  } else {

    int dup_count = 0; // for statistics
    int sup_count = 0;

    for (int y = 0; y < 2; ++y)
      std::sort(data[y].begin(), data[y].end());

    if (algo.options.verbosity >= DTOptions::NORMAL)
      cout << "d sorttime=" << cpu_time() - t << endl;

    vector<instance>::iterator x[2] = {data[0].begin(), data[1].begin()};
    vector<instance>::iterator end[2] = {data[0].end(), data[1].end()};

    int weight[2] = {1, 1};
    while (x[0] != end[0] and x[1] != end[1]) {
      for (int y = 0; y < 2; ++y)
        while (x[y] != (end[y] - 1) and *(x[y]) == *(x[y] + 1)) {
          ++weight[y];
          ++x[y];
        }
      if (*x[0] < *x[1]) {
        algo.addBitsetExample(*x[0], 0, weight[0]);
        dup_count += (weight[0] - 1);
        ++x[0];
        weight[0] = 1;
      } else if (*x[0] > *x[1]) {
        algo.addBitsetExample(*x[1], 1, weight[1]);
        dup_count += (weight[1] - 1);
        ++x[1];
        weight[1] = 1;
      } else {
        if (weight[0] < weight[1]) {
          algo.addBitsetExample(*x[1], 1, weight[1] - weight[0]);
          sup_count += weight[0];
        } else if (weight[0] > weight[1]) {
          algo.addBitsetExample(*x[0], 0, weight[0] - weight[1]);
          sup_count += weight[1];
        } else {
          sup_count += weight[1];
        }
        for (int y = 0; y < 2; ++y) {
          ++x[y];
          weight[y] = 1;
        }
      }
    }

    for (int y = 0; y < 2; ++y) {
      weight[y] = 0;

      for (; x[y] != end[y]; ++x[y]) {
        assert(x[1 - y] == end[1 - y]);

        ++weight[y];

        if (x[y] == end[y] - 1 or *x[y] != *(x[y] + 1)) {
          algo.addBitsetExample(*x[y], y, weight[y]);
          weight[y] = 0;
        } else {
          dup_count++;
        }
      }
    }

    algo.setErrorOffset(sup_count);

    // print stats
    std::cout << "d duplicate=" << dup_count << " suppressed=" << sup_count
              << " ratio=" << float(dup_count + sup_count) / example_count()
              << " count=" << example_count()
              << " final_count=" << example_count() - (dup_count + sup_count)
              << std::endl;
  }

  if (algo.options.verbosity >= DTOptions::NORMAL)
    cout << "d preprocesstime=" << cpu_time() - t << endl;

}
}

#endif // _PRIMER_WEIGHTEDDATASET_HPP