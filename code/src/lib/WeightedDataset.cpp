

#include "WeightedDataset.hpp"
#include "typedef.hpp"

namespace blossom {

void WeightedDataset::addBitsetExample(instance &x, const bool y) {

  data[y].push_back(x);
}

void WeightedDataset::printDatasetToCSVFile(ostream &outfile,
                                            const string &delimiter,
                                            const bool first) const {

  if (first)
    outfile << "label";
  for (auto x{0}; x < data[0][0].size(); ++x) {
    if (first)
      outfile << delimiter << "f" << (x + 1);
    else
      outfile << "f" << (x + 1) << delimiter;
  }
  if (not first)
    outfile << "target\n";

  for (auto y{0}; y < 2; ++y) {
    for (auto x{0}; x < data[y].size(); ++x) {
      if (first)
        outfile << y;
      for (auto f{0}; f < data[y][x].size(); ++f) {
        if (first)
          outfile << delimiter << data[y][x][f];
        else
          outfile << data[y][x][f] << delimiter;
      }
      if (not first)
        outfile << y;
      outfile << endl;
    }
  }
}

void WeightedDataset::printDatasetToTextFile(ostream &outfile,
                                             const bool first) const {

  for (auto y{0}; y < 2; ++y) {
    for (auto x{0}; x < data[y].size(); ++x) {
      if (first)
        outfile << y;
      for (auto f{0}; f < data[y][x].size(); ++f) {
        if (first)
          outfile << " " << data[y][x][f];
        else
          outfile << data[y][x][f] << " ";
      }
      if (not first)
        outfile << y;
      outfile << endl;
    }
  }
}
}