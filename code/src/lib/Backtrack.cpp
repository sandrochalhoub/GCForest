

#include "Backtrack.hpp"

namespace primer {



BacktrackingAlgorithm::BacktrackingAlgorithm(DataSet &d, Wood &w, Options &opt)
    : wood(w), data(d), options(opt) {

  // statistics and options
  ub_error = static_cast<size_t>(data.count());
  ub_node = options.max_size;
  ub_depth = options.max_depth;

  search_size = 0;
  num_backtracks = 0;
  num_restarts = 0;

  seed(options.seed);

  //
  auto m{data.numFeature()};
  f_error.resize(m, 1);

  // initialize the data structure to store the examples (vectors of positive
  // feature indices)
  for (int y{0}; y < 2; ++y) {
    example[y].resize(data.example[y].count());
    auto k{0};
    for (auto i : data.example[y]) {
      for (auto j{0}; j < data.numFeature(); ++j)
        if (data.hasFeature(i, j))
          example[y][k].push_back(j);
      ++k;
    }

    // Partition of the examples
    P[y].init(data.example[y].count());
  }

  // tree must have at least one node (0)
  resize(1);
}


int BacktrackingAlgorithm::get_feature_count(const int y, const int n,
                                             const int f) const {
  return (f >= data.numFeature()
              ? P[y][n].count() - pos_feature_count[y][n][f - data.numFeature()]
              : pos_feature_count[y][n][f]);
}

int BacktrackingAlgorithm::get_feature_error(const int n, const int f) const {
  auto not_f{f + data.numFeature()};
  return min(get_feature_count(0, n, f), get_feature_count(1, n, f)) +
         min(get_feature_count(0, n, not_f), get_feature_count(1, n, not_f));
}

void BacktrackingAlgorithm::seed(const int s) { random_generator.seed(s); }

size_t BacktrackingAlgorithm::size() { return blossom.size(); }

void BacktrackingAlgorithm::setUbDepth(const size_t u) { ub_depth = u; }

void BacktrackingAlgorithm::setUbNode(const size_t u) { ub_node = u; }

void BacktrackingAlgorithm::setUbError(const size_t u) { ub_error = u; }

size_t BacktrackingAlgorithm::node_error(const int i) const {
  return std::min(P[0][i].count(), P[1][i].count());
}

bool BacktrackingAlgorithm::no_feature(const int node) const {
  return feature[node] == ranked_feature[node].end();
}

// return true if the feature f is true/false in all examples
bool BacktrackingAlgorithm::max_entropy(const int node, const int f) const {

  // auto error_parent{node_error(node)};
  // auto error_feature{get_feature_error(node, f)};

  // auto f{*(feature[node])};
  auto numNeg{P[0][node].count()};
  auto numPos{P[1][node].count()};

  auto me{(pos_feature_count[0][node][f] == numNeg and
           pos_feature_count[1][node][f] == numPos) or
          (pos_feature_count[1][node][f] == 0 and
           pos_feature_count[0][node][f] == 0)};

  return me;
}

// return true if the feature f reduces the error
bool BacktrackingAlgorithm::reduce_error(const int node, const int f) const {

  // auto error_parent{node_error(node)};
  // auto error_feature{get_feature_error(node,f)};

  return get_feature_error(node, f) < node_error(node);
}

// return true if the feature f classifies all examples
bool BacktrackingAlgorithm::null_entropy(const int node, const int f) const {
  auto numNeg{P[0][node].count()};
  auto numPos{P[1][node].count()};

  return (pos_feature_count[0][node][f] == numNeg and
          pos_feature_count[1][node][f] == 0) or
         (pos_feature_count[1][node][f] == numPos and
          pos_feature_count[0][node][f] == 0);
}

void BacktrackingAlgorithm::separator() const {
	cout << setfill('-') << setw(95) << "-" << endl << setfill(' ');
}

void BacktrackingAlgorithm::print_new_best() const {

  cout << " size=" << left << setw(4) << ub_node << " depth=" << left << setw(3)
       << ub_depth << " error=" << left << setw(4)
       << ub_error << " accuracy=" << setw(9) << setprecision(6)
       << (1.0 -
           static_cast<double>(ub_error) / static_cast<double>(data.count()))
       // << " backtracks=" << setw(9) << num_backtracks
       << " choices=" << setw(9) << search_size << " restarts=" << setw(5)
       << num_restarts << " time=" << setprecision(3) << cpu_time() << right
       << endl;
}

bool BacktrackingAlgorithm::isLeaf(const int node) const {
  return not blossom.contain(node) and child[0][node] < 0 and
         child[1][node] < 0;
}

void BacktrackingAlgorithm::resize(const int k) {

  optimal.resize(k, false);
  best_tree.resize(k, -1);
  feature.resize(k);
  blossom.reserve(k);
  depth.resize(k, 0);
  optimal.resize(k, -2);

  best_error.resize(k, data.count());
  // cbest_feature.resize(k, -1);
  // cbest_size.resize(k, numeric_limits<int>::max());

  auto i{ranked_feature.size()};
  ranked_feature.resize(k);
  for (; i < ranked_feature.size(); ++i) {
    for (auto f{0}; f < data.numFeature(); ++f)
      ranked_feature[i].push_back(f);
    feature[i] = ranked_feature[i].begin();
  }

  for (int y{0}; y < 2; ++y) {
    auto i = pos_feature_count[y].size();
    pos_feature_count[y].resize(k);
    while (i < pos_feature_count[y].size()) {
      pos_feature_count[y][i++].resize(data.numFeature());
    }
  }

  child[0].resize(k, -1);
  child[1].resize(k, -1);

  for (auto y{0}; y < 2; ++y)
    while (P[y].size() < k)
      P[y].addNode();
}

int BacktrackingAlgorithm::choose() const {

  auto selected_node{-1};
  auto max_error{0};

  auto max_reduction{0};

  // for (auto i{blossom.fbegin()}; i!=blossom.fend(); ++i) {// : blossom) {
  for (auto i : blossom) {
    assert(depth[i] < ub_depth);
    auto err{node_error(i)};
    auto reduction{err - get_feature_error(i, *(feature[i]))};

    if (reduction > max_reduction or
        (reduction == max_reduction and err > max_error)) {
      max_reduction = reduction;
      max_error = err;
      selected_node = i;
    }
  }

  assert(selected_node >= 0);

  // cout << "--> " << selected_node << endl;

  return selected_node;
}

void BacktrackingAlgorithm::random_perturbation(const int selected_node,
                                                const int kbest, const int p) {

  if (f_error[*(ranked_feature[selected_node].begin())] != 0 and
      random_generator() % 1000 > p) {
    swap(*(ranked_feature[selected_node].begin()),
         *(ranked_feature[selected_node].begin() +
           (random_generator() % kbest)));
  }
}

void BacktrackingAlgorithm::sort_features(const int selected_node) {
  if (feature[selected_node] != ranked_feature[selected_node].begin()) {
    cout << "sort features of " << selected_node << " "
         << (feature[selected_node] - ranked_feature[selected_node].begin())
         << endl;
    exit(1);
  }

  auto max_error{min(P[0][selected_node].count(), P[1][selected_node].count())};

  for (auto f{0}; f < data.numFeature(); ++f) {
    // f_entropy[f] = entropy(selected_node, f);
    f_error[f] = get_feature_error(selected_node, f) +
                 max_error * (max_entropy(selected_node, f));
  }

  sort(
      ranked_feature[selected_node].begin(),
      ranked_feature[selected_node].end(),
      // [&](const int a, const int b) { return f_entropy[a] < f_entropy[b]; });
      [&](const int a, const int b) { return f_error[a] < f_error[b]; });

  // while()
}

void BacktrackingAlgorithm::count_by_example(const int node, const int y) {

  auto n{data.numFeature()};

  pos_feature_count[y][node].clear();
  pos_feature_count[y][node].resize(n, 0);

  auto stop = P[y][node].end();
  for (auto i{P[y][node].begin()}; i != stop; ++i)
    for (auto f : example[y][*i])
      ++pos_feature_count[y][node][f];
}

void BacktrackingAlgorithm::deduce_from_sibling(const int parent,
                                                const int node,
                                                const int sibling,
                                                const int y) {
  for (auto f{0}; f < data.numFeature(); ++f)
    pos_feature_count[y][node][f] =
        pos_feature_count[y][parent][f] - pos_feature_count[y][sibling][f];
}

bool BacktrackingAlgorithm::notify_solution() {

  if (current_error < ub_error) {
    ub_error = current_error;
    ub_node = blossom.size();

    print_new_best();

    // cout << "error = " << current_error << endl;
    //
    // if (best_tree[0] >= 0)
    //   cout << wood[best_tree[0]] << endl;
    //
    // if (best_tree[0] >= 0) {
    //   cout << "try to free treee[0]\n";
    //
    //   wood[best_tree[0]].free();
    // }
    // for (auto i{blossom.frbegin()}; i != blossom.frend(); ++i)
    //   if (not optimal[*i]) {
    //
    //     cout << "store " << *i << endl;
    //
    //     store_best_tree(*i, false);
    //   }
    // // }
    //
    // cout << wood[best_tree[0]] << endl;
    //
    // auto err{wood[best_tree[0]].predict(data)};
    //
    // cout << " ==> " << err << endl;
    //
    // assert(err == current_error);

    // exit(1);

    // cout << "print\n";

    // store_solution();
  }

  return backtrack();
}

void BacktrackingAlgorithm::prune(const int node) {

#ifdef PRINTTRACE
  if (options.verbosity >= Options::SOLVERINFO)
    cout << "PRUNE " << node << endl;
#endif

  if (node >= 0) {

    if (depth[node] == ub_depth - 1 or optimal[node]) {

#ifdef PRINTTRACE
      if (options.verbosity >= Options::SOLVERINFO)
        cout << "ERROR = " << current_error << " - " << best_error[node]
             << endl;
#endif

      current_error -= best_error[node];
    }

    for (auto i{0}; i < 2; ++i)
      if (child[i][node] >= 0)
        prune(child[i][node]);

    blossom.add(node);
    blossom.remove_back(node);
  }
}

void BacktrackingAlgorithm::restart() {
  ++num_restarts;

  decision.clear();

  prune(child[0][0]);
  prune(child[1][0]);

  current_error = node_error(0);

  blossom.add(0);

  backtrack_node = -1;

  assert(blossom.count() == blossom.size() and blossom.size() == 1);
}

bool BacktrackingAlgorithm::backtrack() {
  bool dead_end{false};
  do {

    ++num_backtracks;

    // cout << "backtrack\n";
    if (backtrack_node == 0)
      return false;

    backtrack_node = decision.back();
    decision.pop_back();

#ifdef PRINTTRACE
    if (PRINTTRACE) {
      for (auto i{0}; i < decision.size(); ++i)
        cout << "   ";
      cout << "backtrack to " << backtrack_node << endl;
    }
#endif

    auto err{
        (child[0][backtrack_node] >= 0 ? best_error[child[0][backtrack_node]]
                                       : 0) +
        (child[1][backtrack_node] >= 0 ? best_error[child[1][backtrack_node]]
                                       : 0)};

    if (err < best_error[backtrack_node]) {
      best_error[backtrack_node] = err;
      store_best_tree(backtrack_node, true);

// remove the pointers to the subtrees, as they should be freed only when the
// current tree is freed

#ifdef PRINTTRACE
      if (PRINTTRACE)
        cout << "new best for node " << backtrack_node
             << ": feat=" << *feature[backtrack_node]
             << ", error=" << best_error[backtrack_node] << endl;
#endif

    } else {

#ifdef PRINTTRACE
      if (PRINTTRACE)
        cout << "no improvement for node " << backtrack_node
             << ": feat=" << *feature[backtrack_node] << "("
             << ") -> free the best subtrees" << endl;
#endif

      for (auto i{0}; i < 2; ++i)
        if (child[i][backtrack_node] >= 0 and best_tree[child[i][backtrack_node]] > 1)
          wood[best_tree[child[i][backtrack_node]]].free();
			
    }

    ++feature[backtrack_node];

    prune(child[0][backtrack_node]);
    prune(child[1][backtrack_node]);

    child[0][backtrack_node] = -1;
    child[1][backtrack_node] = -1;

    dead_end = (
        // current_error >= ub_error or
        best_error[backtrack_node] == 0 or no_feature(backtrack_node) or
        max_entropy(backtrack_node, *feature[backtrack_node]));

    if (dead_end) {
      optimal[backtrack_node] = true;

#ifdef PRINTTRACE
      if (PRINTTRACE) {
        for (auto i{0}; i < decision.size(); ++i)
          cout << "   ";
        cout << "end domain for " << backtrack_node << "! ==> set optimal\n";
      }

      if (options.verbosity >= Options::SOLVERINFO) {
        cout << "ERROR = " << current_error << " + "
             << best_error[backtrack_node] << endl;
      }
#endif
			
        if (backtrack_node == 0) {
          auto err{wood[best_tree[backtrack_node]].predict(data)};
          assert(err == ub_error);
        }

      current_error += best_error[backtrack_node];
    }

  } while (dead_end);

  // backtrack node is available for branching
  blossom.add(backtrack_node);

#ifdef PRINTTRACE
  if (options.verbosity >= Options::SOLVERINFO)
    cout << "ERROR = " << current_error << " + " << node_error(backtrack_node)
         << endl;
#endif

  current_error += node_error(backtrack_node);

  return true;
}

void BacktrackingAlgorithm::setChild(const int node, const bool branch,
                                     const int c) {

  if (P[0][c].count() == 0 or P[1][c].count() == 0) {
    child[branch][node] = -1 - (P[1][c].count() < P[0][c].count());
    // parent[c] = -1;
  } else {
    child[branch][node] = c;
    // parent[c] = node;
    depth[c] = depth[node] + 1;
  }
}

void BacktrackingAlgorithm::branch(const int node, const int f) {

  // we assume that we branch only nodes
  assert(depth[node] < ub_depth - 1);

  // we assume that we branch only on tests with non-null error
  assert(get_feature_error(node, f) > 0);

  decision.push_back(node);
  blossom.remove_front(node);

  // we create two nodes even if one branch is pure, but we'll free it
  if (blossom.capacity() < blossom.size() + 2)
    resize(blossom.size() + 2);

  int c[2] = {*blossom.bbegin(), *(blossom.bbegin() + 1)};

  // partition
  for (auto y{0}; y < 2; ++y) {
    P[y].branch(node, c[1], c[0],
                [&](const int x) { return data.ithHasFeature(y, x, f); });
  }

#ifdef PRINTTRACE
  if (PRINTTRACE) {
    cout << setw(3) << decision.size();
    for (auto i{0}; i < decision.size(); ++i)
      cout << "   ";
    cout << "branch on " << node << " with " << f << " children: " << c[0]
         << " (" << P[0][c[0]].count() << "/" << P[1][c[0]].count() << ") and  "
         << c[1] << "(" << P[0][c[1]].count() << "/" << P[1][c[1]].count()
         << ")" << endl;
  }
#endif

  for (auto i{0}; i < 2; ++i)
    setChild(node, i, c[i]);

  for (auto y{0}; y < 2; ++y) {
    auto smallest{P[y][c[1]].count() < P[y][c[0]].count()};

    count_by_example(c[smallest], y);

    deduce_from_sibling(node, c[1 - smallest], c[smallest], y);
  }

#ifdef PRINTTRACE
  if (options.verbosity >= Options::SOLVERINFO)
    cout << "ERROR = " << current_error << " - " << node_error(node) << endl;
#endif

  current_error -= node_error(node);
  for (auto i{0}; i < 2; ++i) {
    auto c{child[i][node]};
    if (c >= 0) {
      grow(c);

#ifdef PRINTTRACE
      if (options.verbosity >= Options::SOLVERINFO)
        cout << "ERROR = " << current_error << " + " << best_error[c] << endl;
#endif

      current_error += best_error[c];
    }
  }
}

void BacktrackingAlgorithm::grow(const int node) {
  blossom.add(node);
  feature[node] = ranked_feature[node].begin();
  best_tree[node] = (P[0][node].count() < P[1][node].count());
  optimal[node] = false;
  sort_features(node);

  int f[2] = {*feature[node] + static_cast<int>(data.numFeature()),
              *feature[node]};
  int err[2] = {
      min(get_feature_count(0, node, f[1]), get_feature_count(1, node, f[1])),
      min(get_feature_count(0, node, f[0]), get_feature_count(1, node, f[0]))};

  if (depth[node] == ub_depth - 1 or err[0] + err[1] == 0) {
    blossom.remove_front(node);
    best_error[node] = err[0] + err[1];

    for (auto branch{0}; branch < 2; ++branch) {
      child[branch][node] = -1 - (get_feature_count(1, node, f[branch]) <
                                  get_feature_count(0, node, f[branch]));
    }

    store_best_tree(node, false);

    optimal[node] = true;

  } else {
    best_error[node] = node_error(node);
  }
}

void BacktrackingAlgorithm::expend() {

  auto selected_node{backtrack_node};

  if (selected_node < 0) {
    selected_node = choose();
    // selected_node = blossom[0];
    random_perturbation(selected_node, options.width,
                        static_cast<int>(options.focus * 1000.0));
  } 

  assert(feature[selected_node] >= ranked_feature[selected_node].begin() and
         feature[selected_node] < ranked_feature[selected_node].end());

  branch(selected_node, *(feature[selected_node]));

  backtrack_node = -1;
}

void BacktrackingAlgorithm::search() {

  auto restart_limit{options.restart_base};
  double restart_base{static_cast<double>(restart_limit)};

  // compute error and sort features for the root node
  for (auto y{0}; y < 2; ++y)
    count_by_example(0, y);

  grow(0);

  current_error = best_error[0];

  backtrack_node = -1;

  while (true) {

    ++search_size;

    if (num_backtracks > restart_limit) {
      restart();
      restart_base *= options.restart_factor;
      restart_limit += static_cast<int>(restart_base);
    }

    PRINT_TRACE;

    DO_ASSERTS;

    if (blossom.empty()) {
      if (not notify_solution())
        break;
    } 
		// else if (fail()) {
		//       if (not backtrack())
		//         break;
		//     }
		else {
      expend();
    }
  }

	separator();
  print_new_best();
	// cout << "error = " << ub_error << endl;
}

//// GARBAGE /////

bool BacktrackingAlgorithm::fail() {
	return false;
	
  auto lb{current_error};

  for (auto b : blossom)
    lb -= node_error(b);

  return lb >= ub_error;
}

// a new optimal tree routed at node k has been found
void BacktrackingAlgorithm::store_best_tree(const int node, const bool global) {

  // optimal nodes already have their best tree stored
  assert(not optimal[node]);

  // the previous best tree can be forgotten
  if (global and best_tree[node] > 1)
    wood[best_tree[node]].free();

  // grow a new one
  best_tree[node] = wood.grow();

  // auto rc{(child[true][node] >= 0 ? best_tree[child[true][node]]
  //                                 : child[true][node] == -1)};
  //
  // auto lc{(child[false][node] >= 0 ? best_tree[child[false][node]]
  //                                  : child[false][node] == -1)};

  // cout << "store best tree for node " << node << " (use " << best_tree[node]
  //      << ") -> " << lc << "/" << rc << endl;

  assert(child[true][node] < 0 or optimal[child[true][node]]);
  assert(child[false][node] < 0 or optimal[child[false][node]]);

  wood[best_tree[node]].feature = *feature[node];

  for (auto i{0}; i < 2; ++i) {
    if (child[i][node] >= 0) {
      wood[best_tree[node]].setChild(i, best_tree[child[i][node]]);

      // cout << "set child[" << i << "] of " << node << ": " <<
      // best_tree[child[i][node]] << endl;

      best_tree[child[i][node]] = -1;

      // cout << wood[best_tree[node]].child(i).getIndex() << " -- " <<
      // best_tree[child[i][node]] << endl;
    } else {
      wood[best_tree[node]].setChild(i, child[i][node] == -1);
    }
  }
}

std::ostream &BacktrackingAlgorithm::display(std::ostream &os) const {

  for (auto i : blossom) {
    cout << i << ": " << P[0][i].count() << "/" << P[1][i].count() << endl;
  }

  return os;
}

std::ostream &operator<<(std::ostream &os, const BacktrackingAlgorithm &x) {
  return x.display(os);
}

double BacktrackingAlgorithm::entropy(const int node, const int feature) {
  double feature_entropy{0};

  int not_feature = (feature + data.numFeature());
  int truef[2] = {not_feature, feature};

  double total_size{
      static_cast<double>(P[0][node].count() + P[1][node].count())};

  for (auto x{0}; x < 2; ++x) {

    double val_size{static_cast<double>(get_feature_count(0, node, truef[x]) +
                                        get_feature_count(1, node, truef[x]))};

    double entropy_x{0};
    for (auto y{0}; y < 2; ++y) {
      if (get_feature_count(y, node, truef[x]) != 0 and
          get_feature_count(y, node, truef[x]) != val_size) {
        entropy_x -= (get_feature_count(y, node, truef[x]) / val_size) *
                     std::log2(get_feature_count(y, node, truef[x]) / val_size);
      }
    }
    // Pr(X=x) = val_size / total_size

    // H(Y|X) = \sum_x Pr(X=x) H(Y|X=x)
    feature_entropy += (entropy_x * val_size / total_size);
  }

  return feature_entropy;
}

#ifdef PRINTTRACE

size_t BacktrackingAlgorithm::leaf_error(const int node) const {
  if (isLeaf(node) or optimal[node]) {
    return best_error[node];
  } else {
    return node_error(node);
  }
}

void BacktrackingAlgorithm::print_trace() {
  if (PRINTTRACE) {

    cout << setw(3) << decision.size();
    for (auto i{0}; i < decision.size(); ++i)
      cout << "   ";
    cout << "ub (error) = " << ub_error << "; ub (depth) = " << ub_depth
         << "; search=" << search_size << endl;

    if (options.verbosity >= Options::SOLVERINFO) {
      cout << "nodes: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << *d << (optimal[*d] ? "*" : " ");
      }
      cout << "| ";
      for (auto b : blossom) {
        cout << setw(3) << b << (optimal[b] ? "*" : " ");
      }
      cout << "| ";
      for (auto d{blossom.bbegin()}; d != blossom.bend(); ++d) {
        cout << setw(3) << *d << (optimal[*d] ? "*" : " ");
      }
      // cout << endl << "parent ";
      // for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
      //   cout << setw(3) << parent[*d] << " ";
      // }
      // cout << "  ";
      // for (auto b : blossom) {
      //   cout << setw(3) << parent[b] << " ";
      // }
      // cout << "  ";
      // for (auto d{blossom.bbegin()}; d != blossom.bend(); ++d) {
      //   cout << setw(3) << parent[*d] << " ";
      // }
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
        cout << setw(3) << leaf_error(*d) << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3) << leaf_error(b) << " ";
      }
      cout << endl << "b(er): ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << best_error[*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3) << best_error[b] << " ";
      }
      cout << "  ";
      for (auto d{blossom.bbegin()}; d != blossom.bend(); ++d) {
        cout << setw(3) << best_error[*d] << " ";
      }
      // cout << "  ";
      //       for (auto d{blossom.bbegin()}; d != blossom.bend(); ++d) {
      //         cout << setw(3) << best_error[*d] << " ";
      //       }
      cout << endl;
    }
  }
}

void BacktrackingAlgorithm::do_asserts() {

  for (auto b : blossom) {
    assert(not optimal[b]);
  }

  auto total_error{0};
  for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
    if (isLeaf(*d)) {
      total_error += leaf_error(*d);
    }
  }
  for (auto b : blossom) {
    assert(leaf_error(b) == node_error(b));
    total_error += node_error(b);
  }

  for (auto d : decision) {
    // assert((d == 0 or parent[d] >= 0));
    assert(blossom.index(d) < blossom.size());
  }

  if (total_error != current_error)
    cout << current_error << " / " << total_error << endl;

  assert(current_error == total_error);
}
#endif
}