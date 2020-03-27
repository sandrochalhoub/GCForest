
#include <assert.h>

#include "Tree.hpp"

#define POSITIVE -1

using namespace std;

namespace primer {

TreeNode::TreeNode(Wood &w) : wood(w) {
  idx = wood.size();
  child_[0] = -1;
  child_[1] = -1;
}

void TreeNode::free() {
  wood.freeNode(idx);
  for (auto i{0}; i < 2; ++i) {
    assert(child_[i] != idx);
    if (child_[i] >= 2)
      wood[child_[i]].free();
  }
}

int TreeNode::copy() {
  if (idx > 1) {
    int root{wood.grow()};
    wood[root].feature = feature;
    for (auto i{0}; i < 2; ++i)
      wood[root].setChild(i, wood[child_[i]].copy());
    return root;
  }
  return idx;
}

// void TreeNode::setLeaf(const bool y) {
// 	feature = y;
// }

const TreeNode &TreeNode::child(const bool t) const { return wood[child_[t]]; }

const TreeNode &TreeNode::next(const instance x) const {
  return wood[child_[x[feature]]];
}

bool TreeNode::isLeaf() const { return child_[0] < 0; }

bool TreeNode::prediction() const { return feature; }

void TreeNode::setChild(const bool branch, const int node_id) {
  // cout << "set child[" << branch << "] of " << getIndex() << " is " <<
  // node.getIndex() << endl;
  child_[branch] = node_id; //.getIndex();
  // cout << "(" << feature << "|" << child_[0] << "|" << child_[1] << ")\n";
}

// void TreeNode::setLeaf(const bool branch, const bool y) {
//   child_[branch] = y;
// }

int TreeNode::getIndex() const { return idx; }

TreeNode &TreeNode::getLeaf(const instance &x) const {	
  if (isLeaf())
    return wood[idx];
  return next(x).getLeaf(x);
}

bool TreeNode::predict(const instance &x) const {
  TreeNode &leaf{getLeaf(x)};
  return leaf.prediction();
}

int TreeNode::predict(const DataSet &data) const {
  auto error{0};
  for (auto y{0}; y < 2; ++y)
    for (auto i : data.example[y])
      error += (predict(data[i]) != y);
  return error;
}

std::ostream &TreeNode::display(std::ostream &os, const int depth) const {

  // cout << "(" << feature << "|" << child_[0] << "|" << child_[1] << ") ";

  if (depth > 5) {
    cout << "cutting\n";
    return os;
  }

  if (isLeaf())
    os << "class-" << feature << endl;
  else {
    os
        //<< " [" << idx << ":" << child_[0] << "|" << child_[1] << "] "
        << feature << endl;

    assert(child_[0] >= 0 and child_[1] >= 0);

    for (auto i{0}; i < depth; ++i)
      os << "    ";
    os << " yes:";

    assert(child(true).getIndex() != getIndex());

    child(true).display(os, depth + 1);

    for (auto i{0}; i < depth; ++i)
      os << "    ";
    os << " no:";

    assert(child(false).getIndex() != getIndex());

    child(false).display(os, depth + 1);
  }
  return os;
}

Wood::Wood() {
  stock[grow()].feature = false;
  stock[grow()].feature = true;
}

size_t Wood::size() { return stock.size(); }

TreeNode &Wood::operator[](const int i) { return stock[i]; }

const TreeNode &Wood::operator[](const int i) const { return stock[i]; }

void Wood::resize(const int k) {
  available.reserve(k);
  while (size() < k) {
    available.add(static_cast<int>(stock.size()));
    TreeNode node(*this);
    stock.push_back(node);
  }
}

//
int Wood::grow() {
  if (available.empty())
    resize(stock.size() + 1);
  auto node{*available.begin()};
  available.remove_front(node);

  // cout << "grow (" << node << "): " << available << endl;

  return node;
}

void Wood::freeNode(const int node) {
  available.add(node);

  // cout << "free (" << node << "): " << available << endl;
}

std::ostream &operator<<(std::ostream &os, const TreeNode &x) {
  x.display(os,0);
  return os;
}


}
