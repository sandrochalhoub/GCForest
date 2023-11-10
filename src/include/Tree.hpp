
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

  Tree<E_t> prune(const E_t *total, const E_t max_error, const bool terminal=true);
  Tree<E_t> prune(const E_t *total, const double max_loss);

  int getChild(const int node, const int branch) const;

  int getFeature(const int node) const;

  template <class sample> bool predict(const sample &i) const;
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

  const vector<const string>* feature_label{NULL};

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

  // prune from leaves until reaching a given maximum error
  Tree<E_t> prune_leaf(const int root, const E_t *total, const E_t max_error);
  // prune from anywhere until reaching a given maximum error
  Tree<E_t> prune_all(const int root, const E_t *total, const E_t max_error);
  // prune from anywhere until there is no node whose loss is above the given
  // limit
  Tree<E_t> prune_loss(const int root, const E_t *total, const double max_loss);

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

  void setFeatureLabels(const vector<const string>* fl);

  std::ostream &display(std::ostream &os, const int node,
                        const int p,
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
Tree<E_t> Tree<E_t>::prune(const E_t *total, const E_t max_error, const bool leaf) {
	if(leaf)
          return wood->prune_leaf(idx, total, max_error);
        else
          return wood->prune_all(idx, total, max_error);
}

template <class E_t>
Tree<E_t> Tree<E_t>::prune(const E_t *total, const double max_loss) {
  return wood->prune_loss(idx, total, max_loss);
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
  return wood->display(os, idx, -1, 0);
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
std::ostream &Wood<E_t>::display(std::ostream &os, const int node, const int p,
                                 const int depth) const {
  if (node <= 1)
    os << "class-" << node << " (" << static_cast<double>(getCount(p, node))/static_cast<double>(getCount(p, 1-node) + getCount(p, node))<< ")"<< endl;
  else {
   if(feature_label != NULL)
      os << (*feature_label)[feature[node]] << "?" << endl;
    else
      os << feature[node] << "?" << endl;

    assert(child[0][node] >= 0 and child[1][node] >= 0);

    for (auto i{0}; i < depth; ++i)
      os << "    ";
    os << " yes:";

    display(os, child[true][node], node, depth + 1);

    for (auto i{0}; i < depth; ++i)
      os << "    ";
    os << " no:";

    display(os, child[false][node], node, depth + 1);
  }

  return os;
}

 template <class E_t> void Wood<E_t>::setFeatureLabels(const vector<const string>* fl) {
  feature_label = fl;
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
    
    // cout << node << " (" << total[0] << "/" << total[1] << ")|("
    // 	<< getCount(node,0) << "/" << getCount(node,1) << ") " << feature[node]
    // 	<< ": " << child[0][node] << "=" << lc[0] << "/" << lc[1] << " <> "
    // 		<< child[1][node] << "=" << rc[0] << "/" << rc[1] << endl;
    
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
// }

template <class E_t>
Tree<E_t> Wood<E_t>::prune_leaf(const int root, const E_t *total,
                                const E_t max_error) {

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

  return (*this)[root];
}

template <class E_t>
Tree<E_t> Wood<E_t>::prune_all(const int root, const E_t *total,
                               const E_t max_error) {

  // cout << "prune\n" ;
  // this->display(cout, root, 0);

  // cout << endl;

  /* all of the following vectors hold info indexed by nodes*/

  // number of leaves under the node
  vector<size_t> num_leaf;
  num_leaf.resize(size(), 1);

  // error at the node
  vector<size_t> error;
  error.resize(size(), 0);

  // extra error if the node is removed
  vector<size_t> marginal;
  marginal.resize(size(), 0);

  // the label used if the node is removed
  vector<bool> mode;
  mode.resize(size(), false);

  // get a list of all nodes from root
  vector<int> nodes;
	get_descendants(root, nodes);

  // compute all of the info
  compute_size(root, num_leaf);
  compute_error(root, error, marginal, mode, total, true);
	
	
  // for (auto l : nodes) {
  //   cout << "node_" << l << " f=" << getFeature(l) << " s=" << (2 * num_leaf[l] - 1) << " e=" << error[l]
  //        << " m=" << marginal[l] << endl;

  //   assert(l>=0);
  //   assert(l<size());
  // }
  

  // the extra error so far
  E_t additional_error{0};
	
  bool empty{false};
	
	while(nodes.size() > 0) {

    // sort the nodes by non-decreasing ratio marginal/size of the subtree
	  sort(nodes.begin(), nodes.end(),
	       [&](const int x, const int y) { return marginal[x]*(2 * num_leaf[y] - 2) > marginal[y]*(2 * num_leaf[x] - 2); });


    // for(auto x : nodes) {
    //   if(marginal[x] + additional_error > max_error)
    //     cout << "keep " << x << endl;
    //   // else {
    //   //   cout << marginal[x] << " + " << additional_error << " <= " << max_error << " -> " << x << " is removable\n";
    //   // }
    // }


		
    // do not prune nodes whose marginal would make the additional error exceed the upper bound
		nodes.erase(std::remove_if(nodes.begin(), nodes.end(), [&](const int x) {return marginal[x] + additional_error > max_error;}), nodes.end());
		
		if(nodes.size() == 0)
			break;
		
    // prune the worst node (and all its descendants)
    auto x{nodes.back()};

    // cout << "remove " << x << endl;

    nodes.pop_back();

    if(x != root) {
      additional_error += marginal[x];
      auto p{parent[x]};
      auto self{child[1][p] == x};
      child[self][p] = mode[x];
  		marginal[p] -= marginal[x]; // not sure we need to update the marginal!!!
  		num_leaf[p] -= (num_leaf[x]-1);
    } else {
      empty = true;
    }
    
    freeNode(x);

		nodes.erase(std::remove_if(nodes.begin(), nodes.end(), [&](const int x) {return available.contain(x);}), nodes.end());
		
	}

        if (empty)
          return (*this)[mode[root]];
        return (*this)[root];
}

template <class E_t>
Tree<E_t> Wood<E_t>::prune_loss(const int root, const E_t *total,
                               const double max_loss) {

  // cout << "prune\n";
  // this->display(cout, root, 0);

  /* all of the following vectors hold info indexed by nodes*/

  // number of leaves under the node
  vector<size_t> num_leaf;
  num_leaf.resize(size(), 1);

  // error at the node
  vector<size_t> error;
  error.resize(size(), 0);

  // extra error if the node is removed
  vector<size_t> marginal;
  marginal.resize(size(), 0);

  // the label used if the node is removed
  vector<bool> mode;
  mode.resize(size(), false);

  // get a list of all nodes from root
  vector<int> nodes;
  get_descendants(root, nodes);

  // compute all of the info
  compute_size(root, num_leaf);
  compute_error(root, error, marginal, mode, total, true);

  // for (auto l : nodes) {
  //   // cout << "node_" << l << " f=" << getFeature(l)
  //   //      << " s=" << (num_leaf[l] - 1) << " e=" << error[l]
  //   //      << " m=" << marginal[l] << " l="
  //   //      << (static_cast<double>(marginal[l]) /
  //   //          (static_cast<double>(total[0] + total[1]) * (num_leaf[l] -
  //   1)))
  //   //      << endl;

  //   assert(num_leaf[l] >= 2);
  //   assert(l >= 0);
  //   assert(l < size());
  // }

  // cout << "size = " << size(root) << " / " << nodes.size() << endl;

  // the extra error so far
  double additional_loss{0};

  bool empty{false};

  while (nodes.size() > 0) {

    // sort the nodes by non-decreasing ratio marginal/size of the subtree
    sort(nodes.begin(), nodes.end(), [&](const int x, const int y) {
      return marginal[x] * (num_leaf[y] - 1) > marginal[y] * (num_leaf[x] - 1);
    });

    // if (nodes.size() == 0 or
    //     additional_loss + static_cast<double>(marginal[nodes.back()]) /
    //             (static_cast<double>(total[0] + total[1]) *
    //              static_cast<double>((num_leaf[nodes.back()] - 1))) >
    //         max_loss) {

    //   if (nodes.size() > 0)
    //     cout << "stop because loss[" << nodes.back() << "]="
    //          << (static_cast<double>(marginal[nodes.back()]) /
    //              (static_cast<double>(total[0] + total[1]) *
    //               static_cast<double>(num_leaf[nodes.back()] - 1)))
    //          << endl;
    //   break;
    // }

    if (nodes.size() == 0 or
        additional_loss + (static_cast<double>(marginal[nodes.back()]) /
                           (static_cast<double>(total[0] + total[1]))) >
            max_loss) {

      // if (nodes.size() > 0)
      //   cout << "stop because loss[" << nodes.back() << "]="
      //        << (static_cast<double>(marginal[nodes.back()]) /
      //            (static_cast<double>(total[0] + total[1])))
      //        << endl;
      break;
    }

    // prune the worst node (and all its descendants)
    auto x{nodes.back()};

    additional_loss +=
        (static_cast<double>(marginal[x]) /
         (static_cast<double>(
             total[0] +
             total[1]))); // * static_cast<double>(num_leaf[x] - 1)));

    // cout << "remove " << x << " (loss=" << additional_loss
    //      << ") num leaves=" << num_leaf[x] << endl;

    nodes.pop_back();

    if (x != root) {
      // additional_error += marginal[x];
      auto p{parent[x]};
      auto self{child[1][p] == x};
      child[self][p] = mode[x];
      marginal[p] -= marginal[x]; // not sure we need to update the marginal!!!
      num_leaf[p] -= (num_leaf[x] - 1);
    } else {
      empty = true;
    }

    freeNode(x);

    nodes.erase(
        std::remove_if(nodes.begin(), nodes.end(),
                       [&](const int x) { return available.contain(x); }),
        nodes.end());

    // cout << "size = " << size(root) << " / " << (2 * nodes.size() + 1) << endl;
  }

  if(empty)
    return (*this)[mode[root]];
  return (*this)[root];
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
  if (node > 1 and not available.contain(node)) {
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
