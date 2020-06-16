#ifndef _PRIMER_WEIGHTEDDATASET_HPP
#define _PRIMER_WEIGHTEDDATASET_HPP

#include <vector>
#include <algorithm>

#include "DataSet.hpp"

namespace primer {

class WeightedDataset {
public:
  typedef std::vector<int> sample;

  WeightedDataset() = default;

  template <class rIter>
  void addExample(rIter beg_sample, rIter end_sample, const bool y);

  template <class Algo> void toInc(Algo &algo);

  template <class Algo> void to(Algo &algo);

  size_t example_count() const {
    return data[0].size() + data[1].size();
  }

private:
  std::vector<std::vector<int>> data[2];
};

template <class rIter>
inline void WeightedDataset::addExample(rIter beg_sample, rIter end_sample, const bool y) {
  std::vector<int> sample(beg_sample, end_sample);
  data[y].push_back(sample);
}

template <class Algo> inline void WeightedDataset::to(Algo &algo) {
  int dup_count = 0; // for statistics

  for (int y = 0; y < 2; ++y) {
    auto &subset = data[y];

    std::sort(subset.begin(), subset.end());
    int weight = 0;

    for (size_t i{0}; i < subset.size(); ++i) {
      weight++;

      if (i == subset.size() - 1 || subset[i] != subset[i+1]) {
        algo.addExample(subset[i].begin(), subset[i].end(), y, weight);
        weight = 0;
      }
      else {
        dup_count++;
      }
    }
  }

  // print stats
  std::cout << "d duplicate=" << dup_count
            << " dratio=" << float(dup_count) / example_count() << std::endl;
}

template <class Algo> inline void WeightedDataset::toInc(Algo &algo) {
  int dup_count = 0; // for statistics
  int sup_count = 0;

  for (int y = 0; y < 2; ++y)
    std::sort(data[y].begin(), data[y].end());

  vector<vector<int>>::iterator x[2] = {data[0].begin(), data[1].begin()};
  vector<vector<int>>::iterator end[2] = {data[0].end(), data[1].end()};

  int weight[2] = {1, 1};
  while (x[0] != end[0] and x[1] != end[1]) {
    for (int y = 0; y < 2; ++y)
      while (x[y] != (end[y] - 1) and *(x[y]) == *(x[y] + 1)) {
        ++weight[y];
        ++x[y];
      }
    if (*x[0] < *x[1]) {
      algo.addExample(x[0]->begin(), x[0]->end(), 0, weight[0]);
      dup_count += (weight[0] - 1);
      ++x[0];
      weight[0] = 1;
    } else if (*x[0] > *x[1]) {
      algo.addExample(x[1]->begin(), x[1]->end(), 1, weight[1]);
      dup_count += (weight[1] - 1);
      ++x[1];
      weight[1] = 1;
    } else {
      if (weight[0] < weight[1]) {
        algo.addExample(x[1]->begin(), x[1]->end(), 1, weight[1] - weight[0]);
        sup_count += weight[0];
      } else if (weight[0] > weight[1]) {
        algo.addExample(x[0]->begin(), x[0]->end(), 0, weight[0] - weight[1]);
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
		
    for (;x[y] != end[y]; ++x[y]) {
      assert(x[1 - y] == end[1 - y]);

      ++weight[y];

      if (x[y] == end[y] - 1 or *x[y] != *(x[y] + 1)) {
        algo.addExample(x[y]->begin(), x[y]->end(), y, weight[y]);
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
            << std::endl;
}

}

#endif // _PRIMER_WEIGHTEDDATASET_HPP