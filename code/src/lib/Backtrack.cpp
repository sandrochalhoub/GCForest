
#include "Backtrack.hpp"

#include <fstream>

// #define DEBUG_GINI

namespace primer {

template<typename T>
T min_positive() {
	return static_cast<int>(static_cast<T>(1) - 2*FLOAT_PRECISION) + 2*FLOAT_PRECISION;
}

template<typename T>
T is_null(const T& x) {
	return equal<T>(x, 0);
}

// ===== CardinalityError

template <typename E_t>
void CardinalityError<E_t>::count_by_example(const int node,
                                             const int y) const {
  auto n{algo.num_feature};

  algo.pos_feature_frequency[y][node].clear();
  algo.pos_feature_frequency[y][node].resize(n, 0);

  auto stop = algo.P[y][node].end();
  for (auto i{algo.P[y][node].begin()}; i != stop; ++i)
    for (auto f : algo.example[y][*i])
      ++algo.pos_feature_frequency[y][node][f];
}

template <typename E_t>
E_t CardinalityError<E_t>::node_error(const int i) const {
  return std::min(algo.P[0][i].count(), algo.P[1][i].count());
}

template <typename E_t>
E_t CardinalityError<E_t>::get_total(const int y, const int n) const {
  return algo.P[y][n].count();
}


// ===== WeightedError

template <typename E_t> void WeightedError<E_t>::update_node(const int n) {

  for (size_t y{0}; y < 2; ++y) {
    if (weight_total[y].size() <= n)
      weight_total[y].resize(n + 1);
    weight_total[y][n] = 0;
  }

  if (n == 0) {

    for (size_t y{0}; y < 2; ++y)
			for (auto s : algo.P[y][n])
        weight_total[y][n] += weights[y][s];

  } else {
    // get parent data
    int p = algo.parent[n];
    int pfeat =
        *algo.feature[p] + (algo.child[0][p] == n ? algo.num_feature : 0);

    weight_total[0][n] = algo.get_feature_frequency(0, p, pfeat);
    weight_total[1][n] = algo.get_feature_frequency(1, p, pfeat);
  }

}

template <typename E_t>
void WeightedError<E_t>::set_weight(const int y, const size_t i, const E_t weight) {
  weights[y][i] = weight;
}

template <typename E_t>
void WeightedError<E_t>::count_by_example(const int node, const int y) const {
  auto n{algo.num_feature};

  algo.pos_feature_frequency[y][node].clear();
  algo.pos_feature_frequency[y][node].resize(n, 0);

  auto stop = algo.P[y][node].end();
  for (auto i{algo.P[y][node].begin()}; i != stop; ++i)
    for (auto f : algo.example[y][*i]) {
      algo.pos_feature_frequency[y][node][f] += weights[y][*i];
    }
}

template <typename E_t> E_t WeightedError<E_t>::node_error(const int i) const {
  return std::min(weight_total[0][i], weight_total[1][i]);
}

template <typename E_t>
E_t WeightedError<E_t>::get_total(const int y, const int n) const {
  return weight_total[y][n];
}

template <typename E_t> void WeightedError<E_t>::clear_examples() {
  for (int i = 0; i < 2; ++i) {
    // TODO deleting weights breaks adaboost because it relies on the weights of
    // the previous iteration.
    // weights[i].clear();
  }
}


// ===== BacktrackingAlgorithm

template <template <typename> class ErrorPolicy, typename E_t>
BacktrackingAlgorithm<ErrorPolicy, E_t>::BacktrackingAlgorithm(Wood &w,
                                                               DTOptions &opt)
    : wood(w), options(opt), error_policy(*this) {

  // start_time = cpu_time();

  num_feature = 0;
  // numExample[0] = 0;
  // numExample[1] = 0;

  // statistics and options
  ub_error = INFTY(E_t); //(numExample());
  ub_size = INFTY(size_t);
  ub_depth = options.max_depth;
  size_matters = false;
  actual_depth = 0;

  search_size = 0;
  num_backtracks = 0;
  num_restarts = 0;
  num_solutions = 0;

  seed(options.seed);

  solution_root = -1;

  search_limit = static_cast<size_t>(options.search);

  time_limit = options.time;

  checking_period = 5000;

  interrupted = false;

  // use_entropy = options.feature_strategy != DTOptions::MINERROR;
  feature_criterion = options.feature_strategy;

  restart_limit = options.restart_base;

  restart_base = static_cast<double>(restart_limit);
}

template <template<typename> class ErrorPolicy, typename E_t>
size_t BacktrackingAlgorithm<ErrorPolicy, E_t>::numExample() const {
  return example[0].size() + example[1].size();
}

template <template<typename> class ErrorPolicy, typename E_t>
size_t BacktrackingAlgorithm<ErrorPolicy, E_t>::numFeature() const { return num_feature; }

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::setReverse() {
  for (int y{0}; y < 2; ++y) {
    reverse_dataset[y].resize(num_feature);
    for (int f{0}; f < num_feature; ++f)
      reverse_dataset[y][f].resize(example[y].size(), 0);
    // buffer[y].resize(example[y].size(), 0);
  }
  for (int y{0}; y < 2; ++y)
    for (auto i{0}; i < example[y].size(); ++i)
      for (auto f : example[y][i])
        reverse_dataset[y][f].set(i);
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::setData(const DataSet &data) {

  // cout << "set data\n";

  num_feature = static_cast<int>(data.numFeature());

  f_error.resize(num_feature, 1);
  f_entropy.resize(num_feature, 1);
  f_gini.resize(num_feature, 1);

  for (int y{0}; y < 2; ++y) {
    dataset[y].resize(data.example[y].count());
    example[y].resize(data.example[y].count());
    auto k{0};
    for (auto i : data.example[y]) {
      // cout << k << ":";
      dataset[y][k].resize(num_feature);
      dataset[y][k] = data[i];
      for (auto j{0}; j < num_feature; ++j)
        if (data.hasFeature(i, j)) {
          example[y][k].push_back(j);
          // cout << " " << j;
        }
      ++k;
      // cout << endl;
      error_policy.add_example(y, i);
    }
  }
}

template <template<typename> class ErrorPolicy, typename E_t>
E_t BacktrackingAlgorithm<ErrorPolicy, E_t>::error() const { return ub_error; }

template <template<typename> class ErrorPolicy, typename E_t>
bool BacktrackingAlgorithm<ErrorPolicy, E_t>::limit_out() {
  ++search_size;

  if (time_limit > 0 and (num_backtracks % checking_period) == 0)
    if (cpu_time() >= (time_limit + start_time))
      interrupted = true;
  interrupted = interrupted or (search_limit and num_backtracks > search_limit);
  return interrupted;
}

template <template<typename> class ErrorPolicy, typename E_t>
E_t BacktrackingAlgorithm<ErrorPolicy, E_t>::get_feature_frequency(
    const int y, const int n, const int f) const {
  return (f >= num_feature
              ? error_policy.get_total(y, n) -
                    pos_feature_frequency[y][n][f - num_feature]
              : pos_feature_frequency[y][n][f]);
}

template <template<typename> class ErrorPolicy, typename E_t>
E_t BacktrackingAlgorithm<ErrorPolicy, E_t>::get_feature_error(const int n, const int f) const {
  auto not_f{f + num_feature};
  return min(get_feature_frequency(0, n, f), get_feature_frequency(1, n, f)) +
         min(get_feature_frequency(0, n, not_f),
             get_feature_frequency(1, n, not_f));
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::seed(const int s) { random_generator.seed(s); }

template <template<typename> class ErrorPolicy, typename E_t>
size_t BacktrackingAlgorithm<ErrorPolicy, E_t>::size() { return blossom.size(); }

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::setUbDepth(const size_t u) { ub_depth = u; }

// void BacktrackingAlgorithm<ErrorPolicy, E_t>::setUbNode(const size_t u) { ub_size = u; }

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::setUbError(const E_t u) { ub_error = u; }

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::addSizeObjective() { size_matters = true; }

template <template<typename> class ErrorPolicy, typename E_t>
E_t BacktrackingAlgorithm<ErrorPolicy, E_t>::getUbError() const { return ub_error; }

template <template<typename> class ErrorPolicy, typename E_t>
size_t BacktrackingAlgorithm<ErrorPolicy, E_t>::getUbDepth() const { return ub_depth; }

template <template<typename> class ErrorPolicy, typename E_t>
size_t BacktrackingAlgorithm<ErrorPolicy, E_t>::getUbSize() const { return ub_size; }

template <template<typename> class ErrorPolicy, typename E_t>
E_t BacktrackingAlgorithm<ErrorPolicy, E_t>::node_error(const int i) const {
  return error_policy.node_error(i);
}

template <template<typename> class ErrorPolicy, typename E_t>
bool BacktrackingAlgorithm<ErrorPolicy, E_t>::no_feature(const int node) const {
  return feature[node] == end_feature[node]; // ranked_feature[node].end();
}

// return true if the feature f is true/false in all examples
template <template<typename> class ErrorPolicy, typename E_t>
bool BacktrackingAlgorithm<ErrorPolicy, E_t>::max_entropy(const int node, const int f) const {

  E_t negTotal = error_policy.get_total(0, node);
  E_t posTotal = error_policy.get_total(1, node);

  auto me{(equal<E_t>(pos_feature_frequency[0][node][f], negTotal) and
           equal<E_t>(pos_feature_frequency[1][node][f], posTotal)) or
          (equal<E_t>(pos_feature_frequency[1][node][f], 0) and
           equal<E_t>(pos_feature_frequency[0][node][f], 0))};

  return me;
}

// return true if the feature f classifies all examples
template <template<typename> class ErrorPolicy, typename E_t>
bool BacktrackingAlgorithm<ErrorPolicy, E_t>::null_entropy(const int node, const int f) const {

  E_t negTotal = error_policy.get_total(0, node);
  E_t posTotal = error_policy.get_total(1, node);

  auto ne{(equal<E_t>(pos_feature_frequency[0][node][f], negTotal) and
           equal<E_t>(pos_feature_frequency[1][node][f], 0)) or
          (equal<E_t>(pos_feature_frequency[1][node][f], posTotal) and
           equal<E_t>(pos_feature_frequency[0][node][f], 0))};

  return ne;
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::separator(const string &msg) const {
  cout << setfill('-') << setw((96 - msg.size()) / 2) << "-"
       << "[" << msg << "]" << setw((96 - msg.size()) / 2 + (msg.size() % 2))
       << "-" << endl
       << setfill(' ');
}

template <template <typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::print_new_best() {

  if (not nb)
    cout << endl;
  nb = true;
  E_t total = error_policy.get_total(0, 0) + error_policy.get_total(1, 0) +
              2 * error_offset;

  double t{
      static_cast<double>(static_cast<int>(100.0 * (cpu_time() - start_time))) /
      100.0};

  cout << setprecision(5) << left << "d accuracy=" << setw(7)
       << (1.0 -
           static_cast<double>(error_offset + ub_error) /
               static_cast<double>(total))
       << " error=" << setw(4) << ub_error + error_offset
       << " depth=" << setw(3) << actual_depth << " size=" << setw(3) << ub_size
       // << " backtracks=" << setw(9) << num_backtracks
       << " choices=" << setw(9) << search_size << " restarts=" << setw(4)
       << num_restarts << " mem=" << setw(3) << wood.size() << " time=" << t
       << right << endl;
}

template <template <typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::print_progress() {
	if(not options.progress)
		return;
  int width{82};
  auto ne{feature[0] - ranked_feature[0].begin() + 1};
  if (ne > num_explored) {
    nb = false;
    num_explored = ne;
    auto k{num_explored * width / num_level_zero_feature};

    double p{static_cast<double>(num_explored) * 100.0 /
             static_cast<double>(num_level_zero_feature)};

    double t{static_cast<double>(
                 static_cast<int>(100.0 * (cpu_time() - start_time))) /
             100.0};
    cout << setw(width + 16) << setfill('\b') << "\b" << setw(5)
         << setprecision(3) << setfill(' ') << p << "%"
         << " t=" << left << setprecision(3) << setw(5) << t << right << "[";
    if (k > 0)
      cout << setw(k) << setfill('=') << "=";
    if (k < width)
      cout << setw(width - k) << setfill(' ') << " ";
    cout << setw(1) << "]";
    cout.flush();
    if (k == width) {
      cout << endl;
      num_explored = num_level_zero_feature;
    }
  }
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::resize(const int k) {

  optimal.resize(k, false);
  best_tree.resize(k, -1);
  feature.resize(k);
  end_feature.resize(k);
  blossom.reserve(k);
  depth.resize(k, 0);
  optimal.resize(k, -2);
  parent.resize(k, -1);

  min_size.resize(k, 0);
  min_error.resize(k, 0);
  max_error.resize(k, numExample());
  max_size.resize(k, numeric_limits<int>::max());

  tree_error.resize(k, numExample());
  tree_size.resize(k, numExample());

  auto i{ranked_feature.size()};
  ranked_feature.resize(k);
  for (; i < ranked_feature.size(); ++i) {
    // for (auto f{0}; f < num_feature; ++f)
    for (auto f : relevant_features)
      ranked_feature[i].push_back(f);
    feature[i] = ranked_feature[i].begin();
  }

  for (int y{0}; y < 2; ++y) {
    auto i = pos_feature_frequency[y].size();
    pos_feature_frequency[y].resize(k);
    while (i < pos_feature_frequency[y].size()) {
      pos_feature_frequency[y][i++].resize(num_feature);
    }
  }

  child[0].resize(k, -1);
  child[1].resize(k, -1);

  for (auto y{0}; y < 2; ++y)
    while (P[y].size() < k)
      P[y].addNode();
}

template <template<typename> class ErrorPolicy, typename E_t>
int BacktrackingAlgorithm<ErrorPolicy, E_t>::highest_error_reduction() const {

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

template <template<typename> class ErrorPolicy, typename E_t>
int BacktrackingAlgorithm<ErrorPolicy, E_t>::highest_error() const {

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
      cout << " " << i << ": (" << err << ")";
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

template <template<typename> class ErrorPolicy, typename E_t>
int BacktrackingAlgorithm<ErrorPolicy, E_t>::lowest_error() const {

  auto selected_node{-1};
  auto lowest_error{numExample()};

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
      cout << " " << i << ": (" << err << ")";
#endif

    if (err < lowest_error) {
      lowest_error = err;
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

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::random_perturbation(
    const int node, const int kbest, const int p) {

  auto limit{min(kbest, static_cast<int>(end_feature[node] - feature[node]))};

  if (limit > 1 and random_generator() % 1000 > p) {

    auto rand_inc{random_generator() % limit};

    if (rand_inc > 0) {

      swap(*feature[node], *(feature[node] + rand_inc));
    }
  }
}

// void BacktrackingAlgorithm<ErrorPolicy, E_t>::filter_features(const int node) {
//   for (auto f{end_feature[node] - 1}; f >= feature[node]; --f)
//     if (max_entropy(node, *f))
//       swap(*f, *(--end_feature[node]));
// }

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::sort_features(const int node) {

  switch (feature_criterion) {
  case DTOptions::MINERROR:
    for (auto f{feature[node]}; f != end_feature[node]; ++f)
      f_error[*f] = get_feature_error(node, *f);
    sort(feature[node], end_feature[node],
         [&](const int a, const int b) { return f_error[a] < f_error[b]; });
    break;
  case DTOptions::ENTROPY:
    for (auto f{feature[node]}; f != end_feature[node]; ++f)
      f_entropy[*f] = entropy(node, *f);
    sort(feature[node], end_feature[node],
         [&](const int a, const int b) { return f_entropy[a] < f_entropy[b]; });
    break;
  case DTOptions::GINI:
    for (auto f{feature[node]}; f != end_feature[node]; ++f)
      f_gini[*f] = gini(node, *f);
    sort(feature[node], end_feature[node],
         [&](const int a, const int b) { return (f_gini[a] < f_gini[b]); });
    break;
  }

  if (depth[node] == ub_depth - 1) {
    for (auto f{feature[node]}; f != end_feature[node]; ++f)
      f_error[*f] = get_feature_error(node, *f);
    auto min_error_f{min_element(
        feature[node], end_feature[node],
        [&](const int a, const int b) { return f_error[a] < f_error[b]; })};
    if (f_error[*min_error_f] < f_error[*feature[node]]) {
      swap(*feature[node], *min_error_f);
    }
  }
  //
  //
  // for(auto f{feature[node]}; f!=end_feature[node]; ++f) {
  // 	cout << setw(4) << *f << " " << f_gini[*f] << endl;
  // }
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::count_by_example(const int node, const int y) {
  error_policy.count_by_example(node, y);
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::deduce_from_sibling(
    const int parent, const int node, const int sibling, const int y) {
  for (auto f{0}; f < num_feature; ++f)
    pos_feature_frequency[y][node][f] = pos_feature_frequency[y][parent][f] -
                                        pos_feature_frequency[y][sibling][f];
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::cleaning() {
  if (solution_root < 0) {
    solution_root = wood.grow();
    wood.setFeature(solution_root, *feature[0]);
    int f[2] = {*feature[0] + num_feature, *feature[0]};

    for (auto i{0}; i < 2; ++i)
      wood.setChild(solution_root, i, get_feature_frequency(1, 0, f[i]) >
                                          get_feature_frequency(0, 0, f[i]));

    ub_error = get_feature_error(0, *feature[0]);
    ub_size = 3;
  }
}

template <template<typename> class ErrorPolicy, typename E_t>
Tree BacktrackingAlgorithm<ErrorPolicy, E_t>::getSolution() const { return wood[solution_root]; }

template <template<typename> class ErrorPolicy, typename E_t>
bool BacktrackingAlgorithm<ErrorPolicy, E_t>::store_new_best() {
  if (lt<E_t>(current_error, ub_error) or
      (size_matters and equal<E_t>(current_error, ub_error) and current_size < ub_size)) {

    ++num_solutions;

    // perfect = (ub_error > 0 and current_error == 0);

    ub_error = current_error;
    ub_size = current_size;

    // cout << "solution " << wood.count() << " -> ";

    if (solution_root > 1)
      wood.freeNode(solution_root);

    // cout << wood.count() << " -> ";

    solution_root = copy_solution(0);

    // improvement = true;

    actual_depth = wood.depth(solution_root);

    if (options.verbosity > DTOptions::QUIET)
      print_new_best();

    // Tree T{getSolution()};
    // cout << T << endl;

    if (options.verified) {
      if (ub_size != wood.size(solution_root)) {
        cout << "c warning, wrong tree size!!\n";
      }

      E_t actual_error{0};
      for (auto y{0}; y < 2; ++y)
        for (auto i{0}; i < example[y].size(); ++i)
          actual_error += error_policy.get_weight(y, i) *
                          (wood.predict(solution_root, dataset[y][i]) != y);

      if (not equal<E_t>(ub_error, actual_error)) {
        cout << "c warning, wrong tree accuracy!!\n"
             << std::setprecision(std::numeric_limits<long double>::digits10 +
                                  1)
             << ub_error << " != " << actual_error << endl;
      }
    }

    // cout << blossom << endl << wood[solution_root] << endl;

    // cout << wood.count() << endl;

    return true;
  }

  return false;
}

template <template<typename> class ErrorPolicy, typename E_t>
bool BacktrackingAlgorithm<ErrorPolicy, E_t>::notify_solution(bool &improvement) {
  improvement |= store_new_best();

  return backtrack();
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::prune(const int node) {

#ifdef PRINTTRACE
  if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO)
    cout << "PRUNE " << node << endl;
#endif

  if (node >= 0) {

    // for terminal nodes, remove their contribution to "error" and "size"
    if (depth[node] == ub_depth - 1 or optimal[node]) {

#ifdef PRINTTRACE
      if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO) {
        cout << "-ERROR = " << current_error << " - " << max_error[node]
             << endl;
        cout << "-SIZE = " << current_size << " - " << max_size[node] << endl;
      }
#endif

      current_size -= max_size[node];
      current_error -= max_error[node];
    } else if (blossom.contain(node)) {

#ifdef PRINTTRACE
      if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO) {
        cout << "-ERROR = " << current_error << " - " << node_error(node)
             << endl;
        cout << "-SIZE = " << current_size << " - " << 1 << endl;
      }
#endif

      --current_size;
      current_error -= node_error(node);
    }

    for (auto i{0}; i < 2; ++i)
      if (child[i][node] >= 0)
        prune(child[i][node]);

    blossom.add(node);
    blossom.remove_back(node);
  } else {

#ifdef PRINTTRACE
    if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO) {
      cout << "-SIZE = " << current_size << " - " << 1 << endl;
    }
#endif

    // non terminal nodes contribute 1 to "size"
    --current_size;
  }
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::restart(const bool full) {
  ++num_restarts;

  decision.clear();

  for (auto i{blossom.frbegin()}; i != blossom.frend(); ++i)
    if (*i != 0 and best_tree[*i] > 1) {
      wood.freeNode(best_tree[*i]);
      best_tree[*i] = -1;
    }

  prune(child[0][0]);
  prune(child[1][0]);

  current_error = node_error(0);
  current_size = 1;
  blossom.add(0);

  backtrack_node = -1;

  restart_base *= options.restart_factor;
  restart_limit += static_cast<int>(restart_base);

  if (full) {
    feature[0] = ranked_feature[0].begin();
    min_error[0] = 0;
    min_size[0] = (get_feature_error(0, *feature[0]) ? 5 : 3);
  }
}

template <template<typename> class ErrorPolicy, typename E_t>
bool BacktrackingAlgorithm<ErrorPolicy, E_t>::update_upperbound(const int node) {
  E_t err{0};
  auto sz{1};
  for (auto i{0}; i < 2; ++i)
    if (child[i][node] >= 0) {
      err += max_error[child[i][node]];
      sz += max_size[child[i][node]];
    } else
      ++sz;

  if (lt<E_t>(err, max_error[node]) or
      (equal<E_t>(err, max_error[node]) and sz < max_size[node])) {
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

template <template<typename> class ErrorPolicy, typename E_t>
bool BacktrackingAlgorithm<ErrorPolicy, E_t>::backtrack() {
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
    //
    // 		current_size -= max_size[backtrack_node];

    auto updt{update_upperbound(backtrack_node)};
    if (updt or lt<E_t>(max_error[backtrack_node],tree_error[backtrack_node]) or
        (equal<E_t>(tree_error[backtrack_node],max_error[backtrack_node]) and
         tree_size[backtrack_node] > max_size[backtrack_node])) {

#ifdef PRINTTRACE
      if (PRINTTRACE) // and not updt)
        cout << "new best for node " << backtrack_node
             << ": feat=" << *feature[backtrack_node]
             << ", error=" << max_error[backtrack_node]
             << ", size=" << max_size[backtrack_node] << endl;
#endif

      tree_error[backtrack_node] = max_error[backtrack_node];
      tree_size[backtrack_node] = max_size[backtrack_node];
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

    ++feature[backtrack_node];

    prune(child[0][backtrack_node]);
    prune(child[1][backtrack_node]);

    child[0][backtrack_node] = -1;
    child[1][backtrack_node] = -1;

    assert(no_feature(backtrack_node) or
           (not max_entropy(backtrack_node, *feature[backtrack_node])));

    dead_end = (
        // current_error >= ub_error or
        (lt<E_t>(0,ub_error) and equal<E_t>(max_error[backtrack_node], 0)) or
        no_feature(backtrack_node) or
        max_entropy(backtrack_node, *feature[backtrack_node]));

    // backtrack again
    if (dead_end) {

      optimal[backtrack_node] = true;

      // store best errors and sizes
      min_error[backtrack_node] = max_error[backtrack_node];
      min_size[backtrack_node] = max_size[backtrack_node];

#ifdef PRINTTRACE
      if (PRINTTRACE) {
        cout << search_size << " ";
        for (auto i{0}; i < decision.size(); ++i)
          cout << "   ";
        cout << "end domain for " << backtrack_node << "! ==> set optimal\n";
      }

      if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO) {
        cout << "~ERROR = " << current_error << " + "
             << max_error[backtrack_node] << endl;
        cout << "~SIZE = " << current_size << " + " << max_size[backtrack_node]
             << endl;
      }
#endif

      // this node will be pruned, so we need to add its current contribution to
      // "error" and "size" (to be removed again in "prune")
      current_size += (max_size[backtrack_node] - 1);
      current_error += max_error[backtrack_node];
    }

  } while (dead_end);

  // backtrack node is available for branching
  blossom.add(backtrack_node);

#ifdef PRINTTRACE
  if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO) {
    cout << "+ERROR = " << current_error << " + " << node_error(backtrack_node)
         << endl;
  }
#endif

  // add the error of the new blossom (the error of its descendant has been
  // subtracted already)
  current_error += node_error(backtrack_node);

  return true;
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::setChild(const int node,
                                                       const bool branch,
                                                       const int c) {

  parent[c] = node;
  if (P[0][c].count() == 0 or P[1][c].count() == 0) {
    child[branch][node] = -1 - (P[1][c].count() < P[0][c].count());
  } else {
    child[branch][node] = c;
    depth[c] = depth[node] + 1;
  }
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::branch(const int node, const int f) {

  // we assume that we branch only nodes
  assert(depth[node] < ub_depth - 1);

  // we assume that we branch only on tests with non-null error
  // assert(get_feature_error(node, f) >= -ErrorPolicy<E_t>::zero);

  if (equal<E_t>(get_feature_error(node, f), 0)) {
    auto fe{get_feature_error(node, f)};

    cout << fe << " " << (equal<E_t>(f, 0)) << endl;
  }

  assert(not equal<E_t>(get_feature_error(node, f), 0));

  decision.push_back(node);
  blossom.remove_front(node);

  // we create two nodes even if one branch is pure, but we'll free it
  if (blossom.capacity() < blossom.size() + 2)
    resize(blossom.size() + 2);

  int c[2] = {*blossom.bbegin(), *(blossom.bbegin() + 1)};

  // partition
  for (auto y{0}; y < 2; ++y) {
    P[y].branch(node, c[1], c[0],
                [&](const int x) { return reverse_dataset[y][f][x]; });
  }

#ifdef PRINTTRACE
  if (PRINTTRACE) {
    cout << setw(3) << decision.size();
    for (auto i{0}; i < decision.size(); ++i)
      cout << "   ";
    cout << "branch on " << node << " with " << f << " children: " << c[0]
         // << " (" << P[0][c[0]].count() << "/" << P[1][c[0]].count() << ") and
         // "
         // << c[1] << "(" << P[0][c[1]].count() << "/" << P[1][c[1]].count()
         // << ")"
         << endl;
  }
#endif

  for (auto i{0}; i < 2; ++i)
    setChild(node, i, c[i]);

  for (auto y{0}; y < 2; ++y) {
    auto lowest{P[y][c[1]].count() < P[y][c[0]].count()};

    count_by_example(c[lowest], y);

    deduce_from_sibling(node, c[1 - lowest], c[lowest], y);
  }

#ifdef PRINTTRACE
  if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO) {
    cout << "xERROR = " << current_error << " - " << node_error(node) << endl;
    // cout << "SIZE = " << current_size << " - " << 1 << endl;
  }
#endif

  // --current_size;
  current_error -= node_error(node);
  for (auto i{0}; i < 2; ++i) {
    auto s{child[i][node]};
    if (s >= 0) {
      grow(s);
// if(not grow(s))
// 	child[i][node] = -1 - (P[1][s].count() < P[0][s].count());

#ifdef PRINTTRACE
      if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO) {
        cout << "gERROR = " << current_error << " + " << max_error[s] << endl;
        // cout << "SIZE = " << current_size << " + " << max_size[s] << endl;
      }
#endif

      // current_size += max_size[s];
      current_error += max_error[s];
    }

    else {

#ifdef PRINTTRACE
      if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO) {
        cout << "gSIZE = " << current_size << " + " << 1 << endl;
    }
#endif
    ++current_size;
    }
  }

  update_upperbound(node);
}

template <template<typename> class ErrorPolicy, typename E_t>
bool BacktrackingAlgorithm<ErrorPolicy, E_t>::grow(const int node) {

#ifdef PRINTTRACE
  if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO) {
    cout << "gSIZE = " << current_size << " + " << 1 << endl;
  }
#endif

  ++current_size;

  feature[node] = ranked_feature[node].begin();
  end_feature[node] = ranked_feature[node].end();

  error_policy.update_node(node);

  filter_features(node, [&](const int f) { return max_entropy(node, f); });

  // cout << search_size;
  // for(auto f{feature[node]}; f != end_feature[node]; ++f)
  // 	cout << " " << *f;
  // cout << endl;

  blossom.add(node);

  best_tree[node] =
      (error_policy.get_total(0, node) < error_policy.get_total(1, node));

  if (feature[node] == end_feature[node]) {

    // assert(false);
    blossom.remove_front(node);
    tree_error[node] = max_error[node] = min_error[node] = node_error(node);
    child[0][node] = -1;
    child[1][node] = -1;
    tree_size[node] = max_size[node] = min_size[node] = 1;
    optimal[node] = true;

  } else {

    sort_features(node);
    optimal[node] = false;

    int f[2] = {*feature[node] + num_feature, *feature[node]};
    E_t err[2] = {min(get_feature_frequency(0, node, f[1]),
                      get_feature_frequency(1, node, f[1])),
                  min(get_feature_frequency(0, node, f[0]),
                      get_feature_frequency(1, node, f[0]))};

    if (depth[node] == ub_depth - 1 or equal<E_t>(err[0] + err[1],0)) {

      blossom.remove_front(node);

      for (auto branch{0}; branch < 2; ++branch) {
        child[branch][node] = -1 - (get_feature_frequency(1, node, f[branch]) <
                                    get_feature_frequency(0, node, f[branch]));
      }

      tree_error[node] = max_error[node] = min_error[node] = err[0] + err[1];
      tree_size[node] = max_size[node] = min_size[node] = 3;
      store_best_tree(node, true);

      optimal[node] = true;

#ifdef PRINTTRACE
      if (PRINTTRACE and options.verbosity >= DTOptions::SOLVERINFO) {
        cout << "gSIZE = " << current_size << " + " << 2 << endl;
      }
#endif

      current_size += 2;

    } else {

      // best_tree[node] = -1;
      min_error[node] = 0;
      min_size[node] = 5;
      tree_size[node] = max_size[node] = maxSize(ub_depth - depth[node]);
      tree_error[node] = max_error[node] = node_error(node);
      // max_size[node] =
  }
  }
  return true;
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::expend() {

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
    case DTOptions::ANTIERROR:
      selected_node = lowest_error();
      break;
    }
    // selected_node = choose();

    if (max_entropy(selected_node, *feature[selected_node])) {

      cout << selected_node << " " << *feature[selected_node] << " "
           << get_feature_frequency(0, selected_node, *feature[selected_node])
           << " / " << error_policy.get_total(0, selected_node) << " || "
           << get_feature_frequency(1, selected_node, *feature[selected_node])
           << " / " << error_policy.get_total(1, selected_node) << endl;
    }

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

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::initialise_search() {
	
	
	if(options.output != "") {
		printDatasetToFile(options.output);
	}
	
  num_level_zero_feature = num_feature;

  setReverse();

  start_time = cpu_time();

  for (int y{0}; y < 2; ++y)
    P[y].init(example[y].size());

  for (auto f{0}; f < num_feature; ++f)
    relevant_features.push_back(f);

  // tree must have at least one node (0)
  resize(1);

  // compute error and sort features for the root node
  for (auto y{0}; y < 2; ++y)
    count_by_example(0, y);

  error_policy.update_node(0);

  if (ub_depth == 0)
    noDecision();
  else {

    current_size = 0;

    grow(0);

    current_error = max_error[0];
    // current_size = 1;

    backtrack_node = -1;

    relevant_features.clear();
    feature_set.resize(num_feature, true);
    for (int fi{0}; fi < num_feature; ++fi) {
      if (feature_set[fi] and not max_entropy(0, fi)) {
        relevant_features.push_back(fi);

        if (options.preprocessing)
          for (int fj{fi + 1}; fj < num_feature; ++fj) {
            if (equal_feature(fi, fj)) {
              feature_set.reset(fj);
            }
          }
      }
    }

    // if (options.preprocessing) {
    //   sort(relevant_features.begin(), relevant_features.end(),
    //        [&](const int a, const int b) {
    //          auto pa{get_feature_frequency(1, 0, a)};
    //          auto pb{get_feature_frequency(1, 0, b)};
    //          auto na{get_feature_frequency(0, 0, a)};
    //          auto nb{get_feature_frequency(0, 0, b)};
    //          if (pa < pb)
    //            return true;
    //          if (pa == pb) {
    //            if (na < nb)
    //              return true;
    //            if (na == nb) {
    //              if (reverse_dataset[0][a] < reverse_dataset[0][b])
    //                return true;
    //              if (reverse_dataset[0][a] > reverse_dataset[0][b])
    //                return false;
    //              return (reverse_dataset[1][a] <= reverse_dataset[0][b]);
    //            }
    //          }
    //          return false;
    //        });
    //
    //   for (int fi{1}; fi < num_feature; ++fi) {
    //     if (equal_feature(relevant_features[fi - 1], relevant_features[fi]))
    //       feature_set.reset(relevant_features[fi]);
    //   }
    // }

    if (options.verbosity >= DTOptions::NORMAL)
      cout << "d feature=" << num_feature << " feature_reduction="
           << (num_feature - relevant_features.size()) << endl;

    filter_features(0, [&](const int f) { return not feature_set[f]; });
    sort_features(0);

    // assert(store_new_best());

    // for(auto f : relevant_features)
    // 	cout << " " << f ;
    // cout << endl;

    // assert(feature[0] == ranked_feature[0].begin());
    num_level_zero_feature = (end_feature[0] - feature[0]);
  }
}

template <template<typename> class ErrorPolicy, typename E_t>
bool BacktrackingAlgorithm<ErrorPolicy, E_t>::search() {
  auto sat = false;

  while ((not limit_out()) and (ub_error > 0 or size_matters)) {

    if (num_backtracks > restart_limit)
      restart(false);

    if (decision.empty())
      print_progress();

    PRINT_TRACE;

    DO_ASSERTS;

    if (blossom.empty()) {
      if (not notify_solution(sat))
        break;
    } else if (options.bounding and sat and fail()) {
      if (not backtrack())
        break;
    } else {
      expend();
    }
  }

  return sat;
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::singleDecision() {
  // if(ub_depth <= 1 or current_error == 0) {
  blossom.remove_front(0);
  for (auto branch{0}; branch < 2; ++branch) {
    child[branch][0] =
        -1 - (get_feature_frequency(1, 0, *feature[0] + num_feature) <
              get_feature_frequency(0, 0, *feature[0]));
  }

  tree_error[0] = max_error[0] = min_error[0] =
      get_feature_error(0, *feature[0]);
  current_size = tree_size[0] = max_size[0] = min_size[0] = 3;
  store_best_tree(0, true);
  optimal[0] = true;
  // } else {
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::noDecision() {
  blossom.remove_front(0);
  current_error = tree_error[0] = max_error[0] = min_error[0] = node_error(0);
  child[0][0] = -1;
  child[1][0] = -1;
  current_size = tree_size[0] = max_size[0] = min_size[0] = 1;
  optimal[0] = true;
  best_tree[0] = (P[0][0].count() < P[1][0].count());
  store_new_best();
}


template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::minimize_error() {

  initialise_search();

  if (options.verbosity > DTOptions::QUIET)
    separator("search");

  search();

  // cleaning();

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

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::minimize_error_depth() {

  initialise_search();

  if (options.verbosity > DTOptions::QUIET)
    separator("search");

  auto perfect{false};
  // auto saved_error{ub_error};
  while (ub_depth > 0 and search() and is_null<E_t>(ub_error)) {
    perfect = true;
    // saved_error = ub_error;
    ub_error = min_positive<E_t>();
    ub_depth = actual_depth - 1;

    restart(true);

    if (ub_depth == 1)
      singleDecision();
  }

  if (perfect) {
    ++ub_depth;
    ub_error = 0;
  }
  // else
  //     cleaning();

  if (options.verbosity > DTOptions::QUIET) {
    if (interrupted)
      separator("interrupted");
    else
      separator("optimal");
  }

  if (options.verbosity > DTOptions::SILENT)
    print_new_best();
}


template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::minimize_error_depth_size() {

  initialise_search();

  if (options.verbosity > DTOptions::QUIET)
    separator("search");

  auto perfect{equal<E_t>(current_error,0)};
  // auto saved_error{ub_error};
  while (ub_depth > 0 and search() and is_null<E_t>(ub_error)) {
    perfect = true;
    // saved_error = ub_error;
    ub_error = min_positive<E_t>();
    ub_depth = actual_depth - 1;

    restart(true);

    if (ub_depth == 1)
      singleDecision();
  }

  if (perfect) {

    ub_error = 0; // saved_error;
    ++ub_depth;

    if (ub_depth > 1) {

      size_matters = true;
      optimal[0] = false;

      restart(true);

      feature[0] = ranked_feature[0].begin();

      search();
    }
  }

  if (options.verbosity > DTOptions::QUIET) {
    if (interrupted)
      separator("interrupted");
    else
      separator("optimal");
  }

  if (options.verbosity > DTOptions::SILENT)
    print_new_best();
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::addExample(const std::vector<int> &example, const E_t weight) {
  addExample(example.begin(), example.end() - 1, example.back(), weight);
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::clearExamples() {
	for (int i{0}; i < 2; ++i) {
		dataset[i].clear();
		example[i].clear();
		reverse_dataset[i].clear();
	}

        error_policy.clear_examples();
}

template <template<typename> class ErrorPolicy, typename E_t>
bool BacktrackingAlgorithm<ErrorPolicy, E_t>::fail() {
  for (auto b : blossom) {

#ifdef PRINTTRACE
    if (PRINTTRACE)
      cout << "bound from " << b << endl;
#endif

    E_t lbe{0};
    auto lbs{0};

    auto p{b};
    auto c{b};
    while (p > 0) {
      c = p;
      p = parent[c];

      E_t ube{max_error[p]};
      auto ubs{max_size[p]};

      for (auto i{0}; i < 2; ++i)
        if (child[i][p] >= 0) {
          lbe += min_error[child[i][p]];
          if (child[i][p] != c)
            lbs += min_size[child[i][p]];
          ++lbs;
        }
#ifdef PRINTTRACE
      if (PRINTTRACE)
        cout << "parent " << p << " (ub=" << ube << "/" << ubs << ", lb=" << lbe
             << "/" << lbs << ") ["
             << (child[0][p] >= 0 ? min_error[child[0][p]] : 0) << "/"
             << (child[1][p] >= 0 ? min_error[child[1][p]] : 0) << " | "
             << (child[0][p] >= 0 ? min_size[child[0][p]] : 1) << "/"
             << (child[1][p] >= 0 ? min_size[child[1][p]] : 1) << "]\n";
#endif

      if (lt<E_t>(ube, lbe) or (equal<E_t>(lbe, ube) and (lbs >= ubs or not size_matters))) {
#ifdef PRINTTRACE
        if (PRINTTRACE)
          cout << "fail!! (" << b << " " << p << " max size = " << max_size[b]
               << ") b = " << backtrack_node << "\n";
#endif

        return true;
    }
  }
  }

  return false;
}

template <template<typename> class ErrorPolicy, typename E_t>
int BacktrackingAlgorithm<ErrorPolicy, E_t>::copy_solution(const int node) {

  if (node >= 0) {
    if (optimal[node]) {
      auto cn{wood.copyNode(best_tree[node])};
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
template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::store_best_tree(const int node, const bool global) {

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

template <template<typename> class ErrorPolicy, typename E_t>
double BacktrackingAlgorithm<ErrorPolicy, E_t>::entropy(const int node, const int feature) {
  double feature_entropy{0};

  int not_feature = (feature + num_feature);
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

template <template<typename> class ErrorPolicy, typename E_t>
double BacktrackingAlgorithm<ErrorPolicy, E_t>::gini(const int node, const int feature) {
  int not_feature = (feature + num_feature);
  int truef[2] = {not_feature, feature};

  double branch_size[2]; // = {0,0};
  double p[2];           // = {0,0};
  double gini[2];        // = {0,0};

  // for (auto x{0}; x < 2; ++x) {
  //
  //   for (auto y{0}; y < 2; ++y)
  //     cout << static_cast<double>(get_feature_frequency(y, node, truef[x]))
  //          << " ";
  // }
  // cout << endl;

  // conditional to value x for feature
  for (auto x{0}; x < 2; ++x) {

    for (auto y{0}; y < 2; ++y)
      p[y] = static_cast<double>(
          get_feature_frequency(y, node,
                                truef[x])); // how many class-i samples if f=x

    // number of samples falling on this side
    branch_size[x] = p[0] + p[1];

    // we compute on integers, normalized by n^2
    gini[x] = (branch_size[x] * branch_size[x]);

    for (auto y{0}; y < 2; ++y)
      gini[x] -= p[y] * p[y];
  }

  // cout << feature << " " << ((gini[0] / (branch_size[0] * branch_size[0])) - 1)
  //      << endl
  //      << ((gini[1] / (branch_size[1] * branch_size[1])) - 1) << endl;

  return ((gini[1] / (branch_size[1])) + (gini[0] / (branch_size[0])));
	// return ((gini[1] / (branch_size[1] * branch_size[1])) + (gini[0] / (branch_size[0] * branch_size[0])));
}

// template <template<typename> class ErrorPolicy, typename E_t>
// double BacktrackingAlgorithm<ErrorPolicy, E_t>::gini(const int node, const
// int feature) {
//   int not_feature = (feature + num_feature);
//   int truef[2] = {not_feature, feature};
//
//   double branch_size[2]; // = {0,0};
//   double p[2];           // = {0,0};
//   double gini[2];        // = {0,0};
//
// 	//   for (auto x{0}; x < 2; ++x) {
// 	//
// 	//     for (auto y{0}; y < 2; ++y)
// 	//       cout << static_cast<double>(get_feature_frequency(y, node, truef[x]))
// 	//            << " ";
// 	//   }
// 	//   cout << endl;
// 	//
// 	// cout << feature << " " ;
//
//   // conditional to value x for feature
//   for (auto x{0}; x < 2; ++x) {
//
//     for (auto y{0}; y < 2; ++y)
//       p[y] = static_cast<double>(
//           get_feature_frequency(y, node,
//                                 truef[x])); // how many class-i samples if f=x
//
//     // number of samples falling on this side
//     branch_size[x] = p[0] + p[1];
//
//     // we compute on integers, normalized by n^2
//     gini[x] = 0;
// 		// auto tsize{(branch_size[x] * branch_size[x])};
//
//     for (auto y{0}; y < 2; ++y)
//       gini[x] -= (p[y] * p[y]);
// 		gini[x] /= (branch_size[x] * branch_size[x]);
//
// 		// cout << gini[x] << endl;
//    }
//
// 	return 2 - gini[0] - gini[1];
//   // return ((gini[1] / branch_size[1]) + (gini[0] / branch_size[0]));
// }

template <template<typename> class ErrorPolicy, typename E_t>
bool BacktrackingAlgorithm<ErrorPolicy, E_t>::equal_feature(const int f_a, const int f_b) {
  int lit_a[2] = {f_a + num_feature, f_a};
  int lit_b[2] = {f_b + num_feature, f_b};

  if ((equal<E_t>(get_feature_frequency(1, 0, lit_a[0]),
           get_feature_frequency(1, 0, lit_b[0])) and
       equal<E_t>(get_feature_frequency(0, 0, lit_a[0]),
           get_feature_frequency(0, 0, lit_b[0])))) {
    if (reverse_dataset[0][f_a] == reverse_dataset[0][f_b] and
        reverse_dataset[1][f_a] == reverse_dataset[1][f_b])
      return true;
  } else if (equal<E_t>(get_feature_frequency(1, 0, lit_a[0]),
                 get_feature_frequency(1, 0, lit_b[1])) and
             equal<E_t>(get_feature_frequency(0, 0, lit_a[0]),
                 get_feature_frequency(0, 0, lit_b[1]))) {
    auto eq{reverse_dataset[0][f_a].flip() == reverse_dataset[0][f_b]};
    reverse_dataset[0][f_a].flip();
    if (eq) {
      eq = (reverse_dataset[1][f_a].flip() == reverse_dataset[1][f_b]);
      reverse_dataset[1][f_a].flip();
    }
    return eq;
  }

  return false;
}

//// GARBAGE /////

template <template<typename> class ErrorPolicy, typename E_t>
size_t BacktrackingAlgorithm<ErrorPolicy, E_t>::maxSize(const int depth) const {
  return (1 << (depth + 1)) - 1;
}

template <template<typename> class ErrorPolicy, typename E_t>
size_t BacktrackingAlgorithm<ErrorPolicy, E_t>::computeSize(const int node) const {

  if (node < 0 or blossom.contain(node)) {
    return 1;
  }

  if (optimal[node]) {
    return max_size[node];
  }

  auto l{computeSize(child[0][node])};
  auto r{computeSize(child[1][node])};

  return 1 + l + r;
}

template <template<typename> class ErrorPolicy, typename E_t>
std::ostream &BacktrackingAlgorithm<ErrorPolicy, E_t>::display(std::ostream &os) const {

  for (auto i : blossom) {
    cout << i << ": " << P[0][i].count() << "/" << P[1][i].count() << endl;
  }

  return os;
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::printDatasetToFile(std::string &filename) const {
	
	ofstream outfile(filename.c_str(), ofstream::out);
	
	for(auto y{0}; y<2; ++y) {
		for(auto x{0}; x<example[y].size(); ++x) {
			outfile << y;
			for(auto f{0}; f<numFeature(); ++f) {
				outfile << " " << dataset[y][x][f];
			}
			outfile << endl;
		}
	}
	outfile.close();
}

template <template<typename> class ErrorPolicy, typename E_t>
std::ostream &operator<<(std::ostream &os, const BacktrackingAlgorithm<ErrorPolicy, E_t> &x) {
  return x.display(os);
}


template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::setErrorOffset(const E_t e) {
	error_offset = e;
}


#ifdef PRINTTRACE

template <template<typename> class ErrorPolicy, typename E_t>
bool BacktrackingAlgorithm<ErrorPolicy, E_t>::isLeaf(const int node) const {
  return not blossom.contain(node) and child[0][node] < 0 and
         child[1][node] < 0;
}

template <template<typename> class ErrorPolicy, typename E_t>
E_t BacktrackingAlgorithm<ErrorPolicy, E_t>::leaf_error(const int node) const {
  if (isLeaf(node) or optimal[node]) {
    return max_error[node];
  } else {
    return node_error(node);
  }
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::print_trace() {
  if (PRINTTRACE) {

    cout << setw(3) << decision.size();
    for (auto i{0}; i < decision.size(); ++i)
      cout << "   ";
    cout << "error = " << current_error << "/" << ub_error
         << "; depth = " << ub_depth << "; size = " << current_size << "/"
         << ub_size << "; search=" << search_size << endl;

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
        cout << setw(3)
             << (optimal[*d] ? wood.getFeature(best_tree[*d]) : *feature[*d])
             << " ";
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
      cout << endl << "max_s: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << max_size[*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3) << max_size[b] << " ";
      }
      cout << endl << "min_s: ";
      for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {
        cout << setw(3) << min_size[*d] << " ";
      }
      cout << "  ";
      for (auto b : blossom) {
        cout << setw(3) << min_size[b] << " ";
      }
      cout << endl;
    }
}
}

template <template<typename> class ErrorPolicy, typename E_t>
void BacktrackingAlgorithm<ErrorPolicy, E_t>::do_asserts() {
  // const double eps = 0.001; // std::numeric_limits<double>::epsilon();

  for (auto b : blossom) {
    assert(not optimal[b]);
  }

  E_t total_error{0};
  for (auto d{blossom.fbegin()}; d != blossom.fend(); ++d) {

    assert(isLeaf(*d) or ((child[0][*d] < 0 or parent[child[0][*d]] == *d) and
                          (child[1][*d] < 0 or parent[child[1][*d]] == *d)));

    if (isLeaf(*d)) {
      total_error += leaf_error(*d);
    }
  }
  for (auto b : blossom) {
    E_t le = leaf_error(b);
    E_t ne = node_error(b);

    if (not equal<E_t>(le, ne)) {
      cout << le << " / " << ne << endl;
    }
    assert(equal<E_t>(le, ne));
    total_error += node_error(b);
  }

  for (auto d : decision) {
    assert(blossom.index(d) < blossom.size());
  }

  if (not equal<E_t>(total_error, current_error))
    cout << current_error << " / " << total_error << " @" << search_size
         << endl;

  assert(equal<E_t>(total_error, current_error));

  auto total_size{computeSize(0)};

  if (total_size != current_size)
    cout << current_size << " / " << total_size << " @" << search_size << endl;

  assert(current_size == total_size);
}
#endif

// // template class CardinalityError<unsigned long>;
// template class CardinalityError<int>;
// template class WeightedError<int>;
// template class WeightedError<double>;

// template class BacktrackingAlgorithm<CardinalityError, unsigned long>;
template class BacktrackingAlgorithm<CardinalityError, int>;
template class BacktrackingAlgorithm<CardinalityError, unsigned long>;
template class BacktrackingAlgorithm<WeightedError, int>;
template class BacktrackingAlgorithm<WeightedError, float>;
template class BacktrackingAlgorithm<WeightedError, double>;
template class BacktrackingAlgorithm<WeightedError, unsigned long>;
}
