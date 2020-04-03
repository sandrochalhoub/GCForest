
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

#ifdef DEBUG
  today = 0;
#endif

  for (auto i{0}; i < 2; ++i)
    grow();
}

size_t Wood::size() { return feature.size(); }

size_t Wood::count() { return size() - available.count(); }

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

#ifdef DEBUG
  birthday.resize(k, today);
#endif
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

#ifdef DEBUG
  birthday[node] = ++today;

// for(auto i{available.fbegin()}; i!=available.fend(); ++i)
// 	cout << " " << (today - birthday[*i]);
// cout << endl;
#endif

  return node;
}

int Wood::copyNode(const int node) {

  // cout << "copy " << node << endl;

  if (node > 1) {
    int root{grow()};
    feature[root] = feature[node];

    // cout << "root: " << root << endl;

    // for (auto i{0}; i < 2; ++i) {
    //
    //
    // 	cout << "child[i][root]: " << child[i][root] << endl;
    //
    // 	cout << "  - " << child[i][node] << endl
    // 		<< "  - " << child[i][node] << endl ;
    //
    // }

    for (auto i{0}; i < 2; ++i) {
      auto aux{copyNode(child[i][node])};

      // cout << " -> " << aux << endl;

      child[i][root] = aux;
    }
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

  // #ifdef DEBUG
  // 	++today;
  // #endif

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

}
