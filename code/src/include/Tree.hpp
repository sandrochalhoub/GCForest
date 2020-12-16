
#ifndef _BLOSSOM_TREE_HPP
#define _BLOSSOM_TREE_HPP

#include <iostream>
#include <vector>

#include "typedef.hpp"
#include "SparseSet.hpp"


/**********************************************
* ListTree
**********************************************/

using namespace std;

namespace blossom {


class Wood;


class Tree {
private:
  const Wood *wood;

public:
  int idx;

  Tree() = default;

  Tree(const Wood *w, const int i);

  int getChild(const int node, const int branch) const;

  int getFeature(const int node) const;
	
	template<class sample>
	bool predict(const sample &i) const;
  //
  // bool predict(const instance &i) const;

  template <typename E_t, class rIter, typename wf_type>
  E_t predict(rIter beg_neg, rIter end_neg, rIter beg_pos, rIter end_pos,
              wf_type weight_function) const;

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

  void resize(const int k);

public:
  SparseSet available;

  Wood();

  Tree operator[](const int i) const;
  // const Tree &operator[](const int i) const;

  size_t size() const;

  size_t count() const;

  // allocate memory for a new node and returns its index
  int grow();

  int copyNode(const int node);

  // free the memory of node i
  void freeNode(const int i);

  void setFeature(const int node, const int f);

  int getFeature(const int node) const;

  void setChild(const int node, const int branch, const int orphan);

  int getChild(const int node, const int branch) const;

	template<class sample>
	bool predict(const int node, const sample &i) const;
  // bool predict(const int node, const instance &x) const;

  size_t size(const int node) const;

  size_t depth(const int node) const;

  std::ostream &display(std::ostream &os, const int node,
                        const int depth) const;

#ifdef DEBUG
  vector<int> birthday;
  int today;
#endif
};


template<class sample>
bool Tree::predict(const sample &i) const {
  return wood->predict(idx, i);
}

template<class sample>
bool Wood::predict(const int node, const sample &x) const {
  if (node <= 1)
    return node;
  return predict(child[x[feature[node]]][node], x);
}

std::ostream &operator<<(std::ostream &os, const Tree &x);


// template<typename E_t, class rIter, typename wf_type>
// E_t Tree::predict(rIter beg_neg, rIter end_neg, rIter beg_pos, rIter end_pos, wf_type weight_function) const {
//   E_t error{0};
//   for (auto i{beg_neg}; i != end_neg; ++i)
//     error += weight_function(0, (i - beg_neg)) * (wood->predict(idx, *i) != 0);
//   for (auto i{beg_pos}; i != end_pos; ++i)
//     error += weight_function(1, (i - beg_pos)) * (wood->predict(idx, *i) != 1);
//   return error;
// }

}


#endif // __TREE_HPP
