#ifndef _BLOSSOM_WEIGHTEDDATASET_HPP
#define _BLOSSOM_WEIGHTEDDATASET_HPP

#include <vector>
#include <algorithm>
#include <random>

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
  void addFeatureLabel(const string &l);

  void rmExample(const bool y, const int idx);

  void addBitsetExample(instance &x, const bool y, const E_t w = 1);

  // template <class Algo> void toInc(Algo &algo);
  // template <class Algo> void setup(Algo &algo) const;
  void preprocess(const bool verbose = false);

  // // randomly select ratio * count(c) examples from classes c in {0,1}
  // void sample(const double ratio, const long seed = 12345);

  size_t input_count(const bool c) const { return data[c].size(); }
  size_t input_example_count() const { return input_count(0) + input_count(1); }

  size_t count(const bool c) const { return examples[c].size(); }
  size_t example_count() const { return count(0) + count(1); }

  E_t total(const bool c) const { return total_weight[c]; }
  E_t total() const { return total_weight[0] + total_weight[1]; }

  size_t numFeature() const { return data[0].empty() ? 0 : data[0][0].size(); }

  const vector<string>* getFeatureLabels() const;

  template <class selector>
  void printDatasetToFile(ostream &outfile, const string &delimiter,
                          const string &endline, selector not_redundant,
                          const bool first = true, const bool header = false,
                          const bool weighted = false) const;

  template <class selector>
  void printHeader(ostream &outfile, const string &delimiter,
                   const string &endline, const string &label,
                   selector not_redundant, const bool first = true) const;

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

  // // remove datapoint in indices and add them to subset
  // template <class Container>
  // void split(WeightedDataset<E_t> &subset, Container &choice);

  void drawSample(const double ratio, WeightedDataset<E_t> &training,
                  WeightedDataset<E_t> &test, const long seed = 12345);

private:
  vector<instance> data[2];
  vector<E_t> weight[2];
  vector<string> feature_label;

public:
  SparseSet examples[2];

private:
  // vector<pair<bool, size_t>> exlog;

  E_t total_weight[2]{0, 0};

  size_t suppression_count{0};
};

template <typename E_t>
void WeightedDataset<E_t>::addBitsetExample(instance &x, const bool y,
                                            const E_t w) {

  // exlog.push_back({y, data[y].size()});

  data[y].push_back(x);
  examples[y].reserve(data[y].capacity());
  examples[y].add(data[y].size() - 1);

  weight[y].push_back(w);
  ++total_weight[y];
}

template <typename E_t>
inline void WeightedDataset<E_t>::addExample(const vector<int> &example) {
  return addExample(example.begin(), example.end(), -1, 1);
}

template <typename E_t>
inline void WeightedDataset<E_t>::addFeatureLabel(const string &l) {
  feature_label.push_back(l);
}

// template <typename E_t>
// template <class Container>
// void WeightedDataset<E_t>::split(WeightedDataset<E_t> &subset,
//                                  Container &tests) {
//   // for (auto y{0}; y < 2; ++y) {
//   //   for (auto i : indices[y]) {
//   //     auto x{examples[y][i]};
//   //     subset.addBitsetExample(data[y][x], y, weight[y][x]);
//   //     total_weight -= weight[y][x];
//   //     examples[y].remove_back(x);
//   //   }
//   // }
//   for (auto t : tests) {
//     auto y{exlog[t].first};
//     auto x{exlog[t].second};
//     subset.addBitsetExample(data[y][x], y, weight[y][x]);
//     total_weight[y] -= weight[y][x];
//     examples[y].remove_back(x);
//   }
// }

template <typename E_t>
void WeightedDataset<E_t>::rmExample(const bool y, const int x) {
  auto cur_pos{examples[y].index(x)};
  auto z{examples[y].back()};

  examples[y].remove_back(x);

  assert(examples[y][cur_pos] == z);

  // cout << "replace " << x << " by " << z << endl;
  // // cout << data[y][cur_pos] << endl;
  // // cout << data[y][z] << endl;
  //
  //
  // assert(data[y].size() > x);
  // assert(data[y].size() > z);
  // data[y][x] = data[y][z];
  // data[y].pop_back();
}

template <typename E_t>
void WeightedDataset<E_t>::drawSample(const double ratio,
                                      WeightedDataset<E_t> &training,
                                      WeightedDataset<E_t> &test,
                                      const long seed) {
  mt19937 random_generator;
  random_generator.seed(seed);

  for (auto y{0}; y < 2; ++y) {
    size_t target{
        static_cast<size_t>(static_cast<double>(count(y)) * (1.0 - ratio))};

    // cout << target << " / " << count(y) << endl;

    // auto last{examples[y].bbegin()};
    while (count(y) > target) {
      auto i{random_generator() % count(y)};
      auto x{examples[y][i]};

      // cout << x << " -> test" << endl;

      // cout //<< " " << i
      // 	<< " " << x ;

      test.addBitsetExample(data[y][x], y, weight[y][x]);
      //       total_weight[y] -= weight[y][x];
      //       // examples[y].remove_back(x);
      //
      rmExample(y, x);

      // cout << (count(y) - target) << endl;
    }
    // cout << endl;

    for (auto x : examples[y]) {
      training.addBitsetExample(data[y][x], y, weight[y][x]);
    }

    // cout << examples[y] << endl;
  }
}

template <typename E_t>
template <class rIter>
inline void WeightedDataset<E_t>::addExample(rIter beg_row, rIter end_row,
                                             const int target, const E_t w) {

  auto width{end_row - beg_row};
  auto column{(width + target) % width};
  auto y{*(beg_row + column)};

  // exlog.push_back({y, data[y].size()});

  ++total_weight[y];

  if (data[y].size() == data[y].capacity()) {
    data[y].reserve(2 * data[y].capacity() + 2 * (data[y].empty()));
    examples[y].reserve(data[y].capacity());
  }

  examples[y].add(data[y].size());

  data[y].resize(data[y].size() + 1);
  data[y].back().resize(width - 1);

  int f{0};
  for (auto x{beg_row}; x != end_row; ++x) {
    // assert(*x == 0 or *x == 1);
    if (*x != 0 and *x != 1)
      throw 0;
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

// template <typename E_t> void WeightedDataset<E_t>::sample(const double ratio,
// const long seed) {
//   mt19937 random_generator;
//   random_generator.seed(seed);
//
//   for (auto y{0}; y < 2; ++y) {
//     size_t target{static_cast<size_t>(static_cast<double>(count(y)) *
//     ratio)};
//     while (count(y) > target) {
//       auto i{random_generator() % count(y)};
//       examples[y].remove_back(examples[y][i]);
//     }
//   }
// }

template <typename E_t> void WeightedDataset<E_t>::preprocess(const bool verbose) {

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

        // assert(i[y] < weight[y].size());

        if (x[y] != end[y])
          wght[y] = weight[y][i[y]];
        else
          wght[y] = 0;
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

  for (auto i{0}; i < 2; ++i)
    total_weight[i] -= suppression_count;

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

// template <typename E_t> void WeightedDataset<E_t>::preprocess(const bool
// verbose) {
//
//   auto t{cpu_time()};
//
//   suppression_count = 0;
//   // unsigned long dup_count = 0; // for statistics
//
//   for (int y = 0; y < 2; ++y) {
//     std::sort(examples[y].begin(), examples[y].end(), [&](const int i, const
//     int j) {return data[y][i] <= data[y][j]});
//   }
//
//   if (verbose)
//     cout << "d sorttime=" << cpu_time() - t << endl;
//
//   vector<instance>::iterator x[2] = {data[0].begin(), data[1].begin()};
//   vector<instance>::iterator end[2] = {data[0].end(), data[1].end()};
//
//   // int wght[2] = {1, 1};
//
//   int i[2] = {0, 0};
//   E_t wght[2] = {weight[0][i[0]], weight[1][i[1]]};
//
//   // cout << endl << setw(3) << data[0].size() << " " << setw(3) <<
//   // data[1].size() << endl;
//
//   while (x[0] != end[0] and x[1] != end[1]) {
//
//     // cout << endl << setw(3) << i[0] << " " << setw(3) << i[1] << endl;
//
//     for (int y = 0; y < 2; ++y)
//       while (x[y] != (end[y] - 1) and *(x[y]) == *(x[y] + 1)) {
//         // cout << "remove (" << y << ") " << i[y] << endl;
//         examples[y].remove_back(i[y]);
//         ++x[y];
//         ++i[y];
//         wght[y] += weight[y][i[y]];
//       }
//
//     // cout << setw(3) << i[0] << " " << setw(3) << i[1] << endl;
//
//     if (*x[0] < *x[1]) {
//       weight[0][i[0]] = wght[0];
//       ++x[0];
//       ++i[0];
//       wght[0] = weight[0][i[0]];
//     } else if (*x[0] > *x[1]) {
//       weight[1][i[1]] = wght[1];
//       ++x[1];
//       ++i[1];
//       wght[1] = weight[1][i[1]];
//     } else {
//       if (wght[0] < wght[1]) {
//         weight[1][i[1]] = wght[1] - wght[0];
//
//         // cout << "remove0 " << i[0] << endl;
//         examples[0].remove_back(i[0]);
//         suppression_count += wght[0];
//       } else if (wght[0] > wght[1]) {
//         weight[0][i[0]] = wght[0] - wght[1];
//
//         // cout << "remove1 " << i[1] << endl;
//         examples[1].remove_back(i[1]);
//         suppression_count += wght[1];
//       } else {
//         suppression_count += wght[1];
//
//         // cout << "remove " << i[0] << " and " << i[1] << endl;
//
//         examples[0].remove_back(i[0]);
//         examples[1].remove_back(i[1]);
//       }
//       for (int y = 0; y < 2; ++y) {
//         ++x[y];
//         ++i[y];
//
//         // assert(i[y] < weight[y].size());
//
//         if (x[y] != end[y])
//           wght[y] = weight[y][i[y]];
//         else
//           wght[y] = 0;
//       }
//     }
//   }
//
//   for (int y = 0; y < 2; ++y) {
//
//     // cout << "wght[y]: " << wght[y] << endl;
//
//     wght[y] = 0;
//
//     for (; x[y] != end[y]; ++x[y]) {
//       assert(x[1 - y] == end[1 - y]);
//
//       wght[y] += weight[y][i[y]];
//
//       if (x[y] == end[y] - 1 or *x[y] != *(x[y] + 1)) {
//         weight[y][i[y]] = wght[y];
//         wght[y] = 0;
//       } else {
//
//         // cout << "remove end " << i[y] << endl;
//
//         examples[y].remove_back(i[y]);
//       }
//       ++i[y];
//     }
//   }
//
//   auto dup_count{input_count(0) + input_count(1) - count(0) - count(1) -
//                  2 * suppression_count};
//   if (verbose)
//     std::cout << "d duplicate=" << dup_count
//               << " suppressed=" << suppression_count << " ratio="
//               << float(dup_count + 2 * suppression_count) /
//                      input_example_count()
//               << " count=" << input_example_count() << " negative=" <<
//               count(0)
//               << " positive=" << count(1) << " final_count=" <<
//               example_count()
//               << "\nd preprocesstime=" << cpu_time() - t << endl;
//
//   for (auto i{0}; i < 2; ++i)
//     total_weight[i] -= suppression_count;
//
//   // for(auto i : examples[0]) {
//   // 	cout << i << " " << weight[0][i] << endl;
//   // }
//
//   // for(auto y{0}; y<2; ++y) {
//   // 	for(auto i{0}; i<data[y].size(); ++i) {
//   // 		if(not examples[y].contain(i))
//   // 			continue;
//   //
//   // 		int j{i+1};
//   // 		while(j < data[y].size() and not examples[y].contain(j))
//   {
//   // 			++j;
//   // 		}
//   // 		if(j >= data[y].size())
//   // 			break;
//   //
//   // 		if(data[y][i] == data[y][j]) {
//   // 			cout << y << " " << i << " " << j << endl;
//   // 		}
//   // 		// cout << examples[y][i-1] << " " << examples[y][i] <<
//   "
//   "
//   <<
//   // data[y][examples[y][i-1]] << endl;
//   // 		// if(data[y][examples[y][i-1]] ==
//   data[y][examples[y][i]])
//   {
//   // 		// 	cout << examples[y][i-1] << " " << examples[y][i]
//   <<
//   "
//   "
//   // <<
//   // data[y][examples[y][i]] << endl;
//   // 		// }
//   // 		assert(data[y][i] != data[y][j]);
//   // 	}
//   // }
//   //   // cout << suppression_count << endl;
// }

template <typename E_t>
const vector<string>*
 WeightedDataset<E_t>::getFeatureLabels() const {
  return &feature_label;
 }

template <typename E_t>
template <class selector>
void WeightedDataset<E_t>::printHeader(
    ostream &outfile, const string &delimiter, const string &endline,
    const string &label, selector not_redundant, const bool first) const {
  if (first)
    outfile << label;


  // cout << "PRINTHEADER " << feature_label.size() << " / " << data[0][0].size() << endl;

  for (auto x{0}; x < data[0][0].size(); ++x) {
    if (not_redundant(x)) {
      // if (first)
      //   outfile << delimiter << "f" << (x + 1);
      // else
      //   outfile << "f" << (x + 1) << delimiter;
      if (first)
        outfile << delimiter << feature_label[x];
      else
        outfile << feature_label[x] << delimiter;
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
    selector not_redundant, const bool first, const bool header,
    const bool weighted) const {

  if (header) {
    printHeader(outfile, delimiter, string(endline + "\n"),
                (first ? string("label") : string("target")), not_redundant,
                first);
  }

  for (auto y{0}; y < 2; ++y) {
    for (auto x : examples[y]) {

      E_t count{(weighted ? 1 : weight[y][x])};

      while (count-- > 0) {

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

        if (weighted)
          outfile << delimiter << weight[y][x];

        outfile << endline << endl;
      }
    }
  }
}

template <typename E_t>
std::ostream &operator<<(std::ostream &os, const WeightedDataset<E_t> &x) {
  x.printDatasetToFile(os, string(" "), string(""),
                       [](const int f) { return true; }, 0, true);
  return os;
}
}

#endif // _BLOSSOM_WEIGHTEDDATASET_HPP
