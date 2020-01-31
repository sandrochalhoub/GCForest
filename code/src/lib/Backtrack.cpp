
#include "Backtrack.hpp"




namespace primer {

BacktrackingAlgorithm::BacktrackingAlgorithm(DataSet& d, const int k) : data(d) {
	auto m{data.numFeature()};
	// auto n{data.size()};

	feature.resize(k);
	
	leaf.reserve(k);
	leaf.add(0);
		
	for(auto y{0}; y<2; ++y) {
		P[y].copy(data.example[y].begin(), data.example[y].end());
		P[y].addNode();
	}
	
	// the root is its own parent
	parent.resize(k, -1);
	feature.resize(k, -1);
	
  feature_count[0].resize(2 * m);
  feature_count[1].resize(2 * m);

}

void BacktrackingAlgorithm::expend() {
	
	cout << "current leaves: " << leaf << endl;
	
	// auto selected_node{feature.size()};
	// auto max_error{std::min(P[0][selected_node].count(), P[1][selected_node].count())};
	auto selected_node{-1};
	auto max_error{0};
	// for(auto i{selected_node+1}; i<parent.size(); ++i) {
	for(auto i : leaf) {
		auto error{std::min(P[0][i].count(), P[1][i].count())};
		if(error > max_error) {
			max_error = error;
			selected_node = i;
		}
	}
	
	cout << "expand node " << selected_node << endl;
	
	auto selected_feature{0};
	auto lowest_entropy{entropy(selected_node, selected_feature)};
	for(auto f{1}; f<data.numFeature(); ++f) {
		auto feat_entropy{entropy(selected_node, f)};
		if(feat_entropy < lowest_entropy) {
			lowest_entropy = feat_entropy;
			selected_feature = f;
		}
	}
	
	cout << "branch on feature " << selected_feature << " of entropy " << lowest_entropy << endl;
	
	split(selected_node, selected_feature);
	leaf.remove_front(selected_node);
	int child;
	if(leaf.size() < parent.size())
	{
		child = leaf.size();
		parent[child] = selected_node;
		leaf.add(child);
		if(P[0][child].count() == 0 or P[1][child].count() == 0)
			leaf.remove_front(child);
		
		child = leaf.size();
		parent[child] = selected_node;
		leaf.add(child);
		if(P[0][child].count() == 0 or P[1][child].count() == 0)
			leaf.remove_front(child);
	}

}


void BacktrackingAlgorithm::split(const int node, const int feature) {
	
	for(auto y{0}; y<2; ++y) {
		assert(P[y][node].count() > 0);
		P[y].branch(node, [&](const int x){ return data.hasFeature(x, feature); });
	}
}


double BacktrackingAlgorithm::entropy(const int node, const int feature) {

  // cout << "compute entropy of " << featureName(feature) << endl;

  double feature_entropy{0};

  int not_feature = (feature + data.numFeature());
  int truef[2] = {not_feature, feature};

  for (auto y{0}; y < 2; ++y) {
    feature_count[y][feature] = 0;
    feature_count[y][not_feature] = 0;
  }
	
	

  for (auto y{0}; y < 2; ++y) 
	{
		assert(P[y][node].count());
		auto stop{P[y][node].end()};
		for(auto i{P[y][node].begin()}; i!=stop; ++i) {
			++feature_count[y][truef[data.hasFeature(*i,feature)]];
		}
	}

  // double entropy{0};
  double total_size{
      static_cast<double>(P[0][node].count() + P[1][node].count())};

  for (auto x{0}; x < 2; ++x) {

    double val_size{static_cast<double>(feature_count[0][truef[x]] + feature_count[1][truef[x]])};

    // Pr(Y=y|X=x) = (count[y][x] / val_size)

    // H(Y|X=x) = \sum_y (Pr(Y=y|X=x) log2 Pr(Y=y|X=x))
    double entropy_x{0};
    for (auto y{0}; y < 2; ++y) {
      if (feature_count[y][truef[x]] != 0 and
          feature_count[y][truef[x]] != val_size) {
        entropy_x -= (feature_count[y][truef[x]] / val_size) *
                     std::log2(feature_count[y][truef[x]] / val_size);
        // cout << " + " << -(count[c][val] / val_size) *
        // std::log(count[c][val] / val_size);
      }
      // else cout << " + 0";
    }

    // cout << " = " << (entropy_val * val_size / total_size) << endl;

    // Pr(X=x) = val_size / total_size

    // H(Y|X) = \sum_x Pr(X=x) H(Y|X=x)
    feature_entropy += (entropy_x * val_size / total_size);
  }

  // cout << " ==> " << feature_entropy << endl << endl;

  return feature_entropy;
}

std::ostream &BacktrackingAlgorithm::display(std::ostream &os) const {

for (auto y{0}; y < 2; ++y) {
	os << "CLASS " << y << endl;
	os << P[y] << endl;	
}


	// os << "here\n";

	// for(auto i{0}; i<parent.size(); ++i) {
	// 	os << "node " << i << ": p=" << parent[i] ;
	//
	// 	if(count[0][i] == 0)
	// 		os << " positive leaf!\n";
	// 	else if(count[1][i] == 0)
	// 		os << " negative leaf!\n";
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