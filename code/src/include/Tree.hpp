
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
  // private:
public:
  Wood &wood;
  int child_[2];
  int idx;

public:
  TreeNode(Wood &w);
  void free();

  int feature;
  const TreeNode &child(const bool t) const;
  const TreeNode &next(const instance x) const;

  bool isLeaf() const;
  bool prediction() const;

  void setChild(const bool branch, const int node_id);

  int getIndex() const;

  TreeNode &getLeaf(const instance &x) const;
  bool predict(const instance &x) const;
  int predict(const DataSet &data) const;
	
  /*!@name Miscellaneous*/
  //@{
  std::ostream &display(std::ostream &os, const int depth) const;
  //@}
};

// M
class Wood {

private:
  vector<TreeNode> stock;
  SparseSet available;

  void resize(const int k);

public:
  Wood();

  TreeNode &operator[](const int i);
  const TreeNode &operator[](const int i) const;

  size_t size();

  // allocate memory for a new node and returns its index
  int grow();

	// free the memory of node i 
  void freeNode(const int i);

};

std::ostream &operator<<(std::ostream &os, const TreeNode &x);


}


#endif // __TREE_HPP
