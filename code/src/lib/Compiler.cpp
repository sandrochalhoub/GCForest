
#include "Compiler.hpp"

namespace blossom {


template <typename E_t> Compiler<E_t>::Compiler(DTOptions &opt) : options(opt) {
  num_feature = 0;
  num_leaf = 0;
  search_size = 0;

  min_depth_backtrack = INFTY(int);
}

int lbLeaf2(const uint64_t P, const int E) {
  assert(P >= 0 and E >= 0);

  // base cases are P==0, P==2^E and P==2^(E-1):
  if (P == 0)
    return 1;

  if (E + 1 < 8 * sizeof(int)) {
    auto H{(1 << (E - 1))};

    if (P == 2 * H)
      return 1;
    if (P == H)
      return 2;
    if (P > H)
      return lbLeaf2(P - H, E - 1) + 1;
  }

  auto NE{log2_64(P) + 1};

  // E is large enough, the best is to put everything in P
  return lbLeaf2(P, NE) + E - NE;
}

template <typename E_t> void Compiler<E_t>::count_by_example(const int node) {

  pos_feature_frequency[node].clear();
  pos_feature_frequency[node].resize(num_feature, 0);

  for (auto i : P[node]) {
    for (auto f : example[i]) {
      ++pos_feature_frequency[node][f];
    }
  }
}

template <typename E_t>
void Compiler<E_t>::deduce_from_sibling(const int parent, const int node,
                                        const int sibling) {
  for (auto f{0}; f < num_feature; ++f)
    pos_feature_frequency[node][f] =
        pos_feature_frequency[parent][f] - pos_feature_frequency[sibling][f];
}

template <typename E_t>
E_t Compiler<E_t>::get_feature_frequency(const int n, const int f) const {
  return (f >= num_feature
              ? P[n].count() - pos_feature_frequency[n][f - num_feature]
              : pos_feature_frequency[n][f]);
}

template <typename E_t>
E_t Compiler<E_t>::get_feature_error(const int n, const int f) const {
  return std::min(static_cast<int>(P[n].count()) - pos_feature_frequency[n][f],
                  pos_feature_frequency[n][f]);
}

template <typename E_t> E_t Compiler<E_t>::node_error(const int node) const {
  return P[node].count();
}

template <typename E_t> size_t Compiler<E_t>::numExample() const {
  return example.size();
}

template <typename E_t> int Compiler<E_t>::numFeature() const {
  return num_feature;
}

template <typename E_t> void Compiler<E_t>::setReverse() {

  reverse_dataset.resize(num_feature);
  for (int f{0}; f < num_feature; ++f)
    reverse_dataset[f].resize(example.size(), 0);

  for (auto i{0}; i < example.size(); ++i)
    for (auto f : example[i])
      reverse_dataset[f].set(i);
}

template <typename E_t> size_t Compiler<E_t>::size() { return blossom.size(); }

template <typename E_t> bool Compiler<E_t>::no_feature(const int node) const {
  return feature[node] == end_feature[node];
}

template <typename E_t> void Compiler<E_t>::separator(const string &msg) const {
  cout << setfill('-') << setw((54 - msg.size()) / 2) << "-"
       << "[" << msg << "]" << setw((54 - msg.size()) / 2 + (msg.size() % 2))
       << "-" << endl
       << setfill(' ');
}

template <typename E_t> void Compiler<E_t>::print_new_best() {
  double t{fixedwidthfloat(cpu_time() - start_time, 3)};
  cout << " size = " << left << setw(5) << ub_size() << " choices = " << setw(9)
       << search_size << " depth = " << setw(3) << min_depth_backtrack
       << " mem = " << setw(4) << wood.size() << " time = " << t << right
       << endl;
}

template <typename E_t> void Compiler<E_t>::resize(const int k) {

  feature.resize(k);
  end_feature.resize(k);
  blossom.reserve(k);
  depth.resize(k, 0);
  parent.resize(k, -1);
  child[0].resize(k, -1);
  child[1].resize(k, -1);

  best.resize(k, INFTY(int));
  lb.resize(k, 1);
  tree.resize(k, -1);
  saved_tree.resize(k, -1);

  auto i{ranked_feature.size()};

  ranked_feature.resize(k);

  i = pos_feature_frequency.size();
  pos_feature_frequency.resize(k);
  while (i < pos_feature_frequency.size()) {
    pos_feature_frequency[i++].resize(num_feature);
  }

  while (P.size() < k)
    P.addNode();
}

template <typename E_t> int Compiler<E_t>::highest_error_reduction() const {

  auto selected_node{-1};
  auto highest_error{0};

  auto highest_reduction{0};

  for (auto i : blossom) {

    auto err{node_error(i)};
    auto reduction{err - get_feature_error(i, *(feature[i]))};

    if (reduction > highest_reduction or
        (reduction == highest_reduction and err > highest_error)) {
      highest_reduction = reduction;
      highest_error = err;
      selected_node = i;
    }
  }

  return selected_node;
}

template <typename E_t> int Compiler<E_t>::highest_error() const {

  auto selected_node{-1};
  auto highest_error{0};

  for (auto i : blossom) {
    auto err{node_error(i)};

    if (err > highest_error) {
      highest_error = err;
      selected_node = i;
    }
  }

  return selected_node;
}

template <typename E_t> void Compiler<E_t>::sort_features(const int node) {

  for (auto f{feature[node]}; f != end_feature[node]; ++f)
    f_error[*f] = get_feature_error(node, *f);
  sort(feature[node], end_feature[node],
       [&](const int a, const int b) { return (f_error[a] < f_error[b]); });

}

template <typename E_t> void Compiler<E_t>::prune(const int node) {

  if (node < 0)
    --num_leaf;
  else {
    if (not blossom.contain(node)) {
      if (best[node] < INFTY(int))
        num_leaf -= best[node];
      // since it was in the "used" pool, add it to move it to the "free" pool
      blossom.add(node);
    }
    blossom.remove_back(node);
    wood.freeNode(tree[node]);
  }
}

// template <typename E_t> void Compiler<E_t>::propagateDown(const int node) {
//   auto bl{numLeaf(child[0][node])};
//   auto br{numLeaf(child[1][node])};
//
//   if (bl < INFTY(int) and br < INFTY(int)) {
//     auto new_best{bl + br};
//     if (new_best < best[node]) {
//       best[node] = new_best;
//       if (node == 0 and options.verbosity > DTOptions::QUIET) {
//         if (decision.size() > 0)
//           min_depth_backtrack =
//               std::min(min_depth_backtrack, depth[decision.back()]);
//         print_new_best();
//       }
//
//       if (node)
//         propagateDown(parent[node]);
//     }
//   }
// }

template <typename E_t> void Compiler<E_t>::storeSolution() {

if (solution_root > 1) {
  wood.freeNode(solution_root);
}

solution_root = buildTree(0);
}

template <typename E_t> int Compiler<E_t>::buildTree(const int node) {

	// assert(node >= 0);

	int root = (node == -1);
	
	if(node >= 0) {
	  if (tree[node] >= 0) {
	    root = wood.copyNode(tree[node]);
	  } else {
	    root = wood.grow();

	    wood.setFeature(root, *feature[node]);
	    for (int i{0}; i < 2; ++i) {
	      auto c{buildTree(child[i][node])};
	      wood.setChild(root, i, c);
	    }
	  }
	}
	
	return root;
}

template <typename E_t> void Compiler<E_t>::updateTree(const int node) {
  // this is a terminal node, free current tree if it exists and grow a
  // new tree
  if (tree[node] > 1) {
    // cout << endl << " free tree[" << node << "]\n";
    // wood.display(cout, tree[node]);
    wood.freeNode(tree[node]);
  }
  tree[node] = wood.grow();

  // new best tree
  wood.setFeature(tree[node], *feature[node]);
  for (auto i{0}; i < 2; ++i) {
    if (child[i][node] < 0)
      wood.setChild(tree[node], i, child[i][node] == -1);
    else {
      wood.setChild(tree[node], i, tree[child[i][node]]);
      tree[child[i][node]] = -1;
    }
  }

  // //
  // cout << endl << " save tree[" << node << "]\n";
  // wood.display(cout, tree[node], depth[node]);
}

template <typename E_t>
void Compiler<E_t>::updateBest(const int node, const bool terminal) {

  // cout << "update(" << node << ")\n";

  auto bl{numLeaf(child[0][node])};
  auto br{numLeaf(child[1][node])};

  if (bl < INFTY(int) and br < INFTY(int)) {
    auto new_best{bl + br};
    if (new_best < best[node]) {
      best[node] = new_best;

      if (terminal) {
        updateTree(node);
      }

      if (node == 0 and options.verbosity > DTOptions::QUIET) {
        if (decision.size() > 0)
          min_depth_backtrack =
              std::min(min_depth_backtrack, depth[decision.back()]);

        print_new_best();

        storeSolution();
				
				// wood.display(cout, solution_root);
      }

      if (node)
        updateBest(parent[node], false);
    }
  }
}

template <typename E_t> bool Compiler<E_t>::fail() {
  if (decision.empty())
    return lb[0] >= best[0];
  auto node{decision.back()};

  auto LB{minLeaf(child[0][node]) + minLeaf(child[1][node])};
	
	// cout << LB << " (" << node << ")" << endl;

  while (node > 0) {
    auto c{node};
    node = parent[node];

    LB += minLeaf(child[child[0][node] == c][node]);
		
		// cout << LB << " / " << best[node] << " (" << node << ")" << endl;

    if (LB > best[node] or (node == 0 and LB >= best[node])) {
			
			// cout << "FAIL!\n" ;
			
      return true;
    }
  }

  return false;
}

template <typename E_t>
bool Compiler<E_t>::is_optimal(const int node, const int f) const {
  auto count{get_feature_frequency(node, f)};
  return count == 0 or count == P[node].count();
}

template <typename E_t> bool Compiler<E_t>::backtrack() {
  bool dead_end{false};

  do {

    if (decision.empty())
      return false;

    auto node{decision.back()};
    decision.pop_back();

    // cout << "bkt\n";
    // updateBest(node);

    if (depth[node] < min_depth_backtrack) {
      min_depth_backtrack = depth[node];
    }

#ifdef PRINTTRACE
    if (PRINTTRACE) {
      cout << setw(3) << decision.size();
      for (auto i{0}; i < decision.size(); ++i)
        cout << "   ";
      cout << "backtrack on " << node << " = " << *feature[node] << " ("
           << (end_feature[node] - feature[node]) << ")" << endl;
    }
#endif

    assert(not no_feature(node));

    for (auto i{0}; i < 2; ++i)
      prune(child[i][node]);

    dead_end = (is_optimal(node, *feature[node]++) or no_feature(node) or
                best[node] == lb[node]);

    if (not dead_end) {
      // branch on the next feature
      blossom.add(node);
    } else {
      // this node is optimal (either from lb reasoning or because the search
      // space was exhausted). it remains in the tree as long as its parent's
      // test has not changed
      if (best[node] < INFTY(int)) {
        num_leaf += best[node];
        lb[node] = best[node];
      }
    }

  } while (dead_end);
	
	
	// exit(1);

  return true;
}

template <typename E_t> bool Compiler<E_t>::purePositive(const int node) const {
  return (log_size(node) < 8 * sizeof(size_t) and
          P[node].count() == usize(node));
}

template <typename E_t>
bool Compiler<E_t>::setChild(const int node, const bool branch, const int c) {

  parent[c] = node;
  depth[c] = depth[node] + 1;

  if (P[c].count() == 0) {
    child[branch][node] = -1;
  } else if (smallEnough(c) and purePositive(c)) {
    child[branch][node] = -2;
  } else {
    child[branch][node] = c;
    return true;
  }
  return false;
}

template <typename E_t>
void Compiler<E_t>::branch(const int node, const int f) {

#ifdef PRINTTRACE
  if (PRINTTRACE) {

    assert(*feature[node] == f);

    cout << setw(3) << decision.size();
    for (auto i{0}; i < decision.size(); ++i)
      cout << "   ";
    cout << "branch on " << node << " (" << P[node].count() << "/2^"
         << log_size(node) << "-" << P[node].count() << ") with " << f << " ("
         << (end_feature[node] - feature[node]) << ")";
  }
  cout.flush();
#endif

  decision.push_back(node);
  blossom.remove_front(node);

  // we create two nodes even if one branch is pure, but we'll free it
  if (blossom.capacity() < blossom.size() + 2)
    resize(blossom.size() + 2);

  int c[2] = {*blossom.bbegin(), *(blossom.bbegin() + 1)};

  branch_features.resize(num_feature);
  branch_features.reset();

  auto p = node;

  while (p > 0) {
    p = parent[p];
    branch_features.set(*feature[p]);
  } // while(p > 0);

  // cout << endl << branch_features << endl;
  // for (int f{0}; f < num_feature; ++f)
  // 	cout << (f%10);
  // cout << endl;
  //   for (auto x : P[node]) {
  //     for (int f{0}; f < num_feature; ++f)
  // 		if(not branch_features[f])
  // 			cout << reverse_dataset[f][x];
  // 		else
  // 			cout << " ";
  //     cout << endl;
  //   }
  //   cout << endl;

  P.branch(node, c[0], c[1],
           [&](const int x) { return reverse_dataset[f][x]; });

  // for (auto i{0}; i < 2; ++i) {
  //   cout << 1-i << endl;
  // 		for (int f{0}; f < num_feature; ++f)
  // 			cout << (f%10);
  // 		cout << endl;
  //   for (auto x : P[c[i]]) {
  //     for (int f{0}; f < num_feature; ++f)
  // 				if(not branch_features[f])
  // 					cout << reverse_dataset[f][x];
  // 				else
  // 					cout << " ";
  //     cout << endl;
  //   }
  //   cout << endl;
  // }

  auto smallest{P[c[1]].count() < P[c[0]].count()};

  count_by_example(c[smallest]);

  deduce_from_sibling(node, c[1 - smallest], c[smallest]);
	
	
	assert(P[c[0]].count() + P[c[1]].count() == P[node].count());

  bool pseudo_leaf{true};
  for (auto i{0}; i < 2; ++i)
    if (setChild(node, i, c[i]))
      pseudo_leaf &= grow(c[i]);
    else
      ++num_leaf;

#ifdef PRINTTRACE
  if (PRINTTRACE) {
    // cout << setw(3) << decision.size()-1;
    // for (auto i{0}; i < decision.size()-1; ++i)
    //   cout << "   ";
    // cout << "branch on " << node << " (" << P[node].count() << "/"
    //      << (usize(node) - P[node].count()) << ") with " << f
    cout << " children: " << c[0] << " (" << P[c[0]].count() << "/2^"
         << log_size(c[0]) << "-" << P[c[0]].count() << ") and " << c[1] << " ("
         << P[c[1]].count() << "/2^" << log_size(c[1]) << "-" << P[c[1]].count()
         << ")" << endl;
  }
#endif

  if (pseudo_leaf) {
    // cout << "psd\n";
    updateBest(node);
  }

  // for(auto i{0}; i < 2; ++i)
  // 	if(child[i][node]>=0)
  // 		updateBest(child[i][node]);
}

// returns true if this is a pseudo-leaf
template <typename E_t> bool Compiler<E_t>::grow(const int node) {

  if (node == 0) {

    assert(ranked_feature[0].size() == 0);

    for (auto f{0}; f < numFeature(); ++f)
      ranked_feature[0].push_back(f);
  } else {
    ranked_feature[node].clear();

    for (auto f : ranked_feature[parent[node]]) {
      if (f != *feature[parent[node]])
        ranked_feature[node].push_back(f);
      feature[node] = ranked_feature[node].begin();
    }
  }

  feature[node] = ranked_feature[node].begin();
  end_feature[node] = ranked_feature[node].end();


	vector<bool> positive(numFeature(),false);
	vector<bool> negative(numFeature(),false);
	if(ranked_feature[node].empty()) {
		auto n_i{node};
		
		while(n_i) {
			
			auto p_i{parent[n_i]};
			auto dir{child[0][p_i]==n_i};
			auto s_i = child[dir][p_i];
			
			if(dir) {
				positive[*feature[p_i]] = true;
			} else {
				negative[*feature[p_i]] = true;
			}
				
			
			cout << n_i << ": " << P[n_i].count() ;
			
			if(s_i >= 0)
				cout << "/ " << s_i << ": " << P[s_i].count() ;
			else
				cout << "/ x" ;
			
			cout << endl;
			

			
			
			n_i = p_i;
		}
		
		for(auto f{0}; f<numFeature(); ++f) {
			if(positive[f]) {
				assert(not negative[f]);
				cout << "+";
			} else if(negative[f]) {
				cout << "-";
			} else {
				cout << "?";
			}
		}
		cout << endl;
		
		// if(P[node].count() < 4) {
			for(auto x : P[node]) {
				for(auto f{0}; f<numFeature(); ++f)
					cout << reverse_dataset[f][x] ; 
				cout << endl;
			}
		// }
		
	}


  assert(not ranked_feature[node].empty());

  sort_features(node);

  blossom.add(node);

  best[node] = INFTY(int);

  lb[node] = lbLeaf2(P[node].count(), num_feature - depth[node]);

  tree[node] = -1;
  saved_tree[node] = -1;

  // cout << "\ngrow " << node << " " << (num_feature - depth[node] - 1) << " < " << std::numeric_limits<E_t>::digits << endl;
  if (smallEnough(node) and P[node].count() == halfsize(node)) {
    int f[2] = {*feature[node] + numFeature(), *feature[node]};
    for (auto i{0}; i < 2; ++i) {
      if (get_feature_frequency(node, f[i]) == P[node].count()) {

        child[i][node] = -1;
        child[1 - i][node] = -2;
        blossom.remove_front(node);
        best[node] = 2;
        num_leaf += 2;

        saved_tree[node] = tree[node] = wood.grow();
        wood.setFeature(tree[node], *feature[node]);
        wood.setChild(tree[node], i, 1);
        wood.setChild(tree[node], 1 - i, 0);

        // cout << endl << " new tree[" << node << "]\n";
        // wood.display(cout, tree[node], depth[node]);
        // // cout << " - tree = " << tree[node] << endl;

        return true;
      }
    }
  }

  return false;
}

template <typename E_t> void Compiler<E_t>::expend() {

  auto selected_node{-1};

  switch (options.node_strategy) {
  case DTOptions::FIRST:
    selected_node = *blossom.begin();
    break;
  // case DTOptions::RANDOM:
  //   selected_node = *(blossom.begin() + (random_generator() %
  //   blossom.count()));
  //   break;
  case DTOptions::ERROR:
    selected_node = highest_error();
    break;
  case DTOptions::ERROR_REDUCTION:
    selected_node = highest_error_reduction();
    break;
  }

  branch(selected_node, *(feature[selected_node]));
}

template <typename E_t> void Compiler<E_t>::initialise_search() {

  setReverse();

  start_time = cpu_time();

  P.init(example.size());

  // tree must have at least one node (0)
  resize(1);

  blossom.add(0);

  count_by_example(0);

  grow(0);
}



template <typename E_t> void Compiler<E_t>::search() {

  separator("search");

  while (true) {

    ++search_size;

    PRINT_TRACE;

    if (blossom.empty() or fail()) {
      if (not backtrack())
        break;
    } else {
      expend();
    }
}

separator("optimal");
print_new_best();

auto T{wood[solution_root]};
cout << T << endl;
}

#ifdef PRINTTRACE

template <typename E_t> void Compiler<E_t>::print_trace() {

  if (PRINTTRACE) {

    // cout << setw(3) << decision.size();
    // for (auto i{0}; i < decision.size(); ++i)
    //   cout << "   ";
    cout << "#leaves = " << num_leaf
         // << "; size = " << currentSize()
         << "/";
    if (ub_size() < INFTY(int))
      cout << ub_size();
    else
      cout << "inf";
    cout << "; search=" << search_size << "; depth=" << min_depth_backtrack
         << endl;

    if (options.verbosity >= DTOptions::SOLVERINFO) {
      cout << "nodes: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(4) << *d << " ";
      }
      cout << "| ";
      for (auto b : blossom) {
        cout << setw(4) << b << " ";
      }
      cout << "| ";
      for (auto d{blossom.bbegin()}; d != blossom.bend(); ++d) {
        cout << setw(4) << *d << " ";
      }
      cout << endl << "parent ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(4) << parent[*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(4) << parent[b] << " ";
      }
      cout << endl << " left: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(4) << child[0][*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(4) << child[0][b] << " ";
      }
      cout << endl << "right: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(4) << child[1][*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(4) << child[1][b] << " ";
      }
      cout << endl << "depth: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(4) << depth[*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(4) << depth[b] << " ";
      }
      cout << endl << "featu: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        if (feature[*d] >= end_feature[*d])
          cout << "   * ";
        else
          cout << setw(4) << *feature[*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(4) << *feature[b] << " ";
        assert(feature[b] < end_feature[b]);
      }
      cout << endl << "error: ";
      cout.flush();
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(4) << node_error(*d) << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(4) << node_error(b) << " ";
      }
      cout << endl << "best:  ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(4);
        if (best[*d] < INFTY(int))
          cout << best[*d];
        else
          cout << "inf";
        cout << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(4);
        if (best[b] < INFTY(int))
          cout << best[b];
        else
          cout << "inf";
        cout << " ";
      }
      cout << endl << "lb:    ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(4) << lb[*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(4) << lb[b] << " ";
      }
      cout << endl << "tree:  ";
      cout.flush();
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(4) << tree[*d] << " ";
      }
      cout << endl;
    }
  }
}

#endif

template class Compiler<int>;
template class Compiler<unsigned long>;
}
