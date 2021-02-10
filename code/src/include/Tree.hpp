
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

template <class E_t> class Wood;

template <class E_t> class Tree {
private:
  Wood<E_t> *wood;

public:
  int idx;

  Tree() = default;

  Tree(Wood<E_t> *w, const int i);

  void prune(const E_t *total, const E_t max_error, const bool terminal=true);

  int getChild(const int node, const int branch) const;

  int getFeature(const int node) const;
	
	template<class sample>
	bool predict(const sample &i) const;
  //
  // bool predict(const instance &i) const;

        // template <class rIter, typename wf_type>
        // E_t predict(rIter beg_neg, rIter end_neg, rIter beg_pos, rIter
        // end_pos,
        //             wf_type weight_function) const;

        size_t size() const;

        size_t depth() const;

        /*!@name Miscellaneous*/
        //@{
        std::ostream &display(std::ostream &os) const;
        //@}
};

// M
template <class E_t> class Wood {

private:
  vector<int> parent;
  vector<int> child[2];
  vector<int> feature;

  vector<E_t> weight[2];

  void resize(const int k);

  void compute_size(const int node, vector<size_t> &num_leaf);

  // template<class E_t>
  void compute_error(const int node, vector<E_t> &error, vector<E_t> &maginal,
                     vector<bool> &mode, const E_t *total, const bool cumulative=false);

  void get_descendants(const int node, vector<int> &bag, const bool leaf=false);

public:
  SparseSet available;

  Wood();

  Tree<E_t> operator[](const int i);
  // const Tree &operator[](const int i) const;

  size_t size() const;

  size_t count() const;

  // void cut(const int node);

  void prune(const int root, const E_t *total, const E_t max_error);
	void prune2(const int root, const E_t *total, const E_t max_error);

  // allocate memory for a new node and returns its index
  int grow();

  int copyNode(const int node);

  // free the memory of node i
  void freeNode(const int i);

  void setFeature(const int node, const int f);

  int getFeature(const int node) const;

  void setCount(const int node, const int y, const size_t c);

  size_t getCount(const int node, const int y) const;

  void setChild(const int node, const int branch, const int orphan);

  int getChild(const int node, const int branch) const;

  bool isRoot(const int node) const;

  template <class sample> bool predict(const int node, const sample &i) const;
  // bool predict(const int node, const instance &x) const;

  size_t size(const int node) const;

  size_t depth(const int node) const;

  std::ostream &display(std::ostream &os, const int node,
                        const int depth = 0) const;

#ifdef DEBUG
  vector<int> birthday;
  int today;
#endif
};

template <class E_t>
std::ostream &operator<<(std::ostream &os, const Tree<E_t> &x);

template <class E_t>
template <class sample>
bool Tree<E_t>::predict(const sample &i) const {
  return wood->predict(idx, i);
}

template <class E_t>
Tree<E_t>::Tree(Wood<E_t> *w, const int node) : wood(w), idx(node) {}

template <class E_t>
void Tree<E_t>::prune(const E_t *total, const E_t max_error, const bool leaf) {
	if(leaf)
		wood->prune(idx, total, max_error);
	else
		wood->prune2(idx, total, max_error);
}

template <class E_t>
int Tree<E_t>::getChild(const int node, const int branch) const {
  return wood->getChild(node, branch);
}

template <class E_t> int Tree<E_t>::getFeature(const int node) const {
  return wood->getFeature(node);
}

// template<class sample>
// bool Tree<E_t>::predict(const sample &i) const {
//   return wood->predict(idx, i);
// }

template <class E_t> size_t Tree<E_t>::size() const { return wood->size(idx); }

template <class E_t> size_t Tree<E_t>::depth() const {
  return wood->depth(idx);
}

template <class E_t> std::ostream &Tree<E_t>::display(std::ostream &os) const {

  return wood->display(os, idx, 0);
}

template <class E_t>
template <class sample>
bool Wood<E_t>::predict(const int node, const sample &x) const {
  if (node <= 1)
    return node;
  return predict(child[x[feature[node]]][node], x);
}


// template<typename E_t, class rIter, typename wf_type>
// E_t Tree::predict(rIter beg_neg, rIter end_neg, rIter beg_pos, rIter end_pos, wf_type weight_function) const {
//   E_t error{0};
//   for (auto i{beg_neg}; i != end_neg; ++i)
//     error += weight_function(0, (i - beg_neg)) * (wood->predict(idx, *i) != 0);
//   for (auto i{beg_pos}; i != end_pos; ++i)
//     error += weight_function(1, (i - beg_pos)) * (wood->predict(idx, *i) != 1);
//   return error;
// }

template <class E_t>
std::ostream &Wood<E_t>::display(std::ostream &os, const int node,
                                 const int depth) const {

  if (node <= 1)
    os << "class-" << node << endl;
  else {
    os << node << (isRoot(node) ? "*" : "") << ": "
       // << " [" << getCount(node,0) << "/" << getCount(node,1) << "]:"
       << feature[node] << endl;

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

template <class E_t> Wood<E_t>::Wood() {

#ifdef DEBUG
  today = 0;
#endif

  for (auto i{0}; i < 2; ++i)
    grow();
}

template <class E_t> size_t Wood<E_t>::size() const { return feature.size(); }

template <class E_t> size_t Wood<E_t>::count() const {
  return size() - available.count();
}

template <class E_t> Tree<E_t> Wood<E_t>::operator[](const int i) {
  Tree<E_t> t(this, i);
  return t;
}

template <class E_t> void Wood<E_t>::resize(const int k) {
  available.reserve(k);
  for (auto i{feature.size()}; i < k; ++i)
    available.add(i);
  feature.resize(k, -1);
  parent.resize(k, -1);

  for (int i{0}; i < 2; ++i) {
    child[i].resize(k, -1);
    weight[i].resize(k, 0);
  }
// child[0].resize(k, -1);
// child[1].resize(k, -1);

#ifdef DEBUG
  birthday.resize(k, today);
#endif
}

// template<class E_t>
// void Wood<E_t>::cut(const int node) {
//
// }

template <class E_t>
void Wood<E_t>::compute_size(const int node, vector<size_t> &num_leaf) {
  if (node >= 2) {
    compute_size(child[0][node], num_leaf);
    compute_size(child[1][node], num_leaf);
    num_leaf[node] = num_leaf[child[0][node]] + num_leaf[child[1][node]];

    // cout << node << ": " << num_leaf[node] << endl;
  }
}

template <class E_t>
void Wood<E_t>::compute_error(const int node, vector<E_t> &error,
                              vector<E_t> &marginal, vector<bool> &mode,
                              const E_t *total, const bool cumulative) {
  if (node >= 2) {
    mode[node] = (total[0] < total[1]);
    error[node] = std::min(total[0], total[1]);
    marginal[node] = error[node];

    E_t rc[2] = {getCount(node, 0), getCount(node, 1)};
    E_t lc[2] = {total[0] - getCount(node, 0), total[1] - getCount(node, 1)};

    // // cout << node << " (" << total[0] << "/" << total[1] << ") " <<
    // // feature[node]
    // // 	<< ": " << child[0][node] << "=" << getCount(node,0) << "/" <<
    // // getCount(node,1) << " <> "
    // // 		<< child[0][node] << "=" << (total[0]-getCount(node,0)) << "/"
    // // <<
    // // (total[1]-getCount(node,1)) << endl;
    // //
    //
    // cout << node << " (" << total[0] << "/" << total[1] << ")|("
    // 	<< getCount(node,0) << "/" << getCount(node,1) << ") " << feature[node]
    // 	<< ": " << child[0][node] << "=" << lc[0] << "/" << lc[1] << " <> "
    // 		<< child[1][node] << "=" << rc[0] << "/" << rc[1] << endl;
    //
    // // assert(total[0] >= getCount(node,0));
    // // assert(total[1] >= getCount(node,1));

    if (child[0][node] >= 2) {
      compute_error(child[0][node], error, marginal, mode, lc, cumulative);
      marginal[node] -= error[child[0][node]];
			if(cumulative)
				marginal[node] += marginal[child[0][node]];
    } else {
      marginal[node] -= min(lc[0], lc[1]);
    }

    if (child[1][node] >= 2) {
      compute_error(child[1][node], error, marginal, mode, rc, cumulative);
      marginal[node] -= error[child[1][node]];
			if(cumulative)
				marginal[node] += marginal[child[1][node]];
    } else {
      marginal[node] -= min(rc[0], rc[1]);
    }
		
		// if(cumulative)
		// 	marginal[node] += (marginal[node] + marginal[node]);
  }
}

template <class E_t>
void Wood<E_t>::get_descendants(const int node, vector<int> &bag, const bool terminal) {
  if (node >= 2) {
    if (not terminal or (child[0][node] < 2 and child[1][node] < 2))
      bag.push_back(node);
    if (child[0][node] > 1)
      get_descendants(child[0][node], bag, terminal);
    if (child[1][node] > 1)
      get_descendants(child[1][node], bag, terminal);
  }
}

// template <class E_t>
// void Wood<E_t>::get_node(const int node, vector<int> &nodes) {
//   if (node >= 2) {
//     // if (child[0][node] < 2 and child[1][node] < 2)
//     nodes.push_back(node);
//     if (child[0][node] > 1)
//       get_leaf(child[0][node], nodes);
//     if (child[1][node] > 1)
//       get_leaf(child[1][node], nodes);
//   }
// }

template <class E_t>
void Wood<E_t>::prune(const int root, const E_t *total, const E_t max_error) {

  vector<size_t> num_leaf;
  num_leaf.resize(size(), 1);

  vector<size_t> error;
  error.resize(size(), 0);

  vector<size_t> marginal;
  marginal.resize(size(), 0);

  vector<bool> mode;
  mode.resize(size(), false);

  vector<int> leafs;

  compute_size(root, num_leaf);

  compute_error(root, error, marginal, mode, total);

  get_descendants(root, leafs, true);

  sort(leafs.begin(), leafs.end(),
       [&](const int x, const int y) { return marginal[x] > marginal[y]; });

  E_t additional_error{0};

  // cout << endl
  //      << additional_error << " + " << marginal[leafs.back()] << " < "
  //      << max_error << endl;
  // for (auto l : leafs) {
  //   cout << "leaf_" << l << " " << getFeature(l) << " " << num_leaf[l] << " " << error[l]
  //        << " " << marginal[l] << endl;
  // }
  while (leafs.size() > 0 and leafs.back() != root and
         additional_error + marginal[leafs.back()] <= max_error) {

    auto l{leafs.back()};
    leafs.pop_back();

		//     cout << "remove " << l << endl;
		// for(auto x : available)
		// cout << " " << x;
		// cout << endl;

    additional_error += marginal[l];

    auto p{parent[l]};

    // cout << "parent[" << l << "] = " << p << endl;

    auto self{child[1][p] == l};

    child[self][p] = mode[l];

    // cout << child[self][p] << " <- " << mode[l] << endl;

    freeNode(l);
		
		assert(child[0][l] < 2 and child[1][l] < 2);

    if (child[1 - self][p] <= 1) {

      // cout << "add parent " << p << " (b/c it's a leaf)\n";

      leafs.push_back(p);
      auto rp{leafs.end()};
      while (--rp != leafs.begin() and marginal[*rp] > marginal[*(rp - 1)])
        swap(*rp, *(rp - 1));
    }
    // else {
    // 	cout << "parent " << p << " is not a leaf: " << child[1-self][p] <<
    // "\n";
    // }
		
		// for(auto x : available)
		// cout << " " << x;
		// cout << endl;
		//
		//     cout << additional_error << " + " << marginal[leafs.back()] << " < "
		//          << max_error << endl;
		//     for (auto l : leafs) {
		//       cout << "leaf_" << l << " " << getFeature(l) << " " << num_leaf[l] << " " << error[l]
		//            << " " << marginal[l] << endl;
		//     }
		//     display(cout, root, 0);
		//     cout << endl;
  }
}

template <class E_t>
void Wood<E_t>::prune2(const int root, const E_t *total, const E_t max_error) {

  vector<size_t> num_leaf;
  num_leaf.resize(size(), 1);

  vector<size_t> error;
  error.resize(size(), 0);

  vector<size_t> marginal;
  marginal.resize(size(), 0);

  vector<bool> mode;
  mode.resize(size(), false);

  vector<int> nodes;
	get_descendants(root, nodes);

  compute_size(root, num_leaf);

  compute_error(root, error, marginal, mode, total, true);
	
	
  // for (auto l : nodes) {
  //   cout << "node_" << l << " f=" << getFeature(l) << " s=" << (2 * num_leaf[l] - 1) << " e=" << error[l]
  //        << " m=" << marginal[l] << endl;
  // }
  //

  E_t additional_error{0};
	
	
	while(nodes.size() > 0) {
	  sort(nodes.begin(), nodes.end(),
	       [&](const int x, const int y) { return marginal[x]*(2 * num_leaf[y] - 2) > marginal[y]*(2 * num_leaf[x] - 2); });
		
		nodes.erase(std::remove_if(nodes.begin(), nodes.end(), [&](const int x) {return marginal[x] + additional_error > max_error;}), nodes.end());
		
		if(nodes.size() == 0)
			break;
		
    auto x{nodes.back()};
    nodes.pop_back();
    additional_error += marginal[x];
    auto p{parent[x]};
    auto self{child[1][p] == x};
    child[self][p] = mode[x];
		marginal[p] -= marginal[x];
		num_leaf[p] -= (num_leaf[x]-1);
    freeNode(x);

		nodes.erase(std::remove_if(nodes.begin(), nodes.end(), [&](const int x) {return available.contain(x);}), nodes.end());
		
	}
	
	
	
	
	
	//   sort(nodes.begin(), nodes.end(),
	//        [&](const int x, const int y) { return marginal[x]*(2 * num_leaf[y] - 2) > marginal[y]*(2 * num_leaf[x] - 2); });
	//
	//
	//   cout << endl
	//        << additional_error << " + " << marginal[nodes.back()] << " < "
	//        << max_error << endl;
	//   for (auto l : nodes) {
	// 	double r = static_cast<double>(marginal[l]) / static_cast<double>(2 * num_leaf[l] - 2);
	//     cout << "node_" << l << " f=" << getFeature(l) << " s=" << (2 * num_leaf[l] - 2) << " e=" << error[l]
	//          << " m=" << marginal[l] << " r=" << r << endl;
	//   }
	//   // for (auto l : leafs) {
	//   //   cout << "leaf_" << l << " " << getFeature(l) << " " << num_leaf[l] << " " << error[l]
	//   //        << " " << marginal[l] << endl;
	//   // }
	//
	// nodes.erase(std::remove_if(nodes.begin(), nodes.end(), [&](const int x) {return marginal[x] + additional_error > max_error;}), nodes.end());
	//
	//
	//   while (nodes.size() > 0 and nodes.back() != root and
	//          additional_error + marginal[nodes.back()] <= max_error) {
	//
	//     auto x{nodes.back()};
	//     nodes.pop_back();
	//
	//   	cout << endl << "remove " << x << endl;
	//   		// for(auto x : available)
	//   		// cout << " " << x;
	//   		// cout << endl;
	//
	// 	cout << endl;
	//   for (auto l : nodes) {
	// 		double r = static_cast<double>(marginal[l]) / static_cast<double>(2 * num_leaf[l] - 2);
	//     cout << "node_" << l << " f=" << getFeature(l) << " s=" << (2 * num_leaf[l] - 2) << " e=" << error[l]
	//          << " m=" << marginal[l] << " r=" << r << endl;
	//   }
	//
	//     additional_error += marginal[x];
	//
	//     auto p{parent[x]};
	//
	//     // cout << "parent[" << l << "] = " << p << endl;
	//
	//     auto self{child[1][p] == x};
	//
	//     child[self][p] = mode[x];
	//
	//     // cout << child[self][p] << " <- " << mode[l] << endl;
	//
	// 	marginal[p] -= marginal[x];
	// 	num_leaf[p] -= (num_leaf[x]-1);
	//
	//     freeNode(x);
	//
	//
	//
	//   		// assert(child[0][x] < 2 and child[1][x] < 2);
	//
	//
	// 		// remove x's children
	// 		// for(auto n{nodes.begin()}; n!=nodes.end())
	//
	//
	//
	//
	//
	//
	//     // if (child[1 - self][p] <= 1) {
	//     //
	//     //   // cout << "add parent " << p << " (b/c it's a leaf)\n";
	//     //
	//     //   leafs.push_back(p);
	//     //   auto rp{leafs.end()};
	//     //   while (--rp != leafs.begin() and marginal[*rp] > marginal[*(rp - 1)])
	//     //     swap(*rp, *(rp - 1));
	//     // }
	//     // else {
	//     // 	cout << "parent " << p << " is not a leaf: " << child[1-self][p] <<
	//     // "\n";
	//     // }
	//
	//   		// for(auto x : available)
	//   		// cout << " " << x;
	//   		// cout << endl;
	//   		//
	//   		    cout << additional_error << " + " << marginal[nodes.back()] << " < "
	//   		         << max_error << endl;
	//   		//     for (auto l : leafs) {
	//   		//       cout << "leaf_" << l << " " << getFeature(l) << " " << num_leaf[l] << " " << error[l]
	//   		//            << " " << marginal[l] << endl;
	//   		//     }
	//   		//     display(cout, root, 0);
	//   		//     cout << endl;
	//
	//   for (auto l : nodes) {
	// 		double r = static_cast<double>(marginal[l]) / static_cast<double>(2 * num_leaf[l] - 2);
	//     cout << "node_" << l << " f=" << getFeature(l) << " s=" << (2 * num_leaf[l] - 2) << " e=" << error[l]
	//          << " m=" << marginal[l] << " r=" << r << endl;
	//   }
	//
	// 	nodes.erase(std::remove_if(nodes.begin(), nodes.end(), [&](const int x) {return available.contain(x);}), nodes.end());
	// 	// nodes.erase(nodes.resize()
	//
	// 	nodes.erase(std::remove_if(nodes.begin(), nodes.end(), [&](const int x) {return marginal[x] + additional_error > max_error;}), nodes.end());
	//
	//
	//
	//
	// 	cout << endl;
	//   for (auto l : nodes) {
	// 		double r = static_cast<double>(marginal[l]) / static_cast<double>(2 * num_leaf[l] - 2);
	//     cout << "node_" << l << " f=" << getFeature(l) << " s=" << (2 * num_leaf[l] - 2) << " e=" << error[l]
	//          << " m=" << marginal[l] << " r=" << r << endl;
	//   }
	//
	//   }
}

template <class E_t> int Wood<E_t>::grow() {
  if (available.empty())
    resize(feature.size() + 1);
  auto node{*available.begin()};
  available.remove_front(node);
  parent[node] = -1;
  weight[0][node] = 0;
  weight[1][node] = 0;

#ifdef DEBUG
  birthday[node] = ++today;
#endif

  // cout << "grow " << node << endl;

  return node;
}

template <class E_t> int Wood<E_t>::copyNode(const int node) {
  if (node > 1) {
    int root{grow()};
    feature[root] = feature[node];

    for (auto i{0}; i < 2; ++i) {
      auto aux{copyNode(child[i][node])};
      child[i][root] = aux;
      weight[i][root] = weight[i][node];
      parent[aux] = root;
    }
    return root;
  }
  return node;
}

template <class E_t> void Wood<E_t>::freeNode(const int node) {
  if (node > 1) {

    // cout << "free " << node << endl;
		
		if(node >= available.size() or available.contain(node)) {
			cout << "REMOVE " << node << " FROM " << available << endl;
		}

    assert(node < available.size() and not available.contain(node));

    // parent[node] = -1
    available.add(node);
    for (auto i{0}; i < 2; ++i)
      if (child[i][node] >= 2)
        freeNode(child[i][node]);
  }
}

// template<class sample>
// bool Wood<E_t>::predict(const int node, const sample &x) const {
//   if (node <= 1)
//     return node;
//   return predict(child[x[feature[node]]][node], x);
// }

template <class E_t> size_t Wood<E_t>::size(const int node) const {
  if (node <= 1)
    return 1;
  else
    return 1 + size(child[0][node]) + size(child[1][node]);
}

template <class E_t> size_t Wood<E_t>::depth(const int node) const {
  if (node <= 1)
    return 0;
  else
    return 1 + max(depth(child[0][node]), depth(child[1][node]));
}

template <class E_t> void Wood<E_t>::setFeature(const int node, const int f) {
  feature[node] = f;
}

template <class E_t> int Wood<E_t>::getFeature(const int node) const {
  return feature[node];
}

template <class E_t>
void Wood<E_t>::setCount(const int node, const int y, const size_t c) {
  weight[y][node] = c;
}

template <class E_t>
size_t Wood<E_t>::getCount(const int node, const int y) const {
  return weight[y][node];
}

template <class E_t>
void Wood<E_t>::setChild(const int node, const int branch, const int orphan) {
  child[branch][node] = orphan;
  parent[orphan] = node;
}

template <class E_t>
int Wood<E_t>::getChild(const int node, const int branch) const {
  return child[branch][node];
}

template <class E_t> bool Wood<E_t>::isRoot(const int node) const {
  return parent[node] == -1;
}

template <class E_t>
std::ostream &operator<<(std::ostream &os, const Tree<E_t> &x) {
  x.display(os);
  return os;
}
}


#endif // __TREE_HPP
