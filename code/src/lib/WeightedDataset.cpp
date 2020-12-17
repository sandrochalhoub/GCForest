

#include "WeightedDataset.hpp"
// #include "typedef.hpp"

namespace blossom {

template <typename E_t>
void WeightedDataset<E_t>::addBitsetExample(instance &x, const bool y) {

  data[y].push_back(x);
}

template class WeightedDataset<int>;
template class WeightedDataset<unsigned long>;
template class WeightedDataset<float>;
template class WeightedDataset<double>;
}