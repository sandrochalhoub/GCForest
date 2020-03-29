
#include <assert.h>

#include "Tree.hpp"

#define POSITIVE -1

using namespace std;

namespace primer {

TreeNode::TreeNode(Wood *w, const int node) : wood(w), idx(node) {}

int TreeNode::predict(const DataSet &data) const {
  auto error{0};
  for (auto y{0}; y < 2; ++y)
    for (auto i : data.example[y])
      error += (wood->predict(idx, data[i]) != y);
  return error;
}

std::ostream &TreeNode::display(std::ostream &os) const {

  return wood->display(os, idx, 0);
}

std::ostream &Wood::display(std::ostream &os, const int node,
                            const int depth) const {
  // cout << "(" << feature << "|" << child_[0] << "|" << child_[1] << ") ";

  if (node <= 1)
    os << "class-" << node << endl;
  else {
    os << feature[node] << endl;

    assert(child[0][node] >= 0 and child[1][node] >= 0);

    for (auto i{0}; i < depth; ++i)
      os << "    ";
    os << " yes:";

    display(os, child[true][node], depth + 1);

    for (auto i{0}; i < depth; ++i)
      os << "    ";
    os << " no:";

    display(os, child[false][node], depth + 1);
  }

  return os;
}

Wood::Wood() {
  for (auto i{0}; i < 2; ++i)
    grow();
}

size_t Wood::size() { return feature.size(); }

TreeNode Wood::operator[](const int i) {
  TreeNode t(this, i);
  return t;
}

void Wood::resize(const int k) {
  available.reserve(k);
  for (auto i{feature.size()}; i < k; ++i)
    available.add(i);
  feature.resize(k, -1);
  child[0].resize(k, -1);
  child[1].resize(k, -1);
}

//
int Wood::grow() {

  // cout << available << " (" << (available.empty() ? "empty" : "not empty") <<
  // ")\n" ;

  if (available.empty())
    resize(feature.size() + 1);
  auto node{*available.begin()};
  available.remove_front(node);

  // cout << "return " << node << endl;

  return node;
}

int Wood::copyNode(const int node) {
  if (node > 1) {
    int root{grow()};
    feature[root] = feature[node];
    for (auto i{0}; i < 2; ++i)
      child[i][root] = copyNode(child[i][node]);
    return root;
  }
  return node;
}

void Wood::freeNode(const int node) {
  if (node > 1) {
    available.add(node);
    for (auto i{0}; i < 2; ++i)
      if (child[i][node] >= 2)
        freeNode(child[i][node]);
  }
  // cout << "free (" << node << "): " << available << endl;
}

bool Wood::predict(const int node, const instance &x) const {
  if (node <= 1)
    return node;
  return predict(child[x[feature[node]]][node], x);
}

void Wood::setFeature(const int node, const int f) {
  feature[node] = f;

  // cout << "free (" << node << "): " << available << endl;
}

void Wood::setChild(const int node, const int branch, const int orphan) {
  child[branch][node] = orphan;

  // cout << "free (" << node << "): " << available << endl;
}

std::ostream &operator<<(std::ostream &os, const TreeNode &x) {
  x.display(os);
  return os;
}

// TreeNode::TreeNode(Wood *w)  {
// 	wood = w;
//   idx = wood->size();
//   child_[0] = -1;
//   child_[1] = -1;
// }
//
// void TreeNode::free() {
//   wood->freeNode(idx);
//   for (auto i{0}; i < 2; ++i) {
//     assert(child_[i] != idx);
//     if (child_[i] >= 2)
//       (*wood)[child_[i]].free();
//   }
// }
//
// int TreeNode::copy() {
//   if (idx > 1) {
//
// 		cout << "copy " << idx << endl;
//
// 		cout << "wood->size() " << wood->size() << endl;
//
//     int root{wood->grow()};
//
// 		cout << "wood->size() " << wood->size() << endl;
//
// 		cout << "get " << root << endl;
//
//
// 		assert(root >= 0 and root < wood->size());
//
//     wood->setFeature(root, feature);
//     for (auto i{0}; i < 2; ++i)
//       wood->setChild(root, i, (*wood)[child_[i]].copy());
//     return root;
//   }
//   return idx;
// }
//
// // void TreeNode::setLeaf(const bool y) {
// // 	feature = y;
// // }
//
// const TreeNode &TreeNode::child(const bool t) const { return
// (*wood)[child_[t]]; }
//
// const TreeNode &TreeNode::next(const instance x) const {
//   return (*wood)[child_[x[feature]]];
// }
//
// bool TreeNode::isLeaf() const { return child_[0] < 0; }
//
// bool TreeNode::prediction() const { return feature; }
//
// void TreeNode::setChild(const bool branch, const int node_id) {
//   // cout << "set child[" << branch << "] of " << getIndex() << " is " <<
//   // node.getIndex() << endl;
//   child_[branch] = node_id; //.getIndex();
//   // cout << "(" << feature << "|" << child_[0] << "|" << child_[1] << ")\n";
// }
//
// // void TreeNode::setLeaf(const bool branch, const bool y) {
// //   child_[branch] = y;
// // }
//
// int TreeNode::getIndex() const { return idx; }
//
// TreeNode &TreeNode::getLeaf(const instance &x) const {
//   if (isLeaf())
//     return (*wood)[idx];
//   return next(x).getLeaf(x);
// }
//
// bool TreeNode::predict(const instance &x) const {
//   TreeNode &leaf{getLeaf(x)};
//   return leaf.prediction();
// }
//
// int TreeNode::predict(const DataSet &data) const {
//   auto error{0};
//   for (auto y{0}; y < 2; ++y)
//     for (auto i : data.example[y])
//       error += (predict(data[i]) != y);
//   return error;
// }
//
// std::ostream &TreeNode::display(std::ostream &os, const int depth) const {
//
//   // cout << "(" << feature << "|" << child_[0] << "|" << child_[1] << ") ";
//
//   if (depth > 5) {
//     cout << "cutting\n";
//     return os;
//   }
//
//   if (isLeaf())
//     os << "class-" << feature << endl;
//   else {
//     os
//         //<< " [" << idx << ":" << child_[0] << "|" << child_[1] << "] "
//         << feature << endl;
//
//     assert(child_[0] >= 0 and child_[1] >= 0);
//
//     for (auto i{0}; i < depth; ++i)
//       os << "    ";
//     os << " yes:";
//
//     assert(child(true).getIndex() != getIndex());
//
//     child(true).display(os, depth + 1);
//
//     for (auto i{0}; i < depth; ++i)
//       os << "    ";
//     os << " no:";
//
//     assert(child(false).getIndex() != getIndex());
//
//     child(false).display(os, depth + 1);
//   }
//   return os;
// }
//
// Wood::Wood() {
//   stock[grow()].feature = false;
//   stock[grow()].feature = true;
// }
//
// size_t Wood::size() { return stock.size(); }
//
// TreeNode &Wood::operator[](const int i) { return stock[i]; }
//
// const TreeNode &Wood::operator[](const int i) const { return stock[i]; }
//
// void Wood::resize(const int k) {
// 	cout << "resize " << k << " (" << size() << ") "  << this << endl;
//
//   available.reserve(k);
// 	stock.reserve(k);
// 	// for(auto i{stock.size()}; i<k; ++i)
//
//   while (size() < k) {
//     available.add(static_cast<int>(stock.size()));
//     TreeNode node(this);
//     stock.push_back(node);
//   }
//
// 	cout << this << endl;
// 	cout << size() << endl;
//
// 	if(k==19)
// 		exit(1);
//
// }
//
// //
// int Wood::grow() {
//
//
//
//   if (available.empty())
//     resize(stock.size() + 1);
//   auto node{*available.begin()};
//   available.remove_front(node);
//
// 	// cout << node << endl;
//
//   // cout << "grow (" << node << "): " << available << endl;
//
// 	cout << "return " << node << endl;
//
//   return node;
// }
//
// void Wood::freeNode(const int node) {
//   available.add(node);
//
//   // cout << "free (" << node << "): " << available << endl;
// }
//
// void Wood::setFeature(const int node, const int f) {
//   stock[node].feature = f;
//
//   // cout << "free (" << node << "): " << available << endl;
// }
//
// void Wood::setChild(const int node, const int branch, const int orphan) {
//   stock[node].setChild(branch,orphan);
//
//   // cout << "free (" << node << "): " << available << endl;
// }
//
// std::ostream &operator<<(std::ostream &os, const TreeNode &x) {
//   x.display(os,0);
//   return os;
// }
}
