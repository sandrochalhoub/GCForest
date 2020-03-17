
#include <iostream>

#include <vector>

#ifndef __PARTITION_HPP
#define __PARTITION_HPP

/**********************************************
* ListTree
**********************************************/

using namespace std;

namespace primer {

class Part
{

private:
  ///
  vector<int> &element;
  //@}

public:
  /*!@name Parameters*/
  //@{
  size_t begin_idx;
  size_t end_idx;

  /*!@name Constructors*/
  //@{
  explicit Part(vector<int> &elt);

  Part &operator=(const Part &p) noexcept;

  size_t count() const;

  vector<int>::iterator begin();
  vector<int>::iterator end();

  vector<int>::const_iterator begin() const;
  vector<int>::const_iterator end() const;

  template <typename boolean_function>
  void split(Part &l1, Part &l2, boolean_function condition);

  /*!@name Miscellaneous*/
  //@{
  std::ostream &display(std::ostream &os) const;
  //@}
};

std::ostream& operator<<(std::ostream& os, const Part& x);

class TreePartition
{
private:
  vector<int> element;
  vector<Part> part;

public:
  TreePartition();
  void clear();

  size_t size() const;

  Part &operator[](const int i);
  const Part &operator[](const int i) const;

  template <typename RandIt> void copy(RandIt s, RandIt e);
  void init(const int n);

  size_t addNode();
  void remNode();

  template <typename boolean_function>
  void branch(const int node, boolean_function condition);

  template <typename boolean_function>
  void branch(const int node, const int x, const int y,
              boolean_function condition);

  /*!@name Miscellaneous*/
  //@{
  std::ostream &display(std::ostream &os) const;
  //@}
};


std::ostream& operator<<(std::ostream& os, const TreePartition& x);

// template <typename boolean_function>
// void Part::split(const int l1, const int l0, boolean_function condition) {
// 	split(part[l1], part[l0], condition);
// }

template <typename boolean_function>
void Part::split(Part &l1, Part &l0, boolean_function condition) {
  auto i{begin()};
  auto j{end()};

  // for (auto k{begin()}; k != end(); ++k) {
  //   cout << condition(*k);
  // }
  // cout << endl;

  assert(begin() <= end());

  while (true) {

    // cout << "split " << (i - begin()) << ".." << (j-begin()) << endl;
    // cout << "split " << *i << ".." << *(j-1) << endl;

    while (i < j and condition(*i))
      ++i;
    if (i + 1 >= j)
      break;

    while (j > i and not condition(*(--j)))
      ;
    if (i == j)
      break;

    // cout << "swap " << *i << ".." << *(j-1) << endl;

    std::swap(*i, *j);

    assert(condition(*i));
    assert(not condition(*j));

    ++i;
  }

  // cout << "stop on " << (i - begin()) << ".." << (j-begin()) << endl;

  assert(i == end() or not condition(*i));
  assert(i == begin() or condition(*(i - 1)));

  l1.begin_idx = begin_idx;
  l1.end_idx = begin_idx + i - begin();

  l0.begin_idx = l1.end_idx;
  l0.end_idx = end_idx;

  // for (auto k{begin()}; k != end(); ++k) {
  //   cout << condition(*k);
  // }
  // cout << endl;
  //
  // cout << l0.count() << "/" << l1.count() << endl;

  // cout << begin_idx << " " << l1.begin_idx << endl;
  // cout << (i - begin()) << " " << l1.end_idx << endl;
  // cout << l1.end_idx << " " << l2.begin_idx << endl;
  // cout << end_idx << " " << l2.end_idx << endl;
  //
  //
  // auto l1c{l1.count()};
  // auto l2c{l2.count()};
  //
  // cout << "split result: " << count() << " -> " << l1c << " / " << l2c
  // << endl;
  //

  assert(l1.begin() <= l1.end());
  assert(l0.begin() <= l0.end());
}


template<typename RandIt>
void TreePartition::copy(RandIt s, RandIt e) {
  element.resize(std::distance(s, e));
  std::copy(s, e, element.begin());
}

template<typename boolean_function>
void TreePartition::branch(const int node, boolean_function condition) {
  auto c1{addNode()};
  auto c0{addNode()};
  part[node].split(part[c1], part[c0], condition);
}

template <typename boolean_function>
void TreePartition::branch(const int node, const int c1, const int c0,
                           boolean_function condition) {
  // auto c1{addNode()};
  // auto c0{addNode()};
  part[node].split(part[c1], part[c0], condition);
}
}


#endif // __PARTITION_HPP
