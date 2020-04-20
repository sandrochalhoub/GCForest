
#include <iostream>
#include <vector>

#include "DataSet.hpp"

#ifndef __TREE_HPP
#define __TREE_HPP

// #define DEBUG

/**********************************************
* ListTree
**********************************************/

using namespace std;

namespace primer {

class Wood;
class Tree {
private:
  Wood *wood;

public:
  const int idx;

  Tree(Wood *w, const int i);

  int getChild(const int i, const int branch);

  // bool predict(const instance &x) const;
  int predict(const DataSet &data) const;
	
	template<class rIter>
	int predict(rIter beg_neg, rIter end_neg, rIter beg_pos, rIter end_pos) const;

        size_t size() const;

        size_t depth() const;

        /*!@name Miscellaneous*/
        //@{
        std::ostream &display(std::ostream &os) const;
        //@}
};

// M
class Wood {

private:
  vector<int> child[2];
  vector<int> feature;
  SparseSet available;

  void resize(const int k);

public:
  Wood();

  Tree operator[](const int i);
  // const Tree &operator[](const int i) const;

  size_t size();

  size_t count();

  // allocate memory for a new node and returns its index
  int grow();

  int copyNode(const int node);

  // free the memory of node i
  void freeNode(const int i);

  void setFeature(const int node, const int f);

  void setChild(const int node, const int branch, const int orphan);

  int getChild(const int node, const int branch);

  // int getFeature(const int node) const;
  //
  // int getChild(const int node, const inr branch);

  bool predict(const int node, const instance &x) const;

  size_t size(const int node) const;

  size_t depth(const int node) const;

  std::ostream &display(std::ostream &os, const int node,
                        const int depth) const;

#ifdef DEBUG
  vector<int> birthday;
  int today;
#endif
};

std::ostream &operator<<(std::ostream &os, const Tree &x);


template<class rIter>
int Tree::predict(rIter beg_neg, rIter end_neg, rIter beg_pos, rIter end_pos) const {
  auto error{0};
  // for (auto y{0}; y < 2; ++y)
    for (auto i{beg_neg}; i!=end_neg; ++i)
      error += (wood->predict(idx, *i) != 0);
    for (auto i{beg_pos}; i!=end_pos; ++i)
      error += (wood->predict(idx, *i) != 1);
  return error;
}

}


#endif // __TREE_HPP
