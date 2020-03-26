
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

  // void setLeaf(const bool branch, const bool y);
  // void setLeaf(const bool y);

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

  //
  int grow();

  void freeNode(const int);

  // create a new
  // getLeaf(const int pr);
};

class Tree
{

private:
  ///
  // vector<int> left_child;
  vector<int> child[2];
  vector<int> feature;
  //@}

public:
  /*!@name Constructors*/
  //@{
  explicit Tree();

  int getFeature(const int node) const;

  bool predict(const instance &x) const;

  int getLeaf(const instance &x) const;

  int predict(const DataSet &data) const;

  int getChild(const int x, const bool t) const;

  void addNode(const int c_true, const int c_false, const int f);
  void addNode(const int node, const int c_true, const int c_false,
               const int f);
  //@}

  /*!@name Miscellaneous*/
  //@{
  std::ostream &display(std::ostream &os) const;
  //@}
};

std::ostream &operator<<(std::ostream &os, const TreeNode &x);

std::ostream& operator<<(std::ostream& os, const Tree& x);


}


#endif // __TREE_HPP
