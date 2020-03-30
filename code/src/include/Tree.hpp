
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
class TreeNode {
private:
  Wood *wood;

public:
  const int idx;

  TreeNode(Wood *w, const int i);

  // bool predict(const instance &x) const;
  int predict(const DataSet &data) const;

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

  TreeNode operator[](const int i);
  // const TreeNode &operator[](const int i) const;

  size_t size();

  size_t count();

  // allocate memory for a new node and returns its index
  int grow();

  int copyNode(const int node);

  // free the memory of node i
  void freeNode(const int i);

  void setFeature(const int node, const int f);

  void setChild(const int node, const int branch, const int orphan);

  // int getFeature(const int node) const;
  //
  // int getChild(const int node, const inr branch);

  bool predict(const int node, const instance &x) const;

  std::ostream &display(std::ostream &os, const int node,
                        const int depth) const;

#ifdef DEBUG
  vector<int> birthday;
  int today;
#endif
};

std::ostream &operator<<(std::ostream &os, const TreeNode &x);


}


#endif // __TREE_HPP
