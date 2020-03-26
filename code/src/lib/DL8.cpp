


#include "DL8.hpp"
#include "Tree.hpp"

namespace primer {

DL8::DL8(DataSet &d, Options &opt) : data(d), options(opt) {

  seed(options.seed);

  auto m{data.numFeature()};

  branch.reserve(m);

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

  // resize(1);

  for (auto y{0}; y < 2; ++y) {
    P[y].init(data.example[y].count());
    // P[y].addNode();
    // count_by_example(0, y);
  }

  ub_depth = options.max_depth;

  // search_size = 0;
}

void DL8::resize(const int k) {

  assert(k % 2);

  node_feature.resize(k, -1);
  node_error.resize(k, data.count());
  parent.resize(k, -1);

  for (auto y{0}; y < 2; ++y)
    while (P[y].size() < k)
      P[y].addNode();

  sorted_feature.resize(k);
}


void DL8::seed(const int s) { random_generator.seed(s); }

size_t DL8::size() const { return parent.size(); }

size_t DL8::error() const { return node_error[0]; }

size_t DL8::error(const int i) const {
  return std::min(P[0][i].count(), P[1][i].count());
}

double DL8::accuracy() const {
  return 1.0 - static_cast<double>(error()) / static_cast<double>(data.count());
}

void DL8::print_new_best() const {

  cout //<< " size=" << left << setw(10) << ub_node
      << " depth=" << left << setw(10) << ub_depth << "  accuracy=" << setw(10)
      << accuracy() << " choices=" << setw(10) << search_size
      // << " restarts=" << setw(10) << num_restarts
      << " time=" << setw(10) << cpu_time() << right << endl;
}


size_t DL8::optimize() {

  resize(1);
  search_size = 0;

  int n{1};

  branch.fill();

  node_error[0] = error(0);
  node_feature[0] = -1;
  parent[0] = 0;

  recurse(branch, n, 0, 0);

  print_new_best();
	
	cout << "total error: " << node_error[0] << endl;
	

  return error();
}


/// branch contain the positive tests (front), the negative tests (back) and the
/// available tests (in)
/// partition s is the one pointed to by the branch
/// n is the number of nodes with feature tests
void DL8::recurse(SparseSet &branch, int &n, const int d, const int s) {

  ++search_size;

#ifdef PRINTTRACE
  if (PRINTTRACE) {
    for (auto i{0}; i < d; ++i)
      cout << "   ";
    cout << "recurse({";
    if (branch.fbegin() != branch.fend()) {
      cout << *branch.fbegin();
      for (auto i{branch.fbegin() + 1}; i != branch.fend(); ++i)
        cout << "," << *i;
    }
    cout << "}," << n << "," << d << "," << s << ") error=" << node_error[s]
         << " [" << error() << "]\n";
  }
#endif

  // is it a leaf?
  if (d == ub_depth or node_error[s] == 0) {
    //
    // cout << "PRUNE " << s << " (parent=" << parent[s] << ", depth=" << d
    //      << ", error=" << node_error[s] << ")\n";

    return;
  }

  // not a leaf, the children of node s
  int c[2] = {n, n + 1};

  // two mode nodes
  n += 2;

  // make sure that the structures can hold the data
  if (size() < n)
    resize(n);

  // not safe to iterate on branch because the order changes, and anyway we'll
  // need to sort 'em
  sorted_feature.clear();
  for (auto f : branch)
    sorted_feature[s].push_back(f);

  int width{static_cast<int>(sorted_feature[s].size())};

  // to store the number of used nodes, couting previous nodes, plus the size of
  // the optimal subtree
  int node_size{n};
  for (auto f : sorted_feature[s]) {

    // if ((s == 0 and f != 1) or (s == 1 and f != 56) or (s == 2 and f != 0) or
    //     (s == 3 and f != 27) or (s == 4 and f != 37) or (s == 9 and f != 73) or
    //     (s == 10 and f != 53))
    //   continue;

    assert(--width >= 0);

    // restart from the same count for every feature
    auto sz{n};

#ifdef PRINTTRACE
    if (PRINTTRACE) {
      for (auto i{0}; i < d; ++i)
        cout << "   ";
      cout << "branch with " << s << "=" << f << ":";
    }
#endif

    // branch with respect to f
    for (auto y{0}; y < 2; ++y)
      P[y].branch(s, c[0], c[1],
                  [&](const int x) { return data.ithHasFeature(y, x, f); });

    // initialise stats for the two subtrees
    for (auto i{0}; i < 2; ++i) {

#ifdef PRINTTRACE
      if (PRINTTRACE)
        cout << " c" << c[i] << " (" << P[0][c[i]].count() << "/"
             << P[1][c[i]].count() << ")";
#endif

      node_error[c[i]] = error(c[i]);
      node_feature[c[i]] = -1;
      parent[c[i]] = s;
    }

#ifdef PRINTTRACE
    if (PRINTTRACE)
      cout << endl;
#endif

    auto largest{node_error[c[1]] > node_error[c[0]]};

    assert(node_error[c[largest]] >= node_error[c[1 - largest]]);

    branch.remove_front(f);

    // optimize left subtree
    recurse(branch, sz, d + 1, c[largest]);

    // optimize right subtree
    if (node_error[c[largest]] < node_error[s])
      recurse(branch, sz, d + 1, c[1 - largest]);

    branch.add(f);

    auto f_error{node_error[c[0]] + node_error[c[1]]};
    // store these subtrees if they are optimal
    if (f_error < node_error[s]) {
      node_error[s] = f_error;
      node_feature[s] = f;
      node_size = sz;

      if (node_error[s] == 0) {
        // cout << "stop?\n";
        break;
      }

#ifdef PRINTTRACE
      if (PRINTTRACE) {
        for (auto i{0}; i < d; ++i)
          cout << "   ";
        cout << "new best feature for node " << s << ": " << f << " ("
             << f_error << ") size=" << sz << "\n";
      }
#endif

      if (s == 0) {

				//         cout << 0 << ": " << node_feature[0] << endl;
				// for(auto i{1}; i<size(); ++i)
				// {
				// 	        cout << i << " (" << parent[i] << ")"
				// 	             << ": " << node_feature[i] << endl;
				// }
				
        // cout << 1 << " (" << parent[1] << ")"
        //      << ": " << node_feature[1] << endl;
        // cout << 2 << " (" << parent[2] << ")"
        //      << ": " << node_feature[2] << endl;
        // cout << 3 << " (" << parent[3] << ")"
        //      << ": " << node_feature[3] << endl;
        // cout << 4 << " (" << parent[4] << ")"
        //      << ": " << node_feature[4] << endl;
        // cout << 9 << " (" << parent[9] << ")"
        //      << ": " << node_feature[9] << endl;
        // cout << 10 << " (" << parent[10] << ")"
        //      << ": " << node_feature[10] << endl;

        // cout << parent[9] << " " << parent[10] << endl;
        // assert(parent[3] == 1);
        // assert(parent[4] == 1);

        print_new_best();

        if (options.verified) {

          vector<int> lchild;
          lchild.resize(node_size, -1);
          Tree T;
          for (auto i{1}; i < node_size; ++i) {
            if (i % 2)
              lchild[parent[i]] = i;
            else
              assert(lchild[parent[i]] == i - 1);
          }
          for (auto i{0}; i < node_size; ++i) {
            if (lchild[i] >= 0)
              T.addNode(lchild[i], lchild[i] + 1, node_feature[i]);
            else
              T.addNode(-1, -1, (P[1][i].count() > P[0][i].count() ? -1 : -2));
          }

          cout << T << endl;
          auto err{T.predict(data)};

          cout << err << " / " << error() << endl;

          assert(err == error());
        }
      }
    }
  }
  n = node_size;
}




void DL8::verify() {
  // for (auto s{0}; s < num_node; ++s) {
  // }
}

std::ostream &DL8::display(std::ostream &os) const {
  //
  // vector<>
  //
  //
  //   for (auto i : blossom) {
  //     cout << i << ": " << P[0][i].count() << "/" << P[1][i].count() << endl;
  //   }

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

std::ostream &operator<<(std::ostream &os, const DL8 &x) {
  return x.display(os);
}
}