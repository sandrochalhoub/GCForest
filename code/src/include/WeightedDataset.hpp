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

  template <class Algo>
  void to(Algo &algo);

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

template <class Algo>
inline void WeightedDataset::to(Algo &algo) {
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
  std::cout << "r duplicate=" << float(dup_count) / example_count() << std::endl;
}

}

#endif // _PRIMER_WEIGHTEDDATASET_HPP