
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include "Backtrack.hpp"

#define PRINTTRACE (options.verbosity >= Options::YACKING)

double cpu_time(void) {
  struct rusage ru;
  getrusage(RUSAGE_SELF, &ru);
  return (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000;
}

namespace primer {

BacktrackingAlgorithm::BacktrackingAlgorithm(DataSet &d, Options &opt)
    : data(d), options(opt) {
  auto m{data.numFeature()};

  //   blossom.reserve(1);
  //   blossom.add(0);
  //
  // parent.push_back(0);

  f_entropy.resize(m, 1);
  f_error.resize(m, 1);

  ub_node = static_cast<size_t>(-1);
  ub_error = static_cast<size_t>(data.count());

  for (int y{0}; y < 2; ++y) {
    example[y].resize(data.example[y].count());
    auto k{0};
    for (auto i : data.example[y]) {
      for (auto j{0}; j < data.numFeature(); ++j)
        if (data.hasFeature(i, j))
          example[y][k].push_back(j);
      ++k;
    }
  }

  num_node = 1;
  resize(num_node);

  for (auto y{0}; y < 2; ++y) {
    P[y].init(data.example[y].count());
    P[y].addNode();
    // pos_feature_count[y].resize(1);
    // pos_feature_count[y][0].resize(data.numFeature(), 0);
    count_by_example(0, y);
  }

  // cout << "here\n";
  //
  //   ranked_feature.resize(1);
  //   for (auto f{0}; f < data.numFeature(); ++f)
  //     ranked_feature.begin()->push_back(f);
  // feature.push_back(ranked_feature.begin()->begin());

  // st_trail.push_back(0);
  // sz_trail.push_back(1);
  //

  // leaf.reserve(1);
  blossom.reserve(1);
  blossom.add(0);

  nodes.reserve(1);
  nodes.add(0);
  // blossom.remove_front(0);

  max_depth.push_back(0);

  // cout << "end constructor\n";

  // cout << P[0][0].count() << endl;

  // cout << P[1][0].count() << endl;

  ub_node = options.max_size;

  ub_depth = options.max_depth;

  search_size = 0;
  num_backtracks = 0;
  num_restarts = 0;

  seed(options.seed);

  size_matters = false;

  current_error = error(0);

  bourgeon.reserve(1);
  bourgeon.add(0);

#ifdef DEBUG_MODE
  debug_sol = NULL;
#endif
}

void BacktrackingAlgorithm::resize(const int k) {

  assert(k % 2);

  // cout << k << " " << num_node << " " << (parent.size()) << endl;

  // assert(num_node == k);
  //
  // assert(num_node == parent.size() + 2);

  // leaf.reserve(k);
  feature.resize(k);
  blossom.reserve(k);

  parent.resize(k, -1);

  depth.resize(k, 0);

  optimal.resize(k, -2);

  best_feature.resize(k, -1);
  best_child.resize(k, -1);

  cbest_feature.resize(k, -1);
  cbest_error.resize(k, data.count());
  cbest_size.resize(k, numeric_limits<int>::max());

  auto i{ranked_feature.size()};
  ranked_feature.resize(k);
  for (; i < ranked_feature.size(); ++i) {
    for (auto f{0}; f < data.numFeature(); ++f)
      ranked_feature[i].push_back(f);
    feature[i] = ranked_feature[i].begin();
  }
  // cout << "here 5\n";

  for (int y{0}; y < 2; ++y) {
    auto i = pos_feature_count[y].size();
    pos_feature_count[y].resize(k);
    while (i < pos_feature_count[y].size()) {
      pos_feature_count[y][i++].resize(data.numFeature());
    }
  }
  // cout << "end resize\n";
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
  //
  // return (f >= data.numFeature()
  //             ? P[y][n].count() - pos_feature_count[y][n][f -
  //             data.numFeature()]
  //             : pos_feature_count[y][n][f]);
}

void BacktrackingAlgorithm::seed(const int s) { random_generator.seed(s); }

size_t BacktrackingAlgorithm::size() { return blossom.size(); }

void BacktrackingAlgorithm::clear(int &node) {

  //   for (auto i{0}; i < 2; ++i)
  //       P[i].clear();
  //
  // blossom.clear();
  //
  // blossom.add(0);
  //
  // num_node = 1;
  //
  // // resize(1);

  //
  // int node{-1};
  while (num_node > 1)
    backtrack(node);

  assert(node == 0);
  feature[0] = ranked_feature[0].begin();

  // cout << "RESTART TO " << num_node << endl;
}

void BacktrackingAlgorithm::setUbDepth(const size_t u) { ub_depth = u; }

void BacktrackingAlgorithm::setUbNode(const size_t u) { ub_node = u; }

void BacktrackingAlgorithm::setUbError(const size_t u) { ub_error = u; }

// void BacktrackingAlgorithm::setMaxNodes(const size_t m) { max_num_node = m; }

size_t BacktrackingAlgorithm::error() const {
  size_t error{0};
  for (auto i : blossom) {

    // cout << " min(" << P[0][i].count() << ", " << P[1][i].count() << ")";

    auto e{std::min(P[0][i].count(), P[1][i].count())};
    error += e;
  }

  return error;
}

size_t BacktrackingAlgorithm::error(const int i) const {
  return std::min(P[0][i].count(), P[1][i].count());
}

double BacktrackingAlgorithm::accuracy() const {
  return 1.0 - static_cast<double>(error()) / static_cast<double>(data.count());
}

// void BacktrackingAlgorithm::greedy(const int kbest, const double p,
//                                    const int max_num_node) {
//
//   // cout << "start greedy\n";
//
//   int pint = static_cast<int>(p * 1000);
//   while (blossom.count() and blossom.size() + 2 * blossom.count() < ub and
//          num_node < max_num_node) {
//
//     // cout << "iteration " << blossom.size() << endl;
//
//     if (num_node >= parent.size())
//       resize(num_node);
//
//     expend(kbest, pint);
//     // cout << setw(4) << blossom.size() << " " << accuracy() << endl;
//   }
// }

// returns a lower bound on the number of nodes necessary to achieve a correct
// classification
size_t BacktrackingAlgorithm::depth_lower_bound() {
  auto lb{0};
  for (auto i : blossom)
    if (lb < depth[i])
      lb = depth[i];
  return lb + 1;
}

size_t BacktrackingAlgorithm::node_lower_bound() {
  auto lb{num_node};
  for (auto i : blossom) {
    // each blossom need to be expended, and depending on the feature count, we
    // can
    // determine if the children will need to be expended again
    lb += 2;
    auto f{*feature[i]};
    auto not_f{f + data.numFeature()};
    if (get_feature_count(0, i, f) > 0 and get_feature_count(1, i, f) > 0)
      lb += 2;
    if (get_feature_count(0, i, not_f) > 0 and
        get_feature_count(1, i, not_f) > 0)
      lb += 2;
  }

  return lb;
}

// returns a lower bound on the error
size_t BacktrackingAlgorithm::error_lower_bound() {

  if (2 * blossom.count() + num_node <= ub_node)
    return 0;

  auto lb_node{num_node};
  auto lb_error{error()};

  buffer.clear();

  // cout << endl << "base error = " << lb_error << endl;;

  for (auto i : blossom) {
    auto err_i{min(P[0][i].count(), P[1][i].count())};

    auto f{*feature[i]};
    auto not_f{f + data.numFeature()};

    // how much we save (optimistically) by having a test at i's left child
    auto err_f{min(get_feature_count(0, i, f), get_feature_count(1, i, f))};

    // how much we save (optimistically) by having a test at i's right child
    auto err_not_f{
        min(get_feature_count(0, i, not_f), get_feature_count(1, i, not_f))};

    // how much we save by having a test at i
    buffer.push_back(err_i - err_f - err_not_f);

    // cout << i << ": " << (err_i - err_f - err_not_f) << endl;

    if (err_f > 0) {
      buffer.push_back(err_f);
      lb_node += 2;

      // cout << i << "'s left child: " << (err_f) << endl;
    }

    if (err_not_f > 0) {
      buffer.push_back(err_not_f);
      lb_node += 2;

      // cout << i << "'s right child: " << (err_not_f) << endl;
    }
  }

  sort(buffer.begin(), buffer.end());
  assert(*buffer.begin() <= *buffer.rbegin());

  // cout << "we need at least " << lb_node << ", however, we can have at most "
  // << ub_node << endl;

  if (lb_node > ub_node) {

    // cout << "select the " << (lb_node - ub_node)/2 << " highest decrement:";

    auto best{buffer.end()};
    for (auto i{0}; i < lb_node - ub_node; i += 2) {
      lb_error -= *(--best);

      // cout << " " << (lb_error + *best) << " -> " << lb_error << endl;
    }

  } else
    lb_error = 0;

  return lb_error;
}

void BacktrackingAlgorithm::set_optimal(const int node) {
  optimal[node] = parent[node];

  if (PRINTTRACE)
    cout << "OPTIMAL " << node << ":" << cbest_feature[node] << "="
         << cbest_error[node] << endl;
}

void BacktrackingAlgorithm::unset(const int node) {

  if (PRINTTRACE)
    cout << "UNSET " << node << endl;

  // optimal[node] = -2;
  // parent[node] = -1;
  blossom.remove_back(node);
}

bool BacktrackingAlgorithm::is_optimal(const int node) const {
  return optimal[node] == parent[node];
}

bool BacktrackingAlgorithm::not_branched(const int node) const {
  return feature[node] == ranked_feature[node].begin();
  // return feature[node] == ranked_feature[node].end();
}

bool BacktrackingAlgorithm::no_feature(const int node) const {

  // std::cout << "**"
  //           << static_cast<int>(ranked_feature[node].end() - feature[node])
  //           << endl;

  return feature[node] == ranked_feature[node].end();
}

// return true if the feature f is true/false in all examples
bool BacktrackingAlgorithm::max_entropy(const int node, const int f) const {
  // auto f{*(feature[node])};
  auto numNeg{P[0][node].count()};
  auto numPos{P[1][node].count()};

  auto me{(pos_feature_count[0][node][f] == numNeg and
           pos_feature_count[1][node][f] == numPos) or
          (pos_feature_count[1][node][f] == 0 and
           pos_feature_count[0][node][f] == 0)};

  // std::cout << "e" << me << ": " << pos_feature_count[0][node][f] << "/"
  //           << numNeg << " and " << pos_feature_count[1][node][f] << "/"
  //           << numPos << endl;

  return me;
}

// return true if the feature f classifies all examples
bool BacktrackingAlgorithm::null_entropy(const int node, const int f) const {
  // auto f{*(feature[node])};
  auto numNeg{P[0][node].count()};
  auto numPos{P[1][node].count()};

  return (pos_feature_count[0][node][f] == numNeg and
          pos_feature_count[1][node][f] == 0) or
         (pos_feature_count[1][node][f] == numPos and
          pos_feature_count[0][node][f] == 0);
}

void BacktrackingAlgorithm::store_new_best() {

  for (auto i{0}; i < num_node; ++i)
    best_child[i] = -1;

  for (auto i{1}; i < num_node; i += 2) {
    best_child[parent[i]] = i;
    // cout << "child of " << parent[i] << " is " << i << endl;
  }

  for (auto i{0}; i < num_node; ++i) {
    if (i == 0 or best_child[i] >= 0)
      best_feature[i] = *(feature[i] - 1);
    else
      best_feature[i] = (P[1][i].count() > P[0][i].count() ? -1 : -2);

    // cout << "feat of " << i << ": " << best_feature[i] << endl;
  }

  Tree T;
  for (auto i{0}; i < solutionTreeSize(); ++i) {
    T.addNode(solutionChild(i, true), solutionFeature(i));
  }

  // cout << T << endl;

  // auto e{T.predict(data)};
  //
  // auto ie{error()};
  //
  // if(e != ie) {
  //
  //
  // 	cout << "error = " << ie << " / observed = " << e << endl;
  //
  // 	for(auto i : blossom)
  // 	{
  // 		cout << i << ": " << P[0][i].count() << "/" << P[1][i].count()
  // <<
  // endl;
  //
  //
  // 		// for(auto x : P[0][i])
  //
  // 	}
  //
  // 	exit(1);
  //
  // }

  if (options.verified)
    assert(T.predict(data) == error());

  // for (auto i{0}; i < num_node; ++i) {
  // 		if(i % 2) {
  // 			best_child[parent[i]] = i;
  // 			cout << "child of " << parent[i] << " is " << i << endl;
  // 		}
  //   if (parent[i] >= 0)
  //     best_feature[i] = *feature[i];
  //   else
  //     best_feature[i] = (P[0][i].count() > P[0][i].count() ? -1 : -2);
  // }

  print_new_best();
}

int BacktrackingAlgorithm::solutionTreeSize() const { return ub_node; }

int BacktrackingAlgorithm::solutionError() const { return ub_error; }

int BacktrackingAlgorithm::solutionFeature(const int i) const {
  return best_feature[i];
}

int BacktrackingAlgorithm::solutionChild(const int i, const bool t) const {
  return best_child[i] + 1 - t;
}

void BacktrackingAlgorithm::print_new_best() const {

  cout << " size=" << left << setw(10) << ub_node << " depth=" << left
       << setw(10) << ub_depth << "  accuracy=" << setw(10)
       << (1.0 -
           static_cast<double>(ub_error) / static_cast<double>(data.count()))
       // << " backtracks=" << setw(10) << num_backtracks
       << " choices=" << setw(10) << search_size << " restarts=" << setw(10)
       << num_restarts << " time=" << setw(10) << cpu_time()

       << right << endl;
}

bool BacktrackingAlgorithm::dead_end(const int branching_node) {

  if (branching_node >= 0 and
      (is_optimal(branching_node) or no_feature(branching_node) or
       max_entropy(branching_node, *(feature[branching_node])))) {

    if (PRINTTRACE) {
      for (auto i{0}; i < num_node; ++i)
        cout << " ";
      if (is_optimal(branching_node))
        cout << "subtree is optimal!\n";
      else
        cout << "domain of " << branching_node << ":"
             << *(feature[branching_node]) << " is exhausted ("
             << (is_optimal(branching_node)
                     ? "optimal"
                     : (no_feature(branching_node)
                            ? "no more feature"
                            : (max_entropy(branching_node,
                                           *(feature[branching_node]))
                                   ? "closed"
                                   : "unknown")))
             << ")!\n";
    }

    return true;
  }

  if (not blossom.count()) {

    assert(branching_node < 0);

    // this is a solution, and the tree is correct!!!
    ub_error = 0;
    ub_node = num_node;
    ub_depth = max_depth.back();

    // if (options.verbosity >= Options::NORMAL) {

    if (PRINTTRACE) {
      for (auto i{0}; i < num_node; ++i)
        cout << " ";
      cout << "tree is correct!\n";
    }

    store_new_best();
    // }

    return true;
  }

  if (size_matters) {
    if (num_node >= ub_node) {

      if (PRINTTRACE) {
        for (auto i{0}; i < num_node; ++i)
          cout << " ";
        cout << "tree is max-sized \n";
      }

      // new tree, check if the accuracy is better
      auto err{error()};
      if (err < ub_error) {
        ub_error = err;
        ub_depth = max_depth.back();

        // if (options.verbosity >= Options::NORMAL) {
        store_new_best();
        // }
      }

      return true;
    }
  } else {
    if (ub_error == 0 and num_node >= ub_node and
        max_depth.back() == ub_depth) {

      assert(branching_node < 0);

      if (PRINTTRACE) {
        for (auto i{0}; i < num_node; ++i)
          cout << " ";
        cout << "tree is max-sized \n";
      }

      //
      //
      // // new tree, check if the accuracy is better
      // auto err{error()};
      // if (err < ub_error or (err == ub_error and max_depth.back() <
      // ub_depth)) {
      //   ub_error = err;
      //   ub_depth = max_depth.back();
      //
      //   if (options.verbosity >= Options::NORMAL) {
      //
      // 		    if (options.verbosity >= Options::YACKING) {
      // 		      for (auto i{0}; i < num_node; ++i)
      // 		        cout << " ";
      // 					cout << "tree is max-sized \n";
      // 		    }
      //
      //     print_new_best();
      //   }
      // }

      return true;
    }

    auto depth_limit{true};
    auto i{blossom.begin()};
    while (depth_limit and i != blossom.end())
      depth_limit = (depth[*i++] == ub_depth);

    if (depth_limit) {

      // new tree, check if the accuracy is better
      auto err{error()};
      if (err < ub_error or (err == ub_error and num_node < ub_node)) {
        ub_error = err;
        ub_node = num_node;

        // if (options.verbosity >= Options::NORMAL) {

        if (PRINTTRACE) {
          for (auto i{0}; i < num_node; ++i)
            cout << " ";
          cout << "tree is of maximum depth\n";
        }

        store_new_best();
        // }
      }

      return true;
    }
  }

  if (ub_error > 0) {

    auto lb{error_lower_bound()};
    if (lb >= ub_error) {
      if (PRINTTRACE) {
        for (auto i{0}; i < num_node; ++i)
          cout << " ";
        cout << "OPTIMISTIC LOWER BOUND (ERROR) = " << lb << "/" << ub_error
             << endl;
      }
      return true;
    }

  } else {

    auto lb{node_lower_bound()};
    if ((not size_matters and max_depth.back() >= ub_depth) or lb > ub_node) {

      if (PRINTTRACE) {
        for (auto i{0}; i < num_node; ++i)
          cout << " ";
        cout << "OPTIMISTIC LOWER BOUND (NODE) = " << lb << "/" << ub_node
             << endl;
      }

      return true;
    }
  }

  return false;
}

bool BacktrackingAlgorithm::notify_solution() {
  // cout << "solution (" << current_error << ")!\n";

  if (current_error < ub_error) {
    ub_error = current_error;

    assert(bourgeon.size() == num_node);

    ub_node = bourgeon.size();
		
		// cout << "print\n";
		
    print_new_best();
  }

  return backtrack();
}

// bool BacktrackingAlgorithm::perfect() {
//
//   cout << "prefect?\n";
//
//   auto perfect{current_error == 0};
//
//   if (perfect)
//     notify_solution();
//
//   return perfect;
// }
//
// bool BacktrackingAlgorithm::bottom() {
//
//   cout << "bottom (" << ub_depth << ")?\n";
//
//   // auto depth_limit{true};
//   // auto i{blossom.begin()};
//   // while (depth_limit and i != blossom.end())
//   //   depth_limit = (depth[*i++] == ub_depth);
//
//   if (depth_limit)
//     notify_solution();
//
//   return depth_limit;
// }

bool BacktrackingAlgorithm::fail() {
  // auto lb{current_error};
  //
  // for (auto i : blossom)
  //   if (depth[i] == ub_depth - 1)
  //     lb += (get_feature_error(i, *(feature[i])) - error(i));
  //   else
  //     lb -= error(i);
  //
  // if(PRINTTRACE and lb >= ub_error)
  // {
  // 	cout << "lb fail: " << lb << " >= " << ub_error << endl;
  // }
  //
  // return lb >= ub_error;

  return false;
}

void BacktrackingAlgorithm::resize_n(const int k) {

  // cout << "resize\n";

  optimal.resize(k, false);
  bourgeon.reserve(k);

  resize(k);

  // best_error.resize(k, data.count());
  // best_left_child.resize(k, -1);
  // best_right_child.resize(k, -1);

  nodes.reserve(k);

  left_child.resize(k, -1);
  right_child.resize(k, -1);

  for (auto y{0}; y < 2; ++y)
    while (P[y].size() < k)
      P[y].addNode();
}

void BacktrackingAlgorithm::branch(const int node, const int f) {

  num_node += 2;
  if (parent.size() < num_node)
    resize_n(num_node);

  // assert(nodes.bend() - nodes.bbegin() > 1);

  int c[2] = {*bourgeon.bbegin(), *(bourgeon.bbegin() + 1)};

  // partition
  for (auto y{0}; y < 2; ++y) {
    assert(P[y][node].count() > 0);

    // cout << "branch on " << node << " (" << y << ") with feature " << f <<
    // "(" << P[y].size() << ")\n";

		assert(f < data.numFeature());

    P[y].branch(node, c[1], c[0],
                [&](const int x) { return data.ithHasFeature(y, x, f); });
  }

  // cout << P[0][node].count() << "/" << P[1][node].count() << endl;
  // cout << P[0][num_node-2].count() << "/" << P[1][num_node-2].count() <<
  // endl;
  // cout << P[0][num_node-1].count() << "/" << P[1][num_node-1].count() <<
  // endl;

  // cout << "children " << left_child.size() << "/" << right_child.size() <<
  // "\n";

  if (PRINTTRACE)
    cout << "branch on " << node << " with " << f << " children: " << c[0]
         << " (" << P[0][c[0]].count() << "/" << P[1][c[0]].count() << ") and  "
         << c[1] << "(" << P[0][c[1]].count() << "/" << P[1][c[1]].count()
         << ")" << endl;

  left_child[node] = c[0];
  right_child[node] = c[1];

  // auto nb{blossom.count()};

  // cout << "before: " << blossom << endl;

  decision.push_back(node);
  bourgeon.remove_front(node);

  // if (num_node < parent.size()) {
  for (auto i{0}; i < 2; ++i) {
    // nodes.add(c[i]);
    parent[c[i]] = node;
    depth[c[i]] = depth[node] + 1;
    bourgeon.add(c[i]);

    // if (P[0][c[i]].count() > 0 and P[1][c[i]].count() > 0 and
    //     depth[c[i]] < ub_depth) {
    //   blossom.add(c[i]);
    // } else {
    // 	cbest_error[c[i]] = 0;
    // }
    if (P[0][c[i]].count() == 0 or P[1][c[i]].count() == 0 or
        depth[c[i]] >= ub_depth) {
      bourgeon.remove_front(c[i]);
      cbest_error[c[i]] = error(c[i]);
    }
  }

  // cout << "after: " << blossom << endl;

  // if (blossom.count() >= nb) {
  //
  //   // cout << blossom.count() << " >= " << nb << endl;
  //
  //   max_depth.push_back(max(max_depth.back(), depth[node] + 1));
  // } else {
  //   max_depth.push_back(max_depth.back());
  // }

  // assert(depth[node] + 1 <= max_depth.back());

  // }
  // blossom.remove_front(node);

  for (auto y{0}; y < 2; ++y) {
    auto smallest{P[y][c[1]].count() < P[y][c[0]].count()};

    count_by_example(c[smallest], y);

    deduce_from_sibling(c[1 - smallest], c[smallest], y);

    //
    // auto smallest_child{
    //     (P[y][c[1]].count() > P[y][c[0]].count() ? c[0] : c[1])};
    // auto largest_child{(c[1] == smallest_child ? c[0] : c[1])};

    // // cout << "-> " << smallest_child << " and " << largest_child <<
    // // endl;
    //
    // count_by_example(smallest_child, y);
    //
    // deduce_from_sibling(largest_child, smallest_child, y);
  }
  // sort_features(child-1);
  // sort_features(child);

  for (auto i{0}; i < 2; ++i) {
    feature[c[i]] = ranked_feature[c[i]].begin();
    sort_features(c[i]);
  }
  // feature[c[1]] = ranked_feature[c[1]].begin();
  // sort_features(c[1]);

  current_error += (error(c[0]) + error(c[1]) - error(node));

  // num_node += 2;
}

void BacktrackingAlgorithm::expend() {

  // cout << "expend ";
  //
  // for (auto i{0}; i < blossom.count(); ++i)
  //   cout << " " << blossom[i];
  // cout << " |";
  // for (auto i : blossom)
  //   cout << " " << i;
  // cout << endl;

  auto selected_node{backtrack_node};

  if (selected_node < 0) {
    auto r{random_generator() % bourgeon.count()};
    // cout << r << " -> " << blossom[r] << endl;
    selected_node = bourgeon[r];
    cbest_error[selected_node] = data.count(); // error(selected_node);
  } else {
		
		assert(left_child[selected_node] >= 0);
		assert(right_child[selected_node] >= 0);
		
    // if we backtrack to this node, it is not optimal, but its children are
    auto err{cbest_error[left_child[selected_node]] +
             cbest_error[right_child[selected_node]]};

    if (PRINTTRACE)
      cout << "new best for node " << selected_node << "? feat"
           << *feature[selected_node] << ", error=" << err << " ("
           << left_child[selected_node] << "/" << right_child[selected_node]
           << ")" << endl;

    if (err < cbest_error[selected_node]) {
      cbest_error[selected_node] = err;
      cbest_feature[selected_node] = *feature[selected_node];

      if (PRINTTRACE)
        cout << "new best for node " << selected_node
             << ": feat=" << cbest_feature[selected_node]
             << ", error=" << cbest_error[selected_node] << endl;
    }

    ++feature[backtrack_node];
  }

  branch(selected_node, *(feature[selected_node]));
  // ++feature[selected_node];

  if (depth[selected_node] == ub_depth - 1) {
    optimal[selected_node] = true;

    cbest_error[selected_node] =
        error(left_child[selected_node]) + error(right_child[selected_node]);
    cbest_feature[selected_node] = *feature[selected_node];

    if (PRINTTRACE)
      cout << "node " << selected_node
           << " has max depth: opt feat=" << cbest_feature[selected_node]
           << ", error=" << cbest_error[selected_node] << endl;
  }

  backtrack_node = -1;
}

void BacktrackingAlgorithm::prune(const int node) {

  // cout << "prune\n";
  //
  if (PRINTTRACE)
    cout << "PRUNE " << node << endl;

  if (node >= 0 and parent[node] >= 0) {

    // assert(blossom.index(node) >= blossom.size());

    optimal[node] = false;
    parent[node] = -1;

    if (left_child[node] >= 0 and parent[left_child[node]] == node) {
      assert(right_child[node] >= 0);

      prune(left_child[node]);
      left_child[node] = -1;
      prune(right_child[node]);
      right_child[node] = -1;
    } else {
      current_error -= error(node);

      // cout << "error -= " << error(node) << endl;
    }

    bourgeon.add(node);
    bourgeon.remove_back(node);
    // nodes.remove_back(node);

    --num_node;
  }

  // if(nodes.count() != num_node)
  // {
  // 	cout << nodes << endl;
  // 	cout << nodes.size() << "/" << num_node << endl;
  // }

  assert(bourgeon.size() == num_node);
  //
}

bool BacktrackingAlgorithm::backtrack() {

  ++num_backtracks;

  // cout << "backtrack\n";
  if (backtrack_node == 0)
    return false;

  // auto backtrack_ptr{blossom.frbegin()};

  if (PRINTTRACE) {
    for (auto i{0}; i < num_node; ++i)
      cout << " ";
    cout << "backtrack to";
  }

  do {
    backtrack_node = decision.back();
    decision.pop_back();

    if (PRINTTRACE)
      cout << " " << backtrack_node;

  } while (optimal[backtrack_node]);

  if (PRINTTRACE)
    cout << endl;

  // if there aren't any, backtrack once step further
  if (no_feature(backtrack_node)) {

    // prune(backtrack_node);

    feature[backtrack_node] = ranked_feature[backtrack_node].begin();

    optimal[backtrack_node] = true;

		if (PRINTTRACE)
    	cout << "end domain for " << backtrack_node << "! ==> set optimal\n";

    return backtrack();
  }

  // backtrack node is available for branching
  bourgeon.add(backtrack_node);
  current_error += error(backtrack_node);

  // cout << "error += " << error(backtrack_node) << endl;

  // make sure that children are not assigned
  prune(left_child[backtrack_node]);
  prune(right_child[backtrack_node]);
	// left_child[backtrack_node] = -1;
	// right_child[backtrack_node] = -1;

  // search can continue
  return true;
}

void BacktrackingAlgorithm::new_search() {
  sort_features(0);

  optimal[0] = false;

  backtrack_node = -1;

  while (true) {

    ++search_size;

    if (PRINTTRACE) {

      cout << "ub (error) = " << ub_error << "; ub (depth) = " << ub_depth
           << endl;
      cout << "nodes: ";
      for (auto d{bourgeon.fbegin()}; d != bourgeon.fend(); ++d) {
        cout << setw(3) << *d << (optimal[*d] or error(*d) == 0 ? "*" : " ");
        // assert(*d == 0 or bourgeon.index(parent[*d]) < bourgeon.index(*d));

        // assert(nodes.contain(*d));
      }
      cout << "| ";
      for (auto b : bourgeon) {
        cout << setw(3) << b << (optimal[b] ? "*" : " ");
        // assert(nodes.contain(b));
        // assert(b == 0 or bourgeon.index(parent[b]) < bourgeon.index(b));
      }
      cout << "| ";
      for (auto d{bourgeon.bbegin()}; d != bourgeon.bend(); ++d) {
        cout << setw(3) << *d << " ";
        // assert(parent[*d]>=0 == nodes.contain(*d));
        // assert(parent[*d]<0 or bourgeon.index(parent[*d]) <
        // bourgeon.index(*d));
      }

      // cout << endl << "optim: ";
      // for (auto d{bourgeon.fbegin()}; d != bourgeon.fend(); ++d) {
      //   cout << setw(3) << optimal[*d] << " ";
      // }
      // cout << "  ";
      // for (auto b{0}; b < bourgeon.count(); ++b) {
      //   cout << setw(3) << optimal[b] << " ";
      //   // cout << "    ";
      //   // assert(!optimal[b]);
      // }
      // cout << "  ";
      // for (auto d{bourgeon.bbegin()}; d != bourgeon.bend(); ++d) {
      //   cout << setw(3) << optimal[*d] << " ";
      // }
      cout << endl << "parent ";
      for (auto d{bourgeon.fbegin()}; d != bourgeon.fend(); ++d) {
        cout << setw(3) << parent[*d] << " ";
        // assert(*d == 0 or parent[left_child[parent[*d]]] == parent[*d]);
        // assert(*d == 0 or parent[right_child[parent[*d]]] == parent[*d]);
      }
      cout << "  ";
      for (auto b : bourgeon) {
        cout << setw(3) << parent[b] << " ";
        // assert(b == 0 or parent[left_child[parent[b]]] == parent[b]);
        // assert(b == 0 or parent[right_child[parent[b]]] == parent[b]);
      }
      // cout << "  ";
      // for (auto d{bourgeon.bbegin()}; d != bourgeon.bend(); ++d) {
      //   cout << "    ";
      //   // assert(not nodes.contain(*d) or
      //   //        parent[left_child[parent[*d]]] == parent[*d]);
      //   // assert(not nodes.contain(*d) or
      //   //        parent[right_child[parent[*d]]] == parent[*d]);
      // }
      // auto total_error{0};
      cout << endl << "depth: ";

      for (auto d{bourgeon.fbegin()}; d != bourgeon.fend(); ++d) {
        cout << setw(3) << depth[*d] << " ";
      }
      cout << "  ";
      for (auto b : bourgeon) {
        cout << setw(3) << depth[b] << " ";
      }
      // cout << "  ";
      // for (auto d{bourgeon.bbegin()}; d != bourgeon.bend(); ++d) {
      //   cout << setw(3) << depth[*d] << " ";
      // }
      cout << endl << "error: ";
      for (auto d{bourgeon.fbegin()}; d != bourgeon.fend(); ++d) {
        cout << setw(3) << error(*d) << " ";
      }
      cout << "  ";
      for (auto b : bourgeon) {
        cout << setw(3) << error(b) << " ";
        // total_error += error(b);
      }
      // cout << "  ";
      // for (auto d{bourgeon.bbegin()}; d != bourgeon.bend(); ++d) {
      //   if (nodes.contain(*d)) {
      //     cout << setw(3) << error(*d) << " ";
      //     // total_error += error(*d);
      //   } else {
      //     cout << "    ";
      //   }
      // }
      cout << endl << "cbest: ";
      for (auto d{bourgeon.fbegin()}; d != bourgeon.fend(); ++d) {
        cout << setw(3) << cbest_error[*d] << " ";
      }
      cout << "  ";
      for (auto b : bourgeon) {
        cout << setw(3) << cbest_error[b] << " ";
        // total_error += cbest_error[b];
      }
      // cout << "  ";
      // for (auto d{bourgeon.bbegin()}; d != bourgeon.bend(); ++d) {
      //   if (nodes.contain(*d)) {
      //     cout << setw(3) << cbest_error[*d] << " ";
      //     // total_error += error(*d);
      //   } else {
      //     cout << "    ";
      //   }
      // }
      cout << endl;

      for (auto b : bourgeon) {
        assert(not optimal[b]);
      }

      auto total_error{0};
      for (auto d{bourgeon.fbegin()}; d != bourgeon.fend(); ++d) {
        // cout << setw(3) << *d << " ";
        // assert(*d == 0 or bourgeon.index(parent[*d]) < bourgeon.index(*d));
        // assert(nodes.contain(*d));
        assert(*d == 0 or parent[left_child[parent[*d]]] == parent[*d]);
        assert(*d == 0 or parent[right_child[parent[*d]]] == parent[*d]);
				if(left_child[*d] < 0)
					total_error += error(*d);
      }
      // cout << "| ";
      for (auto b : bourgeon) {
        // cout << setw(3) << b << " ";
        // assert(nodes.contain(b));
        // assert(b == 0 or bourgeon.index(parent[b]) < bourgeon.index(b));
        assert(b == 0 or parent[left_child[parent[b]]] == parent[b]);
        assert(b == 0 or parent[right_child[parent[b]]] == parent[b]);
        total_error += error(b);
      }

      for (auto d : decision) {
        assert((d == 0 or parent[d] >= 0));
        assert(bourgeon.index(d) < bourgeon.size());
      }

      // cout << "| ";
      // for (auto d{bourgeon.bbegin()}; d != bourgeon.bend(); ++d) {
      //   // cout << setw(3) << *d << (nodes.contain(*d) ? "*" : " ");
      //   assert(parent[*d] >= 0 == (bourgeon.index(*d) < bourgeon.size()));
      //   // assert(parent[*d] < 0 or bourgeon.index(parent[*d]) <
      //   bourgeon.index(*d));
      //   assert(not nodes.contain(*d) or
      //          parent[left_child[parent[*d]]] == parent[*d]);
      //   assert(not nodes.contain(*d) or
      //          parent[right_child[parent[*d]]] == parent[*d]);
      //   if (nodes.contain(*d))
      //     total_error += error(*d);
      // }

      // if (current_error != total_error) {
      //   cout << "discrepancy in error " << current_error << "/" << total_error
      //        << endl;
      //   exit(1);
      // }
    }

    if (bourgeon.empty()) {
      if (not notify_solution())
        break;
    } else if (fail()) {
      if (not backtrack())
        break;
    } else {
      expend();
    }
  }

  print_new_best();
}

void BacktrackingAlgorithm::search() {
  // const int kbest, const double p,
  //   														const
  //   int
  //   max_num_node)
  //   {

  int kbest{options.width};
  int p{static_cast<int>(1000 * options.focus)};

  // const int max_num_node) {

  // auto limit{100000000};
  // size_t search_size{0};

  // ub_node = max_num_node;

  // if (options.verbosity >= Options::YACKING)
  //   cout << "start search\n";

  sort_features(0);

  auto branching_node{-1};

  double restart_base{static_cast<double>(options.restart_base)};
  size_t restart_limit{static_cast<size_t>(restart_base)};
  double restart_factor{options.restart_factor};
  // size_t restart_divisor{1024};

  while (true) {

#ifdef DEBUG_MODE
    if (debug_sol != NULL) {
      // for(auto i{blossom.fbegin()}; i!=blossom.fend(); ++i)
      // {
      // 	cout << setw(3) << *i;
      // }
      // cout << endl;
      // for(auto i{blossom.fbegin()}; i!=blossom.fend(); ++i)
      // {
      // 	cout << setw(3) << *(feature[*i]-1);
      // }
      // cout << endl;
      // for(auto i{blossom.fbegin()}; i!=blossom.fend(); ++i)
      // {
      // 	cout << setw(3) << debug_sol->getFeature(*i);
      // }
      // cout << endl;

      for (auto i{blossom.fbegin()}; i != blossom.fend(); ++i) {
        auto af{feature[*i] - 1};

        if (*af == debug_sol->getFeature(*i))
          continue;
        else {
          for (auto f{ranked_feature[*i].begin()}; f != af; ++f) {
            if (*f == debug_sol->getFeature(*i)) {
              cout << "missed the debug solution at cp " << search_size << "\n";
              exit(1);
            }
          }

          break;
        }

        // cout << *i << ": (" << debug_sol->getFeature(*i) << ") ";
        // for (auto f{ranked_feature[*i].begin()}; f != af; ++f) {
        //   cout << " " << *f;
        // }
        // cout << " | " << *af << " |";
        // for (auto f{af + 1}; f != ranked_feature[*i].end(); ++f) {
        //   cout << " " << *f;
        // }
        // cout << endl;
      }
      // cout << endl;
    }
#endif

    // auto md{0};
    // for(auto i : blossom) {
    // 	if(depth[i] > md)
    // 		md = depth[i];
    // 	// cout << " " << depth[i];
    // }
    // for(auto i{blossom.fbegin()}; i!=blossom.fend(); ++i) {
    // 	if(depth[*i] > md)
    // 		md = depth[*i];
    // 	// cout << " *" << depth[*i];
    // }
    // // cout << " // " << max_depth.back() << endl;
    // assert(md == max_depth.back());

    ++search_size;
    if (num_backtracks == restart_limit) {
      // cout << "restart (" << search_size << ")!\n";

      restart_base *= restart_factor;
      // restart_base /= restart_divisor;
      restart_limit += static_cast<size_t>(restart_base);

      // branching_node = -1;

      // cout << "restart (limit=" << static_cast<size_t>(restart_base) <<
      // ")\n";

      ++num_restarts;
      clear(branching_node);
    }

    if (PRINTTRACE) {

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
      cout << endl;
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << is_optimal(*d) << " ";
      }
      cout << endl;

      // for (auto i{0}; i < cbest_error.size(); ++i)
      // 	cout << setw(3) << i << " " ;
      // cout << endl;
      //       for (auto i{0}; i < cbest_error.size(); ++i) {
      // 	cout << setw(3) << cbest_error[i];
      //         if (is_optimal(i))
      //         //   cout << " " << setw(3) << cbest_error[i];
      //         // else
      //           cout << "*";
      // 	else
      // 		cout << " ";
      //       }
      //       cout << endl;
      //       for (auto i{0}; i < cbest_feature.size(); ++i) {
      // 	cout << setw(3) << cbest_feature[i];
      //         if (is_optimal(i))
      // 		cout << "*";
      //         else
      //           cout << " ";
      //       }
      //       cout << endl;
    }

    if (PRINTTRACE) {
      for (auto i{0}; i < num_node; ++i)
        cout << " ";
      cout << "ub = " << ub_error << "; ";

      if (ub_depth == INFTY)
        cout << "inf";
      else
        cout << ub_depth;

      cout << "; ";

      if (ub_node == INFTY)
        cout << "inf";
      else
        cout << ub_node;

      cout << "; leaf:";

      for (auto i : blossom)
        cout << " " << i << " (" << depth[i] << ", " << P[0][i].count() << "/"
             << P[1][i].count() << ")";

      cout << " " << search_size << "\n";
    }

    // auto

    //     auto end_branch{not blossom.count()};
    //
    //     if (end_branch) {
    //       if (options.verbosity >= Options::YACKING) {
    //         cout << "leaf because correct!\n";
    //       }
    //       ub_error = 0;
    //       ub_node = num_node;
    //
    //       if (options.verbosity >= Options::NORMAL) {
    //         print_new_best();
    //       }
    //
    //     } else {
    //       auto node_limit{num_node >= ub_node};
    // // auto depth_limit{}
    // end_branch = node_limit;
    //       if (end_branch) {
    //
    //         auto err{error()};
    //
    //         if (options.verbosity >= Options::YACKING) {
    //           cout << "leaf because the node limit (" << ub_node
    //                << ") is reached, error=" << err << "\n";
    //         }
    //
    //         // assert(err < ub_error);
    //
    //         if (err < ub_error) {//} or (err == ub_error and max_depth <
    //         ub_depth)) {
    //           ub_error = err;
    // 		// max_depth;
    //
    //           if (options.verbosity >= Options::NORMAL) {
    //             print_new_best();
    //           }
    //         }
    //       }
    //
    //       //
    //       //           num_node + 2 * blossom.count() >= ub_node or ;
    //       //
    //       //       if (end_branch) {
    //       //
    //       // auto err{error()};
    //       //
    //       // assert(err < ub_error);
    //       //
    //       //
    //       //
    //       //
    //       //         if (num_node + 2 * blossom.count() >= ub_node)
    //       //           cout << "blossom because there is no correct subtree
    //       of size
    //       //           less than "
    //       //                << ub_node << endl;
    //       //         else {
    //       //           assert(num_node >= max_num_node);
    //       //           cout << "blossom because the node limit (" <<
    //       max_num_node
    //       //                << ") is reached\n";
    //       //         }
    //       //       }
    //     }
    //
    //     if (not end_branch and branching_node >= 0) {
    //       end_branch = is_optimal(branching_node) or
    //       no_feature(branching_node) or
    //                    max_entropy(branching_node,
    //                    *(feature[branching_node]));
    //
    //       if (options.verbosity >= Options::YACKING) {
    //         if (end_branch) {
    //           for (auto i{0}; i < num_node; ++i)
    //             cout << " ";
    //           if (is_optimal(branching_node))
    //             cout << "subtree is optimal!\n";
    //           else
    //             cout << "domain end!\n";
    //         }
    //       }
    //     }
    //
    //     if (not end_branch) {
    //
    //       if (ub_error > 0) {
    //         auto lb{error_lower_bound()};
    //
    //         if (lb >= ub_error) {
    //           end_branch = true;
    //           if (options.verbosity >= Options::YACKING) {
    //             for (auto i{0}; i < num_node; ++i)
    //               cout << " ";
    //             cout << "OPTIMISTIC LOWER BOUND (ERROR) = " << lb << "/" <<
    //             ub_error
    //                  << endl;
    //           }
    //         }
    //       } else {
    //
    //         auto lb{node_lower_bound()};
    //
    //         if (lb >= ub_node) {
    //           end_branch = true;
    //           if (options.verbosity >= Options::YACKING) {
    //             for (auto i{0}; i < num_node; ++i)
    //               cout << " ";
    //             cout << "OPTIMISTIC LOWER BOUND (NODE) = " << lb << "/" <<
    //             ub_node
    //                  << endl;
    //           }
    //         }
    //       }
    //     }
    //
    //     if (end_branch) {

    if (dead_end(branching_node)) {

      // for (auto i{0}; i < num_node; ++i)
      //   cout << " ";
      // cout << "backtrack over " << branching_node << endl;

      if (num_node == 1)
        break;
      else {
        ++num_backtracks;
        backtrack(branching_node);
        // ++feature[branching_node];
      }

    } else {

      if (branching_node < 0) {

        branching_node = select();

        if (ub_error < data.count())
          random_perturbation(branching_node, kbest, p);

        // sort_features(branching_node);

        if (PRINTTRACE) {
          for (auto i{0}; i < num_node; ++i)
            cout << " ";
          cout << "select new node " << branching_node << endl;
        }
      }

      // for(auto i{0}; i<num_node; ++i)
      // 	cout << " ";
      //     cout << "branch on " << branching_node << endl;
      expend(branching_node);
      sort_features(num_node - 2);
      sort_features(num_node - 1);
      branching_node = -1;
    }

    // cout << end_branch << endl;
    //
    // if (not end_branch and branching_node < 0) {
    //   cout << "continue\n";
    //
    //   branching_node = select();
    //   sort_features(branching_node, kbest, pint);
    //   cout << "select node " << branching_node << endl;
    // }
    //
    // if (end_branch or no_feature(branching_node) or
    //     max_entropy(branching_node, *(feature[branching_node]))) {
    //
    //   cout << "backtrack over " << branching_node;
    //
    //   if (not end_branch)
    //     cout << " (" << end_branch << "/" << no_feature(branching_node) <<
    //     "/"
    //          << max_entropy(branching_node, *(feature[branching_node])) <<
    //          ")";
    //
    //   cout << endl;
    //
    //   if (num_node == 1)
    //     break;
    //   else
    //     backtrack();
    //
    //   // exit(1);
    //
    // } else {
    //   expend(branching_node);
    //   branching_node = -1;
    // }
    // // cout << setw(4) << blossom.size() << " " << accuracy() << endl;

    // if (--limit == 0)
    //   break;
  }

  cout << "search effort: " << search_size << endl;
}

// void BacktrackingAlgorithm::backtrack(const int kbest, const int p) {
//
// }

// void BacktrackingAlgorithm::undo(int &branching_node) {
//   // auto last_node{*(blossom.frbegin())};
//
//   feature[branching_node] = ranked_feature[branching_node].begin();
//
//   branching_node = *(blossom.frbegin());
//   undo(branching_node);
// }

void BacktrackingAlgorithm::undo(const int last_node) {

  // cout << "undo decision expend " << last_node << endl;

  max_depth.pop_back();

  blossom.add(last_node);

  // blossom.remove_back(--num_node);
  // blossom.remove_back(--num_node);
  //
  // unset(num_node);
  // unset(num_node + 1);

  unset(--num_node);
  unset(--num_node);

  for (auto i{0}; i < 2; ++i)
    for (auto j{0}; j < 2; ++j)
      P[i].remNode();

  // cout << "past leaves: " << blossom << endl;
}

void BacktrackingAlgorithm::backtrack(int &branching_node) {

  // cout << branching_node << " / " << optimal.size() << endl;

  auto child{branching_node};

  if (branching_node >= 0) {
    feature[branching_node] = ranked_feature[branching_node].begin();
    // optimal[branching_node] = -1;
  }

  branching_node = *(blossom.frbegin());

  // assert(last_node == branching_node);

  if (PRINTTRACE) {
    for (auto i{0}; i < num_node; ++i)
      cout << " ";
    cout << "backtrack from " << child << " to " << branching_node << endl;
  }

  if (child >= 0) {
    auto sibling{child % 2 ? child + 1 : child - 1};
    auto father{parent[child]};
    assert(parent[sibling] == father);

    auto new_best_error{cbest_error[child] + cbest_error[sibling]};
    auto new_best_size{1 + cbest_size[child] + cbest_size[sibling]};

    if (cbest_error[father] > new_best_error or
        cbest_size[father] > new_best_size) {
      cbest_feature[father] = *feature[father];
      cbest_error[father] = new_best_error;
      cbest_size[father] = new_best_size;

      if (PRINTTRACE)
        cout << "STORE " << father << ":" << cbest_feature[father] << "="
             << cbest_error[father] << endl;
    }

    set_optimal(child);
  }

  undo(branching_node);

  // ++feature[last_node];
}

int BacktrackingAlgorithm::select() {

  // cout << "select a node to expend\n";

  auto selected_node{-1};
  auto max_error{0};

  auto max_reduction{0};

  // for (auto i{blossom.fbegin()}; i!=blossom.fend(); ++i) {// : blossom) {
  for (auto i : blossom)
    if (depth[i] < ub_depth or size_matters) {
      auto error{std::min(P[0][i].count(), P[1][i].count())};

      // auto f{*(feature[i])};
      // auto not_f{f + data.numFeature()};
      //
      // auto err_f{min(get_feature_count(0, i, f), get_feature_count(1, i,
      // f))};
      // auto err_not_f{
      //     min(get_feature_count(0, i, not_f), get_feature_count(1, i,
      //     not_f))};
      //
      // auto reduction{error - err_f - err_not_f};

      auto reduction{error - get_feature_error(i, *(feature[i]))};

      // auto not_f{f + data.numFeature()};
      //
      // auto error_reduction{error - min(get_feature_count(0, i, f),
      // get_feature_count(1, i, f)) - min(get_feature_count(0, i, not_f),
      // get_feature_count(1, i, not_f))};

      // cout << "error of " << i << ": min(" << P[0][i].count() << ", " <<
      // P[1][i].count() << ") best feature: " << f
      //      << " => "
      //      << "min(" << get_feature_count(0, i, f) << ", " <<
      //      get_feature_count(1, i, f) << ") + "
      //      << "min(" << get_feature_count(0, i, not_f) << ", " <<
      //      get_feature_count(1, i, not_f) << ")"
      //      << " reduction: " << reduction << endl;

      if (reduction > max_reduction or
          (reduction == max_reduction and error > max_error)) {
        max_reduction = reduction;
        max_error = error;
        selected_node = i;
      }

      // if (error > max_error) {
      //   max_error = error;
      //   selected_node = i;
      // }
    }

  assert(selected_node >= 0);

  // cout << "--> " << selected_node << endl;

  return selected_node;
}

void BacktrackingAlgorithm::random_perturbation(const int selected_node,
                                                const int kbest, const int p) {
  // if (f_entropy[*(ranked_feature[selected_node].begin())] != 0 and
  if (f_error[*(ranked_feature[selected_node].begin())] != 0 and
      random_generator() % 1000 < p) {

    // cout << "random for " << selected_node << ": "

    swap(*(ranked_feature[selected_node].begin()),
         *(ranked_feature[selected_node].begin() +
           (random_generator() % kbest)));
  }
}

void BacktrackingAlgorithm::sort_features(const int selected_node) {
  if (feature[selected_node] != ranked_feature[selected_node].begin()) {
    cout << selected_node << endl;
    exit(1);
  }

  // cout << "new node: " << selected_node << endl;
  //      << ", compute entropy and rank feature\n";

  auto max_error{min(P[0][selected_node].count(), P[1][selected_node].count())};

  for (auto f{0}; f < data.numFeature(); ++f) {
    // f_entropy[f] = entropy(selected_node, f);
    f_error[f] = get_feature_error(selected_node, f) +
                 max_error * (max_entropy(selected_node, f));
  }

  // cout << "sort\n";

  sort(
      ranked_feature[selected_node].begin(),
      ranked_feature[selected_node].end(),
      // [&](const int a, const int b) { return f_entropy[a] < f_entropy[b]; });
      [&](const int a, const int b) { return f_error[a] < f_error[b]; });

  // if ((selected_node == 1 or selected_node == 2) and *(feature[0] - 1) == 1)
  // {
  //   for (auto f{ranked_feature[selected_node].begin()};
  //        f != ranked_feature[selected_node].end(); ++f)
  //     cout << setw(9) << *f;
  //   cout << endl;
  //
  //   for (auto f{ranked_feature[selected_node].begin()};
  //        f != ranked_feature[selected_node].end(); ++f)
  //     cout << setw(4) << right << get_feature_count(0, selected_node, *f) <<
  //     "/"
  //          << left << setw(4) << get_feature_count(1, selected_node, *f);
  //   cout << endl;
  //
  //   for (auto f{ranked_feature[selected_node].begin()};
  //        f != ranked_feature[selected_node].end(); ++f)
  //     cout << setw(4) << right
  //          << get_feature_count(0, selected_node, *f + data.numFeature()) <<
  //          "/"
  //          << left << setw(4)
  //          << get_feature_count(1, selected_node, *f + data.numFeature());
  //   cout << endl << right;
  //
  //   // for (auto f{ranked_feature[selected_node].begin()};
  //   //      f != ranked_feature[selected_node].end(); ++f)
  //   //   cout << right << setw(9) << setprecision(5) << f_entropy[*f];
  //   // cout << endl;
  //
  //   for (auto f{ranked_feature[selected_node].begin()};
  //        f != ranked_feature[selected_node].end(); ++f)
  //     cout << setw(9) << f_error[*f];
  //   cout << endl;
  // }

  // cout << "select\n" << ranked_feature.size() << " " << feature.size() <<
  // endl;

  // if (f_entropy[*(ranked_feature[selected_node].begin())] != 0 and
  //     random_generator() % 1000 < p)
  //   swap(*(ranked_feature[selected_node].begin()),
  //        *(ranked_feature[selected_node].begin() +
  //          (random_generator() % kbest)));

  // cout << "end select\n";

  // feature[selected_node] = ranked_feature[selected_node].begin();
  // }
}

//
void BacktrackingAlgorithm::expend(const int selected_node) {

  // cout << "f_entropy\n";

  // assert(feature[selected_node] == ranked_feature[selected_node].end());

  // if (feature[selected_node] == ranked_feature[selected_node].begin()) {
  //
  //   cout << "new node, compute entropy\n";
  //
  //   for (auto f{0}; f < data.numFeature(); ++f)
  //     f_entropy[f] = entropy(selected_node, f);
  //
  //   // cout << "sort\n";
  //
  //   sort(ranked_feature[selected_node].begin(),
  //        ranked_feature[selected_node].end(),
  //        [&](const int a, const int b) { return f_entropy[a] < f_entropy[b];
  //        });
  //
  //   // cout << "select\n" << ranked_feature.size() << " " << feature.size()
  //   <<
  //   // endl;
  //
  //   if (f_entropy[*(ranked_feature[selected_node].begin())] != 0 and
  //       random_generator() % 1000 < p)
  //     swap(*(ranked_feature[selected_node].begin()),
  //          *(ranked_feature[selected_node].begin() +
  //            (random_generator() % kbest)));
  //
  //   // cout << "end select\n";
  //
  //   // feature[selected_node] = ranked_feature[selected_node].begin();
  // }
  // selected_feature = *();

  // sort_features(selected_node, kbest, p);

  // cout << "here\n";

  // cout << "branch on " << branching_node << endl;

  // if(num_node < max_num_node) {
  //     if (num_node >= parent.size()) {
  //       resize(num_node);
  // 	}
  // resize(num_node);

  // if(num_node == ub_node - 2)
  // 	cout << "THIS IS GOING TO BE A LEAF!!!\n";

  auto bf{*(ranked_feature[selected_node].begin())};

  auto stop{num_node == ub_node - 2};
  if (not size_matters) {
    stop = depth[selected_node] == ub_depth - 1 or
           (ub_error == 0 and max_depth.back() == ub_depth and
            num_node == ub_node - 2);
  }
  stop = stop or null_entropy(selected_node, bf);

  // if ((size_matters and num_node == ub_node - 2) or
  //     (not size_matters and
  //      (
  //
  //          depth[selected_node] ==
  //              ub_depth - 1 or // this is a leaf because it's depth is
  //              maximum
  //          (ub_error == 0 and max_depth.back() == ub_depth and
  //           num_node == ub_node - 2)) // it is a leaf because error cannot be
  //      // improved and the maximum size and depth
  //      // have been reached
  //      ) or
  //     null_entropy(selected_node, bf) // this is a leaf because it correct
  //     ) {
  if (stop) {

    // cout << "node " << selected_node << " has a null entropy feature:\n" ;
    //
    // cout << " ==> " << *(ranked_feature[selected_node].begin()) << endl;
    //
    // cout << optimal.size() << endl;

    // optimal[selected_node] = parent[selected_node];
    cbest_feature[selected_node] = bf;
    cbest_error[selected_node] =
        min(P[0][selected_node].count(), P[1][selected_node].count());
    cbest_size[selected_node] = 1;

    if (PRINTTRACE)
      cout << "STORE " << selected_node << ":" << cbest_feature[selected_node]
           << "=" << cbest_error[selected_node] << endl;

    set_optimal(selected_node);
  }

  split(selected_node, *(feature[selected_node]));

  // }

  if (PRINTTRACE) {
    for (auto i{0}; i < num_node; ++i)
      cout << " ";
    cout << "expend " << selected_node << " with feature "
         << *(feature[selected_node]) << " ("
         << static_cast<int>(feature[selected_node] -
                             ranked_feature[selected_node].begin())
         << "/" << static_cast<int>(ranked_feature[selected_node].end() -
                                    ranked_feature[selected_node].begin())
         << ") split on " << num_node - 2 << ": " << P[0][num_node - 2].count()
         << "/" << P[1][num_node - 2].count() << " -- " << num_node - 1 << ": "
         << P[0][num_node - 1].count() << "/" << P[1][num_node - 1].count()
         << endl;

    if (search_size > 72690 and selected_node == 0)
      exit(1);

    // 																													<<
    // num_node
    // -
    // 1
    // <<
    // "
    // ("
    // << P[0][num_node - 2].count() << "/" << P[0][num_node - 1].count()
    // << ") (" << P[1][num_node - 2].count() << "/"
    // << P[1][num_node - 1].count() << ")" << endl;
  }

  for (auto y{0}; y < 2; ++y) {
    //
    // auto f{*(feature[selected_node])};
    //     cout << " " << f << ": " << num_node - 2 << "." << num_node - 1 << "
    //     " << get_feature_count(y, selected_node, f) << "-" << P[y][num_node -
    //     2].count() << "/"
    //          << get_feature_count(y, selected_node, f + data.numFeature()) <<
    //          "-" << P[y][num_node - 1].count()
    //          << endl;
    //
    //
    if (P[y][num_node - 2].count() !=
        get_feature_count(y, selected_node, *(feature[selected_node]))) {
      cout << "discrepancy in feature count: "
           << get_feature_count(y, selected_node, *(feature[selected_node]))
           << " != " << P[y][num_node - 2].count() << endl;
      exit(1);
    }
    if (P[y][num_node - 1].count() !=
        get_feature_count(y, selected_node,
                          *(feature[selected_node]) + data.numFeature())) {
      cout << "discrepancy in feature count: "
           << get_feature_count(y, selected_node,
                                *(feature[selected_node]) + data.numFeature())
           << " != " << P[y][num_node - 1].count() << endl;
      exit(1);
    }
  }

  ++feature[selected_node];
  // cout << "current leaves: " << blossom << endl;
}

void BacktrackingAlgorithm::split(const int node, const int f) {

  num_node += 2;
  if (parent.size() < num_node)
    resize(num_node);

  // partition
  for (auto y{0}; y < 2; ++y) {
    assert(P[y][node].count() > 0);

    // cout << "branch on " << node << " (" << y << ") with feature " << f <<
    // "\n";

    P[y].branch(node, [&](const int x) { return data.ithHasFeature(y, x, f); });
  }

  // cout << P[0][node].count() << "/" << P[1][node].count() << endl;
  // cout << P[0][num_node-2].count() << "/" << P[1][num_node-2].count() <<
  // endl;
  // cout << P[0][num_node-1].count() << "/" << P[1][num_node-1].count() <<
  // endl;

  // cout << "parents\n";

  int child;
  auto nb{blossom.count()};

  // cout << "before: " << blossom << endl;

  blossom.remove_front(node);
  // if (num_node < parent.size()) {
  for (auto i{0}; i < 2; ++i) {
    child = num_node - 2 + i;

    // cout << "child " << child << ":" << P[0][child].count() << "/" <<
    // P[1][child].count() << endl;

    parent[child] = node;
    depth[child] = depth[node] + 1;
    // blossom.add(child);
    if (P[0][child].count() > 0 and P[1][child].count() > 0) {
      blossom.add(child);
    }
  }

  // cout << "after: " << blossom << endl;

  if (blossom.count() >= nb) {

    // cout << blossom.count() << " >= " << nb << endl;

    max_depth.push_back(max(max_depth.back(), depth[node] + 1));
  } else {
    max_depth.push_back(max_depth.back());
  }

  // assert(depth[node] + 1 <= max_depth.back());

  // }
  // blossom.remove_front(node);

  for (auto y{0}; y < 2; ++y) {
    auto smallest_child{
        (P[y][child].count() > P[y][child - 1].count() ? child - 1 : child)};
    auto largest_child{(child == smallest_child ? child - 1 : child)};

    // cout << "-> " << smallest_child << " and " << largest_child <<
    // endl;

    count_by_example(smallest_child, y);

    deduce_from_sibling(largest_child, smallest_child, y);
  }
  // sort_features(child-1);
  // sort_features(child);

  // num_node += 2;
}

void BacktrackingAlgorithm::count_by_example(const int node, const int y) {

  // cout << "count " << y << " examples in " << node << " (" <<
  // P[y][node].count() << ")"<< endl;

  auto n{data.numFeature()};

  pos_feature_count[y][node].clear();
  pos_feature_count[y][node].resize(n, 0);

  auto stop = P[y][node].end();
  for (auto i{P[y][node].begin()}; i != stop; ++i)
    for (auto f : example[y][*i])
      ++pos_feature_count[y][node][f];
}

void BacktrackingAlgorithm::deduce_from_sibling(const int node,
                                                const int sibling,
                                                const int y) {

  // cout << "deduce " << y << " examples in " << node << " (" <<
  // P[y][node].count() << ") from " << sibling << endl;

  auto p{parent[node]};
  assert(p == parent[sibling]);

  for (auto f{0}; f < data.numFeature(); ++f)
    pos_feature_count[y][node][f] =
        pos_feature_count[y][p][f] - pos_feature_count[y][sibling][f];
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

void BacktrackingAlgorithm::verify() {
  for (auto s{0}; s < num_node; ++s) {
  }
}

std::ostream &BacktrackingAlgorithm::display(std::ostream &os) const {

  // for (auto y{0}; y < 2; ++y) {
  // 	os << "CLASS " << y << endl;
  // 	os << P[y] << endl;
  // }
  for (auto i : blossom) {
    cout << i << ": " << P[0][i].count() << "/" << P[1][i].count() << endl;
  }

  // os << "here\n";

  // for(auto i{0}; i<parent.size(); ++i) {
  // 	os << "node " << i << ": p=" << parent[i] ;
  //
  // 	if(count[0][i] == 0)
  // 		os << " positive blossom!\n";
  // 	else if(count[1][i] == 0)
  // 		os << " negative blossom!\n";
  // 	else {
  // 		os << endl << "neg:";
  // 		auto j{head[0][i]};
  // 		auto l{tail[0][i]};
  //
  // 		while(true)
  // 		{
  // 			os << " " << j;
  // 			if(j == l) break;
  // 			j = next[j];
  // 		}
  //
  // 		os << endl << "pos:";
  // 		 j = head[1][i];
  // 		 l = tail[1][i];
  //
  //  			while(true)
  //  			{
  //  				os << " " << j;
  //  				if(j == l) break;
  //  				j = next[j];
  //  			}
  //
  // 		os << endl;
  //
  // 	}
  //
  // }

  return os;
}

std::ostream &operator<<(std::ostream &os, const BacktrackingAlgorithm &x) {
  return x.display(os);
}

}