#include <vector>

#include <boost/dynamic_bitset.hpp>

using namespace boost;
using namespace std;

namespace primer {

typedef dynamic_bitset<> instance;

class Dataset {
public:

  /// List of positive features
  vector<vector<int>> example[2];

  /// Dataset as a bitset
  vector<instance> dataset[2];
  /// For each feature, does the example has it?
  vector<dynamic_bitset<>> reverse_dataset[2];
};

template <typename T>
class WeightedDataset : public Dataset {
public:
  vector<T> weights[2];
};

}
