
#include "Compiler.hpp"

namespace primer {


template <typename E_t> Compiler<E_t>::Compiler(DTOptions &opt) : options(opt) {
  num_feature = 0;
	num_leaf = 0;
  ub_size = INFTY(int);
  search_size = 0;
}

int lbLeaf(const int P, const int E) {
  // int E = num_feature - depth[node]; // the total number of examples is 2^E
  // int P = P[node].count(); // there are P positive examples and 2^E - P
  // negative examples

  // cout << "lbLeaf(" << P << "," << E << ")\n";

  if (P < 0 or E < 0)
    exit(1);

  // base cases are P==0, P==2^E and P==2^(E-1):
  if (P == 0)
    return 1;

  auto H{(1 << (E - 1))};

  if (E + 1 < 8 * sizeof(int)) {

    // if (2 * H < P) {
    //   cout << 2 * H << " < " << P << endl;
    //   exit(1);
    // }

    if (P == 2 * H)
      return 1;
    if (P == H)
      return 2;
    if (P > H)
      return lbLeaf(P - H, E - 1) + 1;
  }

  // E is large enough, the best is to put everything in P
  return lbLeaf(P, E - 1) + 1;
}

template <typename E_t> void Compiler<E_t>::count_by_example(const int node) {

  pos_feature_frequency[node].clear();
  pos_feature_frequency[node].resize(num_feature, 0);

  for (auto i : P[node])
    for (auto f : example[i])
      ++pos_feature_frequency[node][f];
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

  // auto left_error{pos_feature_frequency[n][f]};
  // if(halfsize(n) < 2 * left_error)
  // 	left_error = halfsize(n) - left_error;
  //
  // auto right_error{P[n].count() - pos_feature_frequency[n][f]};
  // if(halfsize(n) < 2 * right_error)
  // 	right_error = halfsize(n) - right_error;
  //
  // return

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

template <typename E_t> void Compiler<E_t>::setData(const DataSet &data) {

  num_feature = static_cast<int>(data.numFeature());

  f_error.resize(num_feature, 1);
  f_entropy.resize(num_feature, 1);
  f_gini.resize(num_feature, 1);

  example.resize(data.example[0].count());
  auto k{0};
  for (auto i : data.example[0]) {
    for (auto j{0}; j < num_feature; ++j)
      if (data.hasFeature(i, j)) {
        example[k].push_back(j);
      }
    ++k;
  }
  setReverse();
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

template <typename E_t> int Compiler<E_t>::getUbSize() const { return ub_size; }

template <typename E_t> bool Compiler<E_t>::no_feature(const int node) const {
  return feature[node] == end_feature[node];
}

template <typename E_t> void Compiler<E_t>::separator(const string &msg) const {
  cout << setfill('-') << setw((96 - msg.size()) / 2) << "-"
       << "[" << msg << "]" << setw((96 - msg.size()) / 2 + (msg.size() % 2))
       << "-" << endl
       << setfill(' ');
}

template <typename E_t> void Compiler<E_t>::print_new_best() {

  double t{
      static_cast<double>(static_cast<int>(100.0 * (cpu_time() - start_time))) /
      100.0};

  cout << " size = " << setw(3) << ub_size << " choices = " << setw(9)
       << left << search_size
       << " time=" << t << right << endl;
}

template <typename E_t> void Compiler<E_t>::resize(const int k) {

  // optimal.resize(k, false);
  feature.resize(k);
  end_feature.resize(k);
  blossom.reserve(k);
  depth.resize(k, 0);
  parent.resize(k, -1);
  child[0].resize(k, -1);
  child[1].resize(k, -1);

  best.resize(k, INFTY(int));
  lb.resize(k, 1);

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

  // switch (feature_criterion) {
  // case DTOptions::MINERROR:
  //   for (auto f{feature[node]}; f != end_feature[node]; ++f)
  //     f_error[*f] = error_policy.get_feature_error(node, *f);
  //   sort(feature[node], end_feature[node], [&](const int a, const int b) {
  //     return f_error[a] < f_error[b];
  //   });
  //   break;
  // case DTOptions::ENTROPY:
  //   for (auto f{feature[node]}; f != end_feature[node]; ++f)
  //     f_entropy[*f] = entropy(node, *f);
  //   sort(feature[node], end_feature[node], [&](const int a, const int b) {
  //     return f_entropy[a] < f_entropy[b];
  //   });
  //   break;
  // case DTOptions::GINI:
  //   for (auto f{feature[node]}; f != end_feature[node]; ++f)
  //     f_gini[*f] = gini(node, *f);
  //   sort(feature[node], end_feature[node], [&](const int a, const int b) {
  //     return (f_gini[a] < f_gini[b]);
  //   });
  //   break;
  // }
}

template <typename E_t> void Compiler<E_t>::prune(const int node) {

  if (node < 0)
    --num_leaf;
  else {
    if (not blossom.contain(node)) {
			if(best[node] < INFTY(int))
				num_leaf -= best[node];
      // since it was in the "used" pool, add it to move it to the "free" pool
      blossom.add(node);
    }
    blossom.remove_back(node);
  }

}

template <typename E_t> bool Compiler<E_t>::solutionFound() {

if(blossom.empty()) {
  auto current_size{currentSize()};
	
	assert(current_size > 0);
		
  if (current_size < ub_size) {
    ub_size = currentSize();
    if (options.verbosity > DTOptions::QUIET)
      print_new_best();
  }
	return true;
}

return false;
}


template <typename E_t> void Compiler<E_t>::updateBest(const int node) {

	// cout << "UPDATE BEST (" << node << "): " << best[node] << " / " << numLeaf(child[0][node]) << " + " << numLeaf(child[1][node]) << endl;


	auto bl{numLeaf(child[0][node])};
	auto br{numLeaf(child[1][node])};

	if(bl < INFTY(int) and br < INFTY(int)) {
		auto new_best{bl + br};
		if(new_best < best[node]) {
			best[node] = new_best;
			if(node)
				updateBest(parent[node]);
		}
	} 

}

// returns a lower bound on the size that one can get without changing any decision of the current branch (of the DT!!!)
template <typename E_t> bool Compiler<E_t>::fail() {
	
	if(decision.empty())
		return lb[0] >= best[0];
		
	auto node{decision.back()};
	
	
	cout << endl << "node=" << node << endl;
	cout << child[0][node] << endl; 
	cout << child[1][node] << endl; 
	
	auto LB{minLeaf(child[0][node]) + minLeaf(child[1][node])};
	
	cout << LB << endl;
	
	cout << "\nLB=" << LB << "/" << best[node] << " (" << node << ")\n";
	
	while(node > 0) {
		auto c{node};
		node = parent[node];
		
		LB += minLeaf(child[child[node][0] == c][node]);
		
		cout << "LB=" << LB << "/" << best[node] << " (" << node << ")\n";
	}
	
	return false;
}

template <typename E_t> bool Compiler<E_t>::backtrack() {
  bool dead_end{false};

  do {

    if (decision.empty())
      return false;

    auto node{decision.back()};
    decision.pop_back();

		updateBest(node);
		// auto bl{numLeaf(child[0][node])};
		// auto br{numLeaf(child[1][node])};
		//
		// // cout << "best = min(" << best[node] << ", (" << bl << " + " << br << ")) [" << INFTY(int) << "]\n";
		//
		// if(bl < INFTY(int) and br < INFTY(int)) {
		// 	best[node] =
		//       		std::min(best[node], bl + br);
		// }

#ifdef PRINTTRACE
    if (PRINTTRACE) {
			// cout << setw(3) << decision.size();
			//       for (auto i{0}; i < decision.size(); ++i)
			//         cout << "   ";
      cout << "backtrack on " << node << " = " << *feature[node] << endl;
    }
#endif

    ++feature[node];

    for (auto i{0}; i < 2; ++i)
      prune(child[i][node]);

    dead_end = (no_feature(node) or best[node] == lb[node]);

    if (!dead_end) {
      blossom.add(node);
    } else {
			
			assert(best[node] < INFTY(int));
			assert(lb[node] <= best[node]);
			
			
			// if(lb[node] < best[node]) {
			// 	cout << lb[node] << " -> " << best[node] << endl;
			// }
			
			lb[node] = best[node];
			
			// if(best[node] < INFTY(int))
			num_leaf += best[node];
    }

  } while (dead_end);

  return true;
}

template <typename E_t>
bool Compiler<E_t>::setChild(const int node, const bool branch, const int c) {
	
  parent[c] = node;
  depth[c] = depth[node] + 1;

  if (P[c].count() == 0) {
    child[branch][node] = -1;
  } else if (P[c].count() == usize(c)) {
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
    // cout << setw(3) << decision.size();
    // for (auto i{0}; i < decision.size(); ++i)
    //   cout << "   ";
    cout << "branch on " << node << " (" << P[node].count() << "/"
         << (usize(node) - P[node].count()) << ") with " << f;
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
	
	auto p=node;
	
	while(p>0) {
		p = parent[p];
		branch_features.set(*feature[p]);
	} //while(p > 0);

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

	bool pseudo_leaf{true};
  for (auto i{0}; i < 2; ++i)
    if (setChild(node, i, c[i]))
      pseudo_leaf &= grow(c[i]);
    else
      ++num_leaf;
		
		if(pseudo_leaf) {
			updateBest(node);
		}
		
	// for(auto i{0}; i < 2; ++i)
	// 	if(child[i][node]>=0)
	// 		updateBest(child[i][node]);

#ifdef PRINTTRACE
  if (PRINTTRACE) {
    // cout << setw(3) << decision.size()-1;
    // for (auto i{0}; i < decision.size()-1; ++i)
    //   cout << "   ";
    // cout << "branch on " << node << " (" << P[node].count() << "/"
    //      << (usize(node) - P[node].count()) << ") with " << f
    cout << " children: " << c[0] << " (" << P[c[0]].count() << "/"
         << (usize(c[0]) - P[c[0]].count()) << ") and " << c[1] << " ("
         << P[c[1]].count() << "/" << (usize(c[1]) - P[c[1]].count()) << ")"
         << endl;
  }
#endif
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

  sort_features(node);

  blossom.add(node);

  best[node] = INFTY(int);

  // cout << endl
  //      << "minleaf " << node << " " << num_feature << " " << depth[node]
  //      << endl;
  lb[node] = lbLeaf(P[node].count(), num_feature - depth[node]);

  if (P[node].count() == halfsize(node)) {
    int f[2] = {*feature[node] + numFeature(), *feature[node]};
    for (auto i{0}; i < 2; ++i) {
      if (get_feature_frequency(node, f[i]) == P[node].count()) {

        child[i][node] = -1;
        child[1 - i][node] = -2;
        blossom.remove_front(node);
        best[node] = 2;
				// best[node] = INFTY(int);
        num_leaf += 2;
				// updateBest(node);

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

  while (true) {
		
		++search_size;

    PRINT_TRACE;


    if (solutionFound() or fail()) { //WRONG!!
      if (not backtrack())
        break;
    } else {
      expend();
    }
  }
	
	cout << "search_size = " << search_size << endl;
	
}

template <typename E_t>
void Compiler<E_t>::addExample(const std::vector<int> &example,
                               const E_t weight) {
  addExample(example.begin(), example.end() - 1, example.back(), weight);
}

#ifdef PRINTTRACE

template <typename E_t> void Compiler<E_t>::print_trace() {
  if (PRINTTRACE) {

    // cout << setw(3) << decision.size();
    // for (auto i{0}; i < decision.size(); ++i)
    //   cout << "   ";
    cout << "#leaves = " << num_leaf << "; size = " << currentSize() << "/";
    if (ub_size < INFTY(int))
      cout << ub_size;
    else
      cout << "inf";
    cout << "; search=" << search_size << endl;

    if (options.verbosity >= DTOptions::SOLVERINFO) {
      cout << "nodes: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << *d << " ";
      }
      cout << "| ";
      for (auto b : blossom) {
        cout << setw(3) << b << " ";
      }
      cout << "| ";
      for (auto d{blossom.bbegin()}; d != blossom.bend(); ++d) {
        cout << setw(3) << *d << " ";
      }
      cout << endl << "parent ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << parent[*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3) << parent[b] << " ";
      }
      cout << endl << " left: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << child[0][*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3) << child[0][b] << " ";
      }
      cout << endl << "right: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << child[1][*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3) << child[1][b] << " ";
      }
      cout << endl << "depth: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << depth[*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3) << depth[b] << " ";
      }
      cout << endl << "featu: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << *feature[*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3) << *feature[b] << " ";
      }
      cout << endl << "error: ";
      cout.flush();
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << node_error(*d) << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3) << node_error(b) << " ";
      }
      cout << endl << "best:  ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3);
        if (best[*d] < INFTY(int))
          cout << best[*d];
        else
          cout << "inf";
        cout << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3);
        if (best[b] < INFTY(int))
          cout << best[b];
        else
          cout << "inf";
        cout << " ";
      }
      cout << endl << "lb:    ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << lb[*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3) << lb[b] << " ";
      }
      cout << endl;
      ;
    }
  }
}

#endif

template class Compiler<int>;
template class Compiler<unsigned long>;
}
