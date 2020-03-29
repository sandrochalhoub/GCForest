
#include <iostream>
#include <vector>

#include "DataSet.hpp"

#ifndef __TREE_HPP
#define __TREE_HPP

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
};

// class Wood;
// class TreeNode {
//   // private:
// public:
//   Wood *wood;
//   int child_[2];
//   int idx;
//
// public:
//   TreeNode(Wood *w);
//   void free();
//
//   int copy();
//
//   int feature;
//   const TreeNode &child(const bool t) const;
//   const TreeNode &next(const instance x) const;
//
//   bool isLeaf() const;
//   bool prediction() const;
//
//   void setChild(const bool branch, const int node_id);
//
//   int getIndex() const;
//
//   TreeNode &getLeaf(const instance &x) const;
//   bool predict(const instance &x) const;
//   int predict(const DataSet &data) const;
//
//   /*!@name Miscellaneous*/
//   //@{
//   std::ostream &display(std::ostream &os, const int depth) const;
//   //@}
// };
//
// // M
// class Wood {
//
// private:
//   vector<TreeNode> stock;
//   SparseSet available;
//
//   void resize(const int k);
//
// public:
//   Wood();
//
//   TreeNode &operator[](const int i);
//   const TreeNode &operator[](const int i) const;
//
//   size_t size();
//
//   // allocate memory for a new node and returns its index
//   int grow();
//
// 	// free the memory of node i
//   void freeNode(const int i);
//
// 	void setFeature(const int node, const int f);
//
// 	void setChild(const int node, const int branch, const int orphan);
//
// };

std::ostream &operator<<(std::ostream &os, const TreeNode &x);


}


#endif // __TREE_HPP
