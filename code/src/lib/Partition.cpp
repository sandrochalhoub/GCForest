
#include <assert.h>

#include "Partition.hpp"

using namespace std;

namespace primer {

TreePartition::TreePartition() {}

void TreePartition::init(const int n) {
  element.clear();
  element.reserve(n);
  for (auto e{0}; e < n; ++e)
    element.push_back(e);
}

size_t TreePartition::size() const { return part.size(); }

void TreePartition::clear() { part.clear(); }

size_t TreePartition::addNode() {
  Part p(element);
  part.push_back(p);
  return part.size() - 1;
}

void TreePartition::remNode() { part.pop_back(); }

Part &TreePartition::operator[](const int i) { return part[i]; }

const Part &TreePartition::operator[](const int i) const { return part[i]; }

std::ostream &TreePartition::display(std::ostream &os) const {

  for (auto i{part.begin()}; i != part.end(); ++i) {
    os << (i - part.begin()) << ": (" << i->count() << ") " << *i << endl;
  }

  return os;
}

std::ostream &operator<<(std::ostream &os, const TreePartition &x) {
  x.display(os);
  return os;
}

Part::Part(vector<int>& elt) : element(elt) {
  begin_idx = 0;
  end_idx = element.size();
}

Part& Part::operator=(const Part& p) noexcept {
  begin_idx = p.begin_idx;
  end_idx = p.end_idx;

  return *this;
}

// Part Part::append(Part& l) {
// 	Part r(element);
//
// 	assert(end_idx == l.begin_idx);
//
// 	r.begin_idx = begin_idx;
// 	r.end_idx = l.end_idx;
//
// 	return r;
// }


size_t Part::count() const {
  // assert(end_idx >= begin_idx);

  return end_idx - begin_idx;
}

vector<int>::iterator Part::begin() { return element.begin() + begin_idx; }

vector<int>::iterator Part::end() { return element.begin() + end_idx; }

vector<int>::const_iterator Part::begin() const {
  return element.begin() + begin_idx;
}

vector<int>::const_iterator Part::end() const {
  return element.begin() + end_idx;
}

std::ostream& Part::display(std::ostream& os) const {

  assert(begin() <= end());

  // assert(count() < 10000);

  for (auto i{begin()}; i != end(); ++i) {
    os << " " << *i;
  }

  return os;
}

std::ostream& operator<<(std::ostream& os, const Part& x) {
  x.display(os);
  return os;
}

}
