
#include "Backtrack.hpp"

namespace primer {

BacktrackingAlgorithm::BacktrackingAlgorithm(DataSet &d, Wood &w,
                                             DTOptions &opt)
    : wood(w), data(d), options(opt) {
			
			start_time = cpu_time();

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

  solution_root = -1;

  search_limit = static_cast<size_t>(options.search);

  time_limit = options.time;

  checking_period = 10000;

  interrupted = false;

  // tree must have at least one node (0)
  resize(1);
}

int BacktrackingAlgorithm::error() const { return ub_error; }

bool BacktrackingAlgorithm::limit_out() {
  ++search_size;

  if (time_limit > 0 and (num_backtracks % checking_period) == 0)
    if (cpu_time() >= (time_limit + start_time))
      interrupted = true;
  interrupted = interrupted or (search_limit and num_backtracks > search_limit);
  return interrupted;
}

int BacktrackingAlgorithm::get_feature_frequency(const int y, const int n,
                                                 const int f) const {
  return (f >= data.numFeature()
              ? P[y][n].count() -
                    pos_feature_frequency[y][n][f - data.numFeature()]
              : pos_feature_frequency[y][n][f]);
}

int BacktrackingAlgorithm::get_feature_error(const int n, const int f) const {
  auto not_f{f + data.numFeature()};
  return min(get_feature_frequency(0, n, f), get_feature_frequency(1, n, f)) +
         min(get_feature_frequency(0, n, not_f),
             get_feature_frequency(1, n, not_f));
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
  return feature[node] == end_feature[node]; // ranked_feature[node].end();
}

// return true if the feature f is true/false in all examples
bool BacktrackingAlgorithm::max_entropy(const int node, const int f) const {

  // auto error_parent{node_error(node)};
  // auto error_feature{get_feature_error(node, f)};

  // auto f{*(feature[node])};
  auto numNeg{P[0][node].count()};
  auto numPos{P[1][node].count()};

  auto me{(pos_feature_frequency[0][node][f] == numNeg and
           pos_feature_frequency[1][node][f] == numPos) or
          (pos_feature_frequency[1][node][f] == 0 and
           pos_feature_frequency[0][node][f] == 0)};

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

  return (pos_feature_frequency[0][node][f] == numNeg and
          pos_feature_frequency[1][node][f] == 0) or
         (pos_feature_frequency[1][node][f] == numPos and
          pos_feature_frequency[0][node][f] == 0);
}

void BacktrackingAlgorithm::separator(const string &msg) const {
  cout << setfill('-') << setw((94 - msg.size()) / 2) << "-"
       << "[" << msg << "]" << setw((94 - msg.size()) / 2) << "-" << endl
       << setfill(' ');
}

void BacktrackingAlgorithm::print_new_best() const {

  cout << "d size=" << left << setw(4) << ub_node
       // << " depth=" << left << setw(3) << ub_depth
       << " error=" << left << setw(4) << ub_error << " accuracy=" << setw(9)
       << setprecision(6)
       << (1.0 -
           static_cast<double>(ub_error) / static_cast<double>(data.count()))
       // << " backtracks=" << setw(9) << num_backtracks
       << " choices=" << setw(9) << search_size << " restarts=" << setw(5)
       << num_restarts << " mem=" << left << setw(4) << wood.size() << right
       << " time=" << setprecision(3) << cpu_time()-start_time << right << endl;
}

bool BacktrackingAlgorithm::isLeaf(const int node) const {
  return not blossom.contain(node) and child[0][node] < 0 and
         child[1][node] < 0;
}

void BacktrackingAlgorithm::resize(const int k) {

  optimal.resize(k, false);
  best_tree.resize(k, -1);
  feature.resize(k);
  end_feature.resize(k);
  blossom.reserve(k);
  depth.resize(k, 0);
  optimal.resize(k, -2);
  parent.resize(k, -1);

  min_error.resize(k, 0);
  max_error.resize(k, data.count());
  tree_error.resize(k, data.count());
  max_size.resize(k, numeric_limits<int>::max());

  auto i{ranked_feature.size()};
  ranked_feature.resize(k);
  for (; i < ranked_feature.size(); ++i) {
    for (auto f{0}; f < data.numFeature(); ++f)
      ranked_feature[i].push_back(f);
    feature[i] = ranked_feature[i].begin();
  }

  for (int y{0}; y < 2; ++y) {
    auto i = pos_feature_frequency[y].size();
    pos_feature_frequency[y].resize(k);
    while (i < pos_feature_frequency[y].size()) {
      pos_feature_frequency[y][i++].resize(data.numFeature());
    }
  }

  child[0].resize(k, -1);
  child[1].resize(k, -1);

  for (auto y{0}; y < 2; ++y)
    while (P[y].size() < k)
      P[y].addNode();
}

int BacktrackingAlgorithm::highest_error_reduction() const {

  auto selected_node{-1};
  auto highest_error{0};

  auto highest_reduction{0};

#ifdef PRINTTRACE
  if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO)
    cout << "select";
#endif

  // for (auto i{blossom.fbegin()}; i!=blossom.fend(); ++i) {// : blossom) {
  for (auto i : blossom) {
    assert(depth[i] < ub_depth);
    auto err{node_error(i)};
    auto reduction{err - get_feature_error(i, *(feature[i]))};

#ifdef PRINTTRACE
    if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO)
      cout << " " << i << ": (" << err << " - " << (err - reduction) << ")";
#endif

    if (reduction > highest_reduction or
        (reduction == highest_reduction and err > highest_error)) {
      highest_reduction = reduction;
      highest_error = err;
      selected_node = i;

#ifdef PRINTTRACE
      if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO)
        cout << "*";
#endif
    }
  }

#ifdef PRINTTRACE
  if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO)
    cout << endl;
#endif

  assert(selected_node >= 0);

  // cout << "--> " << selected_node << endl;

  return selected_node;
}

int BacktrackingAlgorithm::highest_error() const {

  auto selected_node{-1};
  auto highest_error{0};

#ifdef PRINTTRACE
  if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO)
    cout << "select";
#endif

  // for (auto i{blossom.fbegin()}; i!=blossom.fend(); ++i) {// : blossom) {
  for (auto i : blossom) {
    assert(depth[i] < ub_depth);
    auto err{node_error(i)};

#ifdef PRINTTRACE
    if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO)
      cout << " " << i << ": (" << err << " - " << (err - reduction) << ")";
#endif

    if (err > highest_error) {
      highest_error = err;
      selected_node = i;

#ifdef PRINTTRACE
      if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO)
        cout << "*";
#endif
    }
  }

#ifdef PRINTTRACE
  if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO)
    cout << endl;
#endif

  assert(selected_node >= 0);

  // cout << "--> " << selected_node << endl;

  return selected_node;
}

void BacktrackingAlgorithm::random_perturbation(const int node, const int kbest,
                                                const int p) {

  auto limit{min(kbest, static_cast<int>(end_feature[node] - feature[node]))};

  // if (limit < kbest)
  //   cout << "LIMIT: " << limit << endl;

  // assert(not null_entropy(node, *feature[node]));

  if (limit > 1 and random_generator() % 1000 > p) {

    auto rand_inc{random_generator() % limit};

    if (rand_inc > 0) {

      // cout << rand_inc << " " << *feature[node] << "/" << *(feature[node] +
      // rand_inc) << endl;

      // auto fb{*feature[node]};

      swap(*feature[node], *(feature[node] + rand_inc));

      // assert(fb != *feature[node]);
    }
  }
}

void BacktrackingAlgorithm::filter_features(const int node) {
  for (auto f{end_feature[node] - 1}; f >= feature[node]; --f)
    if (max_entropy(node, *f))
      swap(*f, *(--end_feature[node]));
}

void BacktrackingAlgorithm::sort_features(const int node) {
  for (auto f{feature[node]}; f != end_feature[node]; ++f) {
    // f_entropy[*f] = entropy(node, *f);
    f_error[*f] = get_feature_error(node, *f);
  }

  sort(
      feature[node], end_feature[node],
      // [&](const int a, const int b) { return f_entropy[a] < f_entropy[b]; });
      [&](const int a, const int b) { return f_error[a] < f_error[b]; });
}

void BacktrackingAlgorithm::count_by_example(const int node, const int y) {

  auto n{data.numFeature()};

  pos_feature_frequency[y][node].clear();
  pos_feature_frequency[y][node].resize(n, 0);

  auto stop = P[y][node].end();
  for (auto i{P[y][node].begin()}; i != stop; ++i)
    for (auto f : example[y][*i])
      ++pos_feature_frequency[y][node][f];
}

void BacktrackingAlgorithm::deduce_from_sibling(const int parent,
                                                const int node,
                                                const int sibling,
                                                const int y) {
  for (auto f{0}; f < data.numFeature(); ++f)
    pos_feature_frequency[y][node][f] = pos_feature_frequency[y][parent][f] -
                                        pos_feature_frequency[y][sibling][f];
}

void BacktrackingAlgorithm::cleaning() {
  if (solution_root < 0) {
    solution_root = wood.grow();
    wood.setFeature(solution_root, *feature[0]);
    int f[2] = {*feature[0] + static_cast<int>(data.count()), *feature[0]};

    for (auto i{0}; i < 2; ++i)
      wood.setChild(solution_root, i, get_feature_frequency(1, 0, f[i]) >
                                          get_feature_frequency(0, 0, f[i]));

    ub_error = get_feature_error(0, *feature[0]);
    ub_node = 1;
  }
}

TreeNode BacktrackingAlgorithm::getSolution() { return wood[solution_root]; }

bool BacktrackingAlgorithm::notify_solution() {

  if (current_error < ub_error or
      (current_error == ub_error and blossom.size() < ub_node)) {
    ub_error = current_error;
    ub_node = blossom.size();

    if (options.verbosity > DTOptions::QUIET)
      print_new_best();

    // cout << "solution " << wood.count() << " -> ";

    if (solution_root > 1)
      wood.freeNode(solution_root);

    // cout << wood.count() << " -> ";

    solution_root = copy_solution(0);

    // cout << wood[solution_root] << endl;

    // cout << wood.count() << endl;
  }

  return backtrack();
}

void BacktrackingAlgorithm::prune(const int node) {

#ifdef PRINTTRACE
  if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO)
    cout << "PRUNE " << node << endl;
#endif

  if (node >= 0) {

    if (depth[node] == ub_depth - 1 or optimal[node]) {

#ifdef PRINTTRACE
      if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO)
        cout << "ERROR = " << current_error << " - " << max_error[node] << endl;
#endif

      current_error -= max_error[node];
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

  // cout << "restart " << wood.count() << " -> ";

  for (auto i{blossom.frbegin()}; i != blossom.frend(); ++i)
    if (*i != 0 and best_tree[*i] > 1) {
      wood.freeNode(best_tree[*i]);
      best_tree[*i] = -1;
    }

  // cout << wood.count() << endl;

  prune(child[0][0]);
  prune(child[1][0]);

  current_error = node_error(0);

  blossom.add(0);

  backtrack_node = -1;

  restart_base *= options.restart_factor;
  restart_limit += static_cast<int>(restart_base);

  assert(blossom.count() == blossom.size() and blossom.size() == 1);
}

bool BacktrackingAlgorithm::update_upperbound(const int node) {
  auto err{0};
  auto sz{0};
  for (auto i{0}; i < 2; ++i)
    if (child[i][node] >= 0) {
      err += max_error[child[i][node]];
      sz += max_size[child[i][node]];
    }

  if (err < max_error[node] or
      (err == max_error[node] and sz < max_size[node])) {
    max_error[node] = err;
    max_size[node] = sz;
// store_best_tree(node, true);
#ifdef PRINTTRACE
    if (PRINTTRACE)
      cout << "new best for node " << node << ": feat=" << *feature[node]
           << ", error=" << max_error[node] << ", size=" << max_size[node]
           << endl;
#endif

    if (node > 0) {
      assert(parent[node] >= 0);
      update_upperbound(parent[node]);
    }

    return true;
  }

  return false;
}

bool BacktrackingAlgorithm::backtrack() {
  bool dead_end{false};
  do {

    ++num_backtracks;

    // cout << "backtrack\n";
    if (decision.empty())
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

    auto updt{update_upperbound(backtrack_node)};
    if (updt or tree_error[backtrack_node] > max_error[backtrack_node]) {

#ifdef PRINTTRACE
      if (PRINTTRACE and not updt)
        cout << "new best for node " << backtrack_node
             << ": feat=" << *feature[backtrack_node]
             << ", error=" << max_error[backtrack_node]
             << ", size=" << max_size[backtrack_node] << endl;
#endif

      tree_error[backtrack_node] = max_error[backtrack_node];
      store_best_tree(backtrack_node, true);

    } else {

#ifdef PRINTTRACE
      if (PRINTTRACE)
        cout << "no improvement for node " << backtrack_node
             << ": feat=" << *feature[backtrack_node] << "("
             << ") -> free the best subtrees" << endl;
#endif

      for (auto i{0}; i < 2; ++i)
        if (child[i][backtrack_node] >= 0 and
            best_tree[child[i][backtrack_node]] > 1)
          wood.freeNode(best_tree[child[i][backtrack_node]]);
    }

    // if (backtrack_node == 0) {
    //   cout << wood[0] << endl;
    // }

    ++feature[backtrack_node];

    prune(child[0][backtrack_node]);
    prune(child[1][backtrack_node]);

    child[0][backtrack_node] = -1;
    child[1][backtrack_node] = -1;

    assert(no_feature(backtrack_node) or
           (not max_entropy(backtrack_node, *feature[backtrack_node])));

    dead_end = (
        // current_error >= ub_error or
        (ub_error > 0 and max_error[backtrack_node] == 0) or
        no_feature(backtrack_node) or
        max_entropy(backtrack_node, *feature[backtrack_node]));

    if (dead_end) {
      optimal[backtrack_node] = true;

      min_error[backtrack_node] = max_error[backtrack_node];

// if (min_error[backtrack_node] >= max_error[parent[backtrack_node]]) {
//
//   if (backtrack_node != 0 and parent[backtrack_node] != decision.back())
//     cout << "bound check for " << backtrack_node << " could backtrack to "
//          << decision.back() << "!!\n";
// }

#ifdef PRINTTRACE
      if (PRINTTRACE) {
        for (auto i{0}; i < decision.size(); ++i)
          cout << "   ";
        cout << "end domain for " << backtrack_node << "! ==> set optimal\n";
      }

      if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO) {
        cout << "ERROR = " << current_error << " + "
             << max_error[backtrack_node] << endl;
      }
#endif

      current_error += max_error[backtrack_node];
    }

  } while (dead_end);

  // backtrack node is available for branching
  blossom.add(backtrack_node);

#ifdef PRINTTRACE
  if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO)
    cout << "ERROR = " << current_error << " + " << node_error(backtrack_node)
         << endl;
#endif

  current_error += node_error(backtrack_node);

  return true;
}

void BacktrackingAlgorithm::setChild(const int node, const bool branch,
                                     const int c) {

  parent[c] = node;
  if (P[0][c].count() == 0 or P[1][c].count() == 0) {
    child[branch][node] = -1 - (P[1][c].count() < P[0][c].count());
  } else {
    child[branch][node] = c;
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
  if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO)
    cout << "ERROR = " << current_error << " - " << node_error(node) << endl;
#endif

  current_error -= node_error(node);
  for (auto i{0}; i < 2; ++i) {
    auto s{child[i][node]};
    if (s >= 0) {
      grow(s);
// if(not grow(s))
// 	child[i][node] = -1 - (P[1][s].count() < P[0][s].count());

#ifdef PRINTTRACE
      if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO)
        cout << "ERROR = " << current_error << " + " << max_error[s] << endl;
#endif

      current_error += max_error[s];
    }
  }

  update_upperbound(node);

  // if(child[0][node] < 0 and child[1][node] < 0)
  // {
  //     max_error[node] = node_error(node);
  //     max_size[node] = 1;
  // 	best_tree[node] = (P[0][node].count() < P[1][node].count());
  //     optimal[node] = true;
  // }

  // assert(child[0][node] >= 0 or child[1][node] >= 0);
}

bool BacktrackingAlgorithm::grow(const int node) {
  // feature[node] = ranked_feature[node].begin();
  // sort_features(node);
  // end_feature[node] = ranked_feature[node].end();
  // filter_features(node);

  feature[node] = ranked_feature[node].begin();
  end_feature[node] = ranked_feature[node].end();
  filter_features(node);

  // if (feature[node] == end_feature[node])
  // 	return false;

  blossom.add(node);
  best_tree[node] = (P[0][node].count() < P[1][node].count());

  // assert(not ranked_feature[node].empty());

  if (feature[node] == end_feature[node]) {

    // cout << "grow " << node << " (deadend)" << endl;

    // assert(false);
    blossom.remove_front(node);
    tree_error[node] = max_error[node] = min_error[node] = node_error(node);
    child[0][node] = -1;
    child[1][node] = -1;
    max_size[node] = 0;
    optimal[node] = true;

    // if(node > 0)
    // 	update_upperbound(parent[node]);

  } else {

    sort_features(node);
    optimal[node] = false;

    int f[2] = {*feature[node] + static_cast<int>(data.numFeature()),
                *feature[node]};
    int err[2] = {min(get_feature_frequency(0, node, f[1]),
                      get_feature_frequency(1, node, f[1])),
                  min(get_feature_frequency(0, node, f[0]),
                      get_feature_frequency(1, node, f[0]))};

    if (depth[node] == ub_depth - 1 or err[0] + err[1] == 0) {

      // cout << "grow " << node << " (terminal)" << endl;

      blossom.remove_front(node);

      for (auto branch{0}; branch < 2; ++branch) {
        child[branch][node] = -1 - (get_feature_frequency(1, node, f[branch]) <
                                    get_feature_frequency(0, node, f[branch]));
      }

      tree_error[node] = max_error[node] = min_error[node] = err[0] + err[1];
      max_size[node] = 1;
      store_best_tree(node, true);
      // cout << "best_tree[" << node << "] = " << best_tree[node] << endl;

      // if(node > 0)
      // 	update_upperbound(parent[node]);

      // cout << "best_tree[" << node << "] = " << best_tree[node] << endl;

      optimal[node] = true;

    } else {

      // cout << "grow " << node << " (bud)" << endl;

      // best_tree[node] = -1;
      min_error[node] = 0;
      tree_error[node] = max_error[node] = node_error(node);
      max_size[node] = numeric_limits<int>::max();
    }
  }
  return true;
}

void BacktrackingAlgorithm::expend() {

  auto selected_node{backtrack_node};

  if (selected_node < 0) {

    switch (options.node_strategy) {
    case DTOptions::FIRST:
      selected_node = *blossom.begin();
      break;
    case DTOptions::RANDOM:
      selected_node =
          *(blossom.begin() + (random_generator() % blossom.count()));
      break;
    case DTOptions::ERROR:
      selected_node = highest_error();
      break;
    case DTOptions::ERROR_REDUCTION:
      selected_node = highest_error_reduction();
      break;
    }
    // selected_node = choose();

    assert(not max_entropy(selected_node, *feature[selected_node]));

    // selected_node = blossom[0];
  }

  if (options.width > 1)
    random_perturbation(selected_node, options.width,
                        static_cast<int>(options.focus * 1000.0));

  assert(feature[selected_node] >= ranked_feature[selected_node].begin() and
         feature[selected_node] < end_feature[selected_node]);

  branch(selected_node, *(feature[selected_node]));

  backtrack_node = -1;
}

void BacktrackingAlgorithm::search() {

  restart_limit = options.restart_base;
  restart_base = static_cast<double>(restart_limit);

  // compute error and sort features for the root node
  for (auto y{0}; y < 2; ++y)
    count_by_example(0, y);

  grow(0);

  current_error = max_error[0];

  backtrack_node = -1;

  if (options.verbosity > DTOptions::QUIET)
    separator("search");

  while (not limit_out()) {

    // ++search_size;

    if (num_backtracks > restart_limit)
      restart();

    PRINT_TRACE;

    DO_ASSERTS;

    if (blossom.empty()) {
      if (not notify_solution())
        break;
    } else if (options.bounding and fail()) {
      if (not backtrack())
        break;
    } else {
      expend();
    }
  }

  cleaning();

  if (options.verbosity > DTOptions::QUIET) {
    if (interrupted)
      separator("interrupted");
    else
      separator("optimal");
  }

  if (options.verbosity > DTOptions::SILENT)
    print_new_best();
  // cout << "error = " << ub_error << endl;
}

//// GARBAGE /////

bool BacktrackingAlgorithm::fail() {
  if (solution_root >= 0)
    for (auto b : blossom) {

#ifdef PRINTTRACE
      if (PRINTTRACE)
        cout << "bound from " << b << endl;
#endif

      auto lb{0};

      auto p{b};
      while (p > 0) {
        p = parent[p];

        auto ub{max_error[p]};

        for (auto i{0}; i < 2; ++i)
          if (child[i][p] >= 0)
            lb += min_error[child[i][p]];

#ifdef PRINTTRACE
        if (PRINTTRACE)
          cout << "parent " << p << " (ub=" << ub << ", lb=" << lb << ") ["
               << min_error[child[0][p]] << "/" << min_error[child[1][p]]
               << "]\n";
#endif

        if (lb >= ub) {

#ifdef PRINTTRACE
          if (PRINTTRACE)
            cout << "fail!! (" << b << " " << p << ")\n";
#endif

          current_error -= node_error(b);
          if (backtrack_node >= 0) {

            assert(backtrack_node == b);

            current_error += max_error[backtrack_node];
            // assert(max_error[backtrack_node] == node_error(b));

            optimal[backtrack_node] = true;
            min_error[backtrack_node] = max_error[backtrack_node];
            blossom.remove_front(backtrack_node);
          }
          // else {
          //           current_error -= node_error(b);
          // }

          // break;
          return true;
        }
      }
    }

  // if(afail)
  // 	exit(1);

  return false;

  // return lb >= ub_error;
}

int BacktrackingAlgorithm::copy_solution(const int node) {

  // cout << "copy node " << node << endl;

  if (node >= 0) {
    if (optimal[node]) {
      auto cn{wood.copyNode(best_tree[node])};

      // cout << best_tree[node] << endl;

      // assert(cn >= 0);

      return cn;
    } else {
      int root{wood.grow()};

      assert(root > 1);

      wood.setFeature(root, *feature[node]);
      for (int i{0}; i < 2; ++i) {
        auto c{copy_solution(child[i][node])};

        assert(c >= 0);

        wood.setChild(root, i, c);
      }

      return root;
    }
  }
  return node == -1;
}

// a new optimal tree routed at node k has been found
void BacktrackingAlgorithm::store_best_tree(const int node, const bool global) {

  // optimal nodes already have their best tree stored
  assert(not optimal[node]);

  // the previous best tree can be forgotten
  if (global and best_tree[node] > 1)
    wood.freeNode(best_tree[node]);

  // grow a new one
  best_tree[node] = wood.grow();

  // assert(child[true][node] < 0 or optimal[child[true][node]]);
  // assert(child[false][node] < 0 or optimal[child[false][node]]);

  wood.setFeature(best_tree[node], *feature[node]);

  for (auto i{0}; i < 2; ++i) {
    if (child[i][node] >= 0) {
      wood.setChild(best_tree[node], i, best_tree[child[i][node]]);
      best_tree[child[i][node]] = -1;
    } else {
      wood.setChild(best_tree[node], i, child[i][node] == -1);
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

    double val_size{
        static_cast<double>(get_feature_frequency(0, node, truef[x]) +
                            get_feature_frequency(1, node, truef[x]))};

    double entropy_x{0};
    for (auto y{0}; y < 2; ++y) {
      if (get_feature_frequency(y, node, truef[x]) != 0 and
          get_feature_frequency(y, node, truef[x]) != val_size) {
        entropy_x -=
            (get_feature_frequency(y, node, truef[x]) / val_size) *
            std::log2(get_feature_frequency(y, node, truef[x]) / val_size);
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
    return max_error[node];
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

    if (options.verbosity >= DTOptions::SOLVERINFO) {
      cout << "nodes: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << *d << (optimal[*d] ? "*" : (isLeaf(*d) ? "-" : " "));
      }
      cout << "| ";
      for (auto b : blossom) {
        cout << setw(3) << b << (optimal[b] ? "*" : (isLeaf(b) ? "-" : " "));
      }
      cout << "| ";
      for (auto d{blossom.bbegin()}; d != blossom.bend(); ++d) {
        cout << setw(3) << *d << (optimal[*d] ? "*" : (isLeaf(*d) ? "-" : " "));
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
        cout << setw(3) << leaf_error(*d) << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3) << leaf_error(b) << " ";
      }
      cout << endl << "max_e: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << max_error[*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3) << max_error[b] << " ";
      }
      cout << endl << "min_e: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << min_error[*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3) << min_error[b] << " ";
      }
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

    // if (not isLeaf(*d) and
    //     (parent[child[0][*d]] != *d or parent[child[1][*d]] != *d)) {
    //   cout << *d << " " << child[0][*d] << "->" << parent[child[0][*d]] << "
    //   / "
    //        << child[1][*d] << "->" << parent[child[1][*d]] << endl;
    // }

    assert(isLeaf(*d) or ((child[0][*d] < 0 or parent[child[0][*d]] == *d) and
                          (child[1][*d] < 0 or parent[child[1][*d]] == *d)));

    if (isLeaf(*d)) {
      total_error += leaf_error(*d);
      // cout << *d << " -> " << leaf_error(*d) << endl;
    }
  }
  for (auto b : blossom) {
    assert(leaf_error(b) == node_error(b));
    total_error += node_error(b);
    // cout << b << " -> " << node_error(b) << endl;
  }

  for (auto d : decision) {
    // assert((d == 0 or parent[d] >= 0));
    assert(blossom.index(d) < blossom.size());
  }

  if (total_error != current_error)
    cout << current_error << " / " << total_error << " @" << search_size
         << endl;

  assert(current_error == total_error);
}
#endif
}