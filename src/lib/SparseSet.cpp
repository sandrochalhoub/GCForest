
#include <assert.h>

#include "SparseSet.hpp"

SparseSet::SparseSet(const size_t n) {
  size_ = 0;
	start_ = 0;
  reserve(n);
}

void SparseSet::reserve(const size_t n) {
  while (list_.size() < n) {
    index_.push_back(list_.size());
    list_.push_back(list_.size());
  }
}

void SparseSet::save(size_t &stamp1, size_t &stamp2) { stamp1 = size_; stamp2 = start_; }
void SparseSet::restore(const size_t stamp1, const size_t stamp2) { size_ = stamp1; start_ = stamp2; }

//@}

/*!@name Accessors*/
//@{

bool SparseSet::safe_contain(const int elt) const {
  if (elt >= 0 && (size_t)elt < index_.size())
    return contain(elt);
  return false;
}

bool SparseSet::contain(const int elt) const { return index_[elt] < size_ and index_[elt] >= start_; }

size_t SparseSet::count() const { return size_ - start_; }
size_t SparseSet::size() const { return size_; }
size_t SparseSet::start() const { return start_; }
size_t SparseSet::capacity() const { return index_.size(); }

bool SparseSet::empty() const { return size_ == start_; }

int SparseSet::next(const int elt) const {
  size_t idx = index_[elt] + 1;
  return (idx < size_ ? list_[idx] : elt);
}
int SparseSet::prev(const int elt) const {
  size_t idx = index_[elt];
  return (idx > start_ ? list_[idx - 1] : elt);
}

int SparseSet::operator[](const size_t idx) const { return list_[idx+start_]; }

int &SparseSet::operator[](const size_t idx) { return list_[idx+start_]; }
//@}

/*!@name List Manipulation*/
//@{
std::vector<int>::iterator SparseSet::begin() { return list_.begin() + start_; }
std::vector<int>::reverse_iterator SparseSet::rbegin() {
  return list_.rend() + size_;
}

std::vector<int>::iterator SparseSet::end() { return list_.begin() + size_; }
std::vector<int>::reverse_iterator SparseSet::rend() { return list_.rend() + start_; }

std::vector<int>::const_iterator SparseSet::begin() const {
  return list_.begin() + start_;
}
std::vector<int>::const_reverse_iterator SparseSet::rbegin() const {
  return list_.rend() + size_;
}

std::vector<int>::const_iterator SparseSet::end() const {
  return list_.begin() + size_;
}
std::vector<int>::const_reverse_iterator SparseSet::rend() const {
  return list_.rend() + start_;
}

// std::vector<int>::iterator SparseSet::begin_after() { return end(); }
// std::vector<int>::reverse_iterator SparseSet::rbegin_after() {
//   return list_.rend();
// }
//
// std::vector<int>::iterator SparseSet::end_after() { return list_.end(); }
// std::vector<int>::reverse_iterator SparseSet::rend_after() { return rbegin(); }
//
// std::vector<int>::const_iterator SparseSet::begin_after() const {
//   return end();
// }
// std::vector<int>::const_reverse_iterator SparseSet::rbegin_after() const {
//   return list_.rend();
// }
//
// std::vector<int>::const_iterator SparseSet::end_after() const {
//   return list_.end();
// }
// std::vector<int>::const_reverse_iterator SparseSet::rend_after() const {
//   return rend();
// }

void SparseSet::fill() { size_ = list_.size(); start_ = 0; }

void SparseSet::clear() { size_ = 0; start_ = 0; }

// void SparseSet::set_size(const int s) { size_ = s; }

// void SparseSet::safe_remove_back(const int elt) {
//   if (elt >= 0) {
//     if (static_cast<size_t>(elt) >= list_.size()) {
//       reserve(elt + 1);
//     }
//     remove_back(elt);
//   }
// }

void SparseSet::remove_back(const int elt) {
  if (index_[elt] < size_ and index_[elt] >= start_)
    pull_back(elt);
}

void SparseSet::pull_back(const int elt) {
  auto last = list_[--size_];
  index_[last] = index_[elt];
  list_[index_[elt]] = last;
  list_[size_] = elt;
  index_[elt] = size_;
}

// void SparseSet::safe_remove_front(const int elt) {
//   if (elt >= 0) {
//     if (static_cast<size_t>(elt) >= list_.size()) {
//       reserve(elt + 1);
//     }
//     remove_front(elt);
//   }
// }

void SparseSet::remove_front(const int elt) {
  if (index_[elt] < size_ and index_[elt] >= start_)
    pull_front(elt);
}

void SparseSet::pull_front(const int elt) {
  auto first = list_[start_];
  index_[first] = index_[elt];
  list_[index_[elt]] = first;
  list_[start_] = elt;
  index_[elt] = start_++;
}

// void SparseSet::move(const int elt, const int idx_to) {
//   auto idx_from = index_[elt];
//
// 	// assert(idx_from )
//
//   // assert(index_[elt] <= static_cast<size_t>(idx_to));
//
//   auto last = list_[idx_to];
//   index_[last] = idx_from;
//   list_[idx_from] = last;
//   list_[idx_to] = elt;
//   index_[elt] = idx_to;
// }

void SparseSet::pop_back() { --size_; }
void SparseSet::pop_front() { ++start_; }

int SparseSet::front() const { return list_[start_]; }
int SparseSet::back() const { return list_[size_ - 1]; }

void SparseSet::safe_add(const int elt) {
  if (elt >= 0) {
    if (static_cast<size_t>(elt) >= list_.size()) {
      reserve(elt + 1);
    }
    add(elt);
  }
}

void SparseSet::add(const int elt) {
  if (index_[elt] >= size_)
    push_back(elt);
	else if(index_[elt] < start_)
		push_front(elt);
}

void SparseSet::push_back(const int elt) {
  auto next = list_[size_];
  index_[next] = index_[elt];
  list_[index_[elt]] = next;
  index_[elt] = size_;
  list_[size_++] = elt;
}

void SparseSet::push_front(const int elt) {
  auto next = list_[--start_];
  index_[next] = index_[elt];
  list_[index_[elt]] = next;
  index_[elt] = start_;
  list_[start_] = elt;
}

int SparseSet::index(const int elt) const { return index_[elt]; }
//@}

std::ostream &SparseSet::display(std::ostream &os) const {
  os << "(";
  for (auto it = begin(); it < end(); ++it) {
    os << " " << *it;
  }
  os << " )";
  return os;
}

std::ostream &operator<<(std::ostream &os, const SparseSet &x) {
  return x.display(os);
}

std::ostream &operator<<(std::ostream &os, const SparseSet *x) {
  return (x ? x->display(os) : os);
}
