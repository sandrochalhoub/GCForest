
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include "Backtrack.hpp"

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
  best_error.resize(k, data.count());
  best_size.resize(k, numeric_limits<int>::max());

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

size_t BacktrackingAlgorithm::error() {
  size_t error{0};
  for (auto i : blossom) {

    // cout << " min(" << P[0][i].count() << ", " << P[1][i].count() << ")";

    auto e{std::min(P[0][i].count(), P[1][i].count())};
    error += e;
  }

  return error;
}

double BacktrackingAlgorithm::accuracy() {
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
    // each blossom need to be expended, and depending on the feature count, we can
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
}

void BacktrackingAlgorithm::unset_optimal(const int node) {
  optimal[node] = -2;
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

void BacktrackingAlgorithm::print_new_best() const {
  cout << " size=" << left << setw(10) << ub_node << "  accuracy=" << setw(10)
       << (1.0 -
           static_cast<double>(ub_error) / static_cast<double>(data.count()))
       // << " backtracks=" << setw(10) << num_backtracks
       << " choices=" << setw(10) << search_size << " restarts=" << setw(10)
       << num_restarts << " time=" << setw(10) << cpu_time()

       << endl;
}

bool BacktrackingAlgorithm::dead_end(const int branching_node) {

  if (not blossom.count()) {

    assert(branching_node < 0);

    // this is a solution, and the tree is correct!!!
    ub_error = 0;
    ub_node = num_node;
    ub_depth = max_depth.back();

    if (options.verbosity >= Options::NORMAL) {
      print_new_best();
    }

    return true;
  }

  if (num_node >= ub_node) {

    assert(branching_node < 0);

    // new tree, check if the accuracy is better
    auto err{error()};
    if (err < ub_error or (err == ub_error and max_depth.back() < ub_depth)) {
      ub_error = err;
      ub_depth = max_depth.back();

      if (options.verbosity >= Options::NORMAL) {
        print_new_best();
      }
    }

    return true;
  }

  if (branching_node >= 0 and
      (is_optimal(branching_node) or no_feature(branching_node) or
       max_entropy(branching_node, *(feature[branching_node])))) {

    if (options.verbosity >= Options::YACKING) {
      for (auto i{0}; i < num_node; ++i)
        cout << " ";
      if (is_optimal(branching_node))
        cout << "subtree is optimal!\n";
      else
        cout << "domain end!\n";
    }

    return true;
  }

  auto depth_limit{true};
  auto i{blossom.begin()};
  while (depth_limit and i++ != blossom.end())
    depth_limit = (depth[*i] == ub_depth);

  if (depth_limit) {
    assert(false);

    // new tree, check if the accuracy is better
    auto err{error()};
    if (err < ub_error) {
      ub_error = err;
      ub_node = num_node;

      if (options.verbosity >= Options::NORMAL) {
        print_new_best();
      }
    }

    return true;
  }

  if (ub_error > 0) {

		auto lb{error_lower_bound()};
    if (lb >= ub_error) {
      if (options.verbosity >= Options::YACKING) {
        for (auto i{0}; i < num_node; ++i)
          cout << " ";
        cout << "OPTIMISTIC LOWER BOUND (ERROR) = " << lb << "/" << ub_error
             << endl;
      }
      return true;
    }

  } else {

		auto lb{node_lower_bound()};
    if (max_depth.back() >= ub_depth or lb > ub_node) {

      if (options.verbosity >= Options::YACKING) {
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

  if (options.verbosity >= Options::YACKING)
    cout << "start search\n";

  sort_features(0);

  auto branching_node{-1};

  double restart_base{static_cast<double>(options.restart_base)};
  size_t restart_limit{static_cast<size_t>(restart_base)};
  double restart_factor{options.restart_factor};
  // size_t restart_divisor{1024};

  while (true) {

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
			
			// cout << "restart (limit=" << static_cast<size_t>(restart_base) << ")\n";
			

      ++num_restarts;
      clear(branching_node);
    }

    if (options.verbosity >= Options::YACKING) {
      for (auto i{0}; i < num_node; ++i) {
        if (is_optimal(i))
          cout << " " << best_error[i];
        else
          cout << " .";
      }
      cout << endl;
    }

    if (options.verbosity >= Options::YACKING) {
      for (auto i{0}; i < num_node; ++i)
        cout << " ";
      cout << "node: " << branching_node << "; ub = " << ub_error << "; "
           << ub_node << (branching_node >= 0 and is_optimal(branching_node)
                              ? " -- optimal"
                              : "")
           << "\n";
    }

    auto end_branch{not blossom.count()};

    if (end_branch) {
      if (options.verbosity >= Options::YACKING) {
        cout << "leaf because correct!\n";
      }
      ub_error = 0;
      ub_node = num_node;

      if (options.verbosity >= Options::NORMAL) {
        print_new_best();
      }

    } else {
      auto node_limit{num_node >= ub_node};
			// auto depth_limit{}
			end_branch = node_limit;
      if (end_branch) {

        auto err{error()};

        if (options.verbosity >= Options::YACKING) {
          cout << "leaf because the node limit (" << ub_node
               << ") is reached, error=" << err << "\n";
        }

        // assert(err < ub_error);

        if (err < ub_error) {//} or (err == ub_error and max_depth < ub_depth)) {
          ub_error = err;
					// max_depth;

          if (options.verbosity >= Options::NORMAL) {
            print_new_best();
          }
        }
      }

      //
      //           num_node + 2 * blossom.count() >= ub_node or ;
      //
      //       if (end_branch) {
      //
      // auto err{error()};
      //
      // assert(err < ub_error);
      //
      //
      //
      //
      //         if (num_node + 2 * blossom.count() >= ub_node)
      //           cout << "blossom because there is no correct subtree of size
      //           less than "
      //                << ub_node << endl;
      //         else {
      //           assert(num_node >= max_num_node);
      //           cout << "blossom because the node limit (" << max_num_node
      //                << ") is reached\n";
      //         }
      //       }
    }

    if (not end_branch and branching_node >= 0) {
      end_branch = is_optimal(branching_node) or no_feature(branching_node) or
                   max_entropy(branching_node, *(feature[branching_node]));

      if (options.verbosity >= Options::YACKING) {
        if (end_branch) {
          for (auto i{0}; i < num_node; ++i)
            cout << " ";
          if (is_optimal(branching_node))
            cout << "subtree is optimal!\n";
          else
            cout << "domain end!\n";
        }
      }
    }

    if (not end_branch) {

      if (ub_error > 0) {
        auto lb{error_lower_bound()};

        if (lb >= ub_error) {
          end_branch = true;
          if (options.verbosity >= Options::YACKING) {
            for (auto i{0}; i < num_node; ++i)
              cout << " ";
            cout << "OPTIMISTIC LOWER BOUND (ERROR) = " << lb << "/" << ub_error
                 << endl;
          }
        }
      } else {

        auto lb{node_lower_bound()};

        if (lb >= ub_node) {
          end_branch = true;
          if (options.verbosity >= Options::YACKING) {
            for (auto i{0}; i < num_node; ++i)
              cout << " ";
            cout << "OPTIMISTIC LOWER BOUND (NODE) = " << lb << "/" << ub_node
                 << endl;
          }
        }
      }
    }

    if (end_branch) {

      // for (auto i{0}; i < num_node; ++i)
      //   cout << " ";
      // cout << "backtrack over " << branching_node << endl;

      if (num_node == 1)
        break;
      else {
        ++num_backtracks;
        backtrack(branching_node);
      }

    } else {

      if (branching_node < 0) {

        branching_node = select();

        if (ub_error < data.count())
          random_perturbation(branching_node, kbest, p);

        // sort_features(branching_node);

        if (options.verbosity >= Options::YACKING) {
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

  blossom.remove_back(--num_node);
  blossom.remove_back(--num_node);

  unset_optimal(num_node);
  unset_optimal(num_node + 1);

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

  if (options.verbosity >= Options::YACKING) {
    for (auto i{0}; i < num_node; ++i)
      cout << " ";
    cout << "backtrack from " << child << " to " << branching_node << endl;
  }

  if (child >= 0) {
    auto sibling{child % 2 ? child + 1 : child - 1};
    auto father{parent[child]};
    assert(parent[sibling] == father);

    auto new_best_error{best_error[child] + best_error[sibling]};
    auto new_best_size{1 + best_size[child] + best_size[sibling]};

    if (best_error[father] > new_best_error or
        best_size[father] > new_best_size) {
      best_feature[father] = *feature[father];
      best_error[father] = new_best_error;
      best_size[father] = new_best_size;
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
  for (auto i : blossom) {
    auto error{std::min(P[0][i].count(), P[1][i].count())};

    auto f{*(feature[i])};
    auto not_f{f + data.numFeature()};

    auto err_f{min(get_feature_count(0, i, f), get_feature_count(1, i, f))};
    auto err_not_f{
        min(get_feature_count(0, i, not_f), get_feature_count(1, i, not_f))};

    auto reduction{error - err_f - err_not_f};

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

  // cout << "--> " << selected_node << endl;

  return selected_node;
}

void BacktrackingAlgorithm::random_perturbation(const int selected_node,
                                                const int kbest, const int p) {
  if (f_entropy[*(ranked_feature[selected_node].begin())] != 0 and
      random_generator() % 1000 < p)
    swap(*(ranked_feature[selected_node].begin()),
         *(ranked_feature[selected_node].begin() +
           (random_generator() % kbest)));
}

void BacktrackingAlgorithm::sort_features(const int selected_node) {
  if (feature[selected_node] != ranked_feature[selected_node].begin()) {
    cout << selected_node << endl;
    exit(1);
  }

  // cout << "new node: " << selected_node
  //      << ", compute entropy and rank feature\n";

  for (auto f{0}; f < data.numFeature(); ++f)
    f_entropy[f] = entropy(selected_node, f);

  // cout << "sort\n";

  sort(ranked_feature[selected_node].begin(),
       ranked_feature[selected_node].end(),
       [&](const int a, const int b) { return f_entropy[a] < f_entropy[b]; });

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
  if (num_node == ub_node - 2 or null_entropy(selected_node, bf)) {

    // cout << "node " << selected_node << " has a null entropy feature:\n" ;
    //
    // cout << " ==> " << *(ranked_feature[selected_node].begin()) << endl;
    //
    // cout << optimal.size() << endl;

    optimal[selected_node] = parent[selected_node];
    best_feature[selected_node] = bf;
    best_error[selected_node] =
        min(P[0][selected_node].count(), P[1][selected_node].count());
    best_size[selected_node] = 1;
  }

  split(selected_node, *(feature[selected_node]));

  // }

  if (options.verbosity >= Options::YACKING) {
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
  blossom.remove_front(node);
  // if (num_node < parent.size()) {
  for (auto i{0}; i < 2; ++i) {
    child = num_node - 2 + i;

    // cout << "child " << child << endl;

    parent[child] = node;
    depth[child] = depth[node] + 1;
    // blossom.add(child);
    if (P[0][child].count() > 0 and P[1][child].count() > 0) {
      blossom.add(child);
    }
  }
	if(blossom.count() >= nb) {
		
		// cout << blossom.count() << " >= " << nb << endl;
		
		max_depth.push_back(max(max_depth.back(), depth[node] + 1));
	} else {
		max_depth.push_back(max_depth.back());
	}
  // }
  // blossom.remove_front(node);

  for (auto y{0}; y < 2; ++y) {
    auto smallest_child{
        (P[y][child].count() > P[y][child - 1].count() ? child - 1 : child)};
    auto largest_child{(child == smallest_child ? child - 1 : child)};

    // cout << "-> " << smallest_child << " and " << largest_child << endl;

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