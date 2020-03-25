
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
    os << " [" << idx << "] " << feature << endl;
    for (auto i{0}; i < depth; ++i)
      os << "  ";
    os << " yes:";

    assert(child(true).getIndex() != getIndex());

    child(true).display(os, depth + 1);
    // os << endl;
    for (auto i{0}; i < depth; ++i)
      os << "  ";
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

  cout << "grow (" << node << "): " << available << endl;

  return node;
}

void Wood::freeNode(const int node) {
  available.add(node);

  cout << "free (" << node << "): " << available << endl;
}

Tree::Tree() {}

int Tree::getFeature(const int node) const { return feature[node]; }

void Tree::addNode(const int c_true, const int c_false, const int f) {
  // left_child.push_back(c);
  child[0].push_back(c_false);
  child[1].push_back(c_true);
  feature.push_back(f);
}

void Tree::addNode(const int node, const int c_true, const int c_false,
                   const int f) {

  if (node >= feature.size()) {
    child[0].resize(node + 1);
    child[1].resize(node + 1);
    feature.resize(node + 1);
  }

  child[0][node] = c_false;
  child[1][node] = c_true;
  feature[node] = f;
}

int Tree::getChild(const int x, const bool t) const {

  // assert(left_child[x]+1-t == child[t][x]);

  return child[t][x]; // left_child[x]+1-t;
}

bool Tree::predict(const instance& x) const {

  // // cout << x << endl << "0";
  //
  //
  //
  //
  // int node{0};
  // while(feature[node] >= 0) {
  //
  // 	// cout << " (" << (x[feature[node]] ? feature[node] : -feature[node]) ;
  //
  // 	node = child(node, x[feature[node]]);
  // 	// cout << ") -> " << node;
  // }
  // // cout << " => " << feature[node] << endl;
  return feature[getLeaf(x)] == POSITIVE;
}

int Tree::getLeaf(const instance& x) const {

  // cout << "0";

  int limit{static_cast<int>(feature.size())};

  int node{0};
  while (feature[node] >= 0) {

    if (limit-- < 0) {
      cout << "tree: inf loop\n";
      exit(1);
    }

    // cout << " (" << (x[feature[node]] ? feature[node] : -feature[node]) ;

    node = getChild(node, x[feature[node]]);
    // cout << ") -> " << node;
  }
  // cout << " => " << feature[node] << endl;
  return node;
}

int Tree::predict(const DataSet& data) const {

  // cout << "Tree::predict\n";

  // vector<int> count[2];
  //         count[0].resize(feature.size(), 0);
  //         count[1].resize(feature.size(), 0);

  instance used_feature;
  used_feature.resize(data.numFeature(), 0);

  for (auto i{0}; i < feature.size(); ++i)
    if (feature[i] >= 0) {

      // cout << feature[i] << "/" << 	used_feature.size() << endl;

      used_feature.set(feature[i]);
    }
  // cout << used_feature << endl;

  auto error{0};
  for (auto y{0}; y < 2; ++y) {
    for (auto i : data.example[y]) {

      // for(auto f{0}; f<data.numFeature(); ++f)
      // 	if(used_feature[f])
      // 		cout << " " << (data[i][f] ? f : -f) ;
      // cout << endl;

      auto p{predict(data[i])};

      // ++count[p][getLeaf(data[i])];

      // cout << p;

      if (p != y)
        ++error;
    }
    // cout << endl;
  }

  // for (auto i{0}; i<left_child.size(); ++i)
  // 	if(feature[i] < 0) {
  //
  // 		cout << i << ": " << count[0][i] << "/" << count[1][i] << endl;
  //
  // 	}

  return error;
}


std::ostream &Tree::display(std::ostream &os) const {

  // os << "NODES\n";
  for (auto i{0}; i < feature.size(); ++i) {
    os << i << " " << feature[i];
    if (feature[i] >= 0)
      os << " " << getChild(i, true) << " " << getChild(i, false);
    os << endl;
  }
  // os << "EDGES\n";
  // for (auto i{0}; i<parent.size(); ++i) {
  //
  // }

  return os;
}

std::ostream &operator<<(std::ostream &os, const Tree &x) {
  x.display(os);
  return os;
}

std::ostream &operator<<(std::ostream &os, const TreeNode &x) {
  x.display(os,0);
  return os;
}


}
