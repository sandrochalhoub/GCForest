
#include <assert.h>

#include "Tree.hpp"

#define POSITIVE -1

using namespace std;

namespace primer {

Tree::Tree() {}

int Tree::getFeature(const int node) const {
	return feature[node];
}

void Tree::addNode(const int c, const int f) {
	left_child.push_back(c);
	child[0].push_back(c+1);
	child[1].push_back(c);
	feature.push_back(f);
}

int Tree::getChild(const int x, const bool t) const {
	
	assert(left_child[x]+1-t == child[t][x]);
	
	return left_child[x]+1-t;
}

bool Tree::predict(const instance& x) const {
	
	// // cout << x << endl << "0";
	//
	//
	//
	//
	// int node{0};
	// while(feature[node] >= 0) {
	//
	// 	// cout << " (" << (x[feature[node]] ? feature[node] : -feature[node]) ;
	//
	// 	node = child(node, x[feature[node]]);
	// 	// cout << ") -> " << node;
	// }
	// // cout << " => " << feature[node] << endl;
	return feature[getLeaf(x)] == POSITIVE;
}

int Tree::getLeaf(const instance& x) const {
	
	// cout << x << endl << "0";
	
	
	
	
	int node{0};
	while(feature[node] >= 0) {
		
		// cout << " (" << (x[feature[node]] ? feature[node] : -feature[node]) ;
		
		node = getChild(node, x[feature[node]]);
		// cout << ") -> " << node;
	}
	// cout << " => " << feature[node] << endl;
	return node;
}

int Tree::predict(const DataSet& data) const {
	
	vector<int> count[2];
	count[0].resize(left_child.size(), 0);
	count[1].resize(left_child.size(), 0);
	
	
	instance used_feature;
	used_feature.resize(data.numFeature(), 0);
	
	for (auto i{0}; i<left_child.size(); ++i) 
		if(feature[i] >= 0) {
		
			// cout << feature[i] << "/" << 	used_feature.size() << endl;
		
			used_feature.set(feature[i]);
			
			
		}
	// cout << used_feature << endl;
		
	
	auto error{0};
	for(auto y{0}; y<2; ++y) {
		for(auto i : data.example[y]) {
			
			// for(auto f{0}; f<data.numFeature(); ++f)
			// 	if(used_feature[f])
			// 		cout << " " << (data[i][f] ? f : -f) ;
			// cout << endl;
			
			auto p{predict(data[i])};
			
			++count[p][getLeaf(data[i])];
			
			// cout << p;
			
			if(p != y) 
				++error;
		}
		// cout << endl;
	}
	
	
	
	// for (auto i{0}; i<left_child.size(); ++i)
	// 	if(feature[i] < 0) {
	//
	// 		cout << i << ": " << count[0][i] << "/" << count[1][i] << endl;
	//
	// 	}
	
	return error;
}


std::ostream &Tree::display(std::ostream &os) const {

	// os << "NODES\n";
  for (auto i{0}; i<left_child.size(); ++i) {
    os << i << " " << feature[i] ;
		if(feature[i]>=0)
			os << " " << getChild(i, true) << " " << getChild(i, false) ;
		os << endl;
  }
	// os << "EDGES\n";
	// for (auto i{0}; i<parent.size(); ++i) {
	//
	// }

  return os;
}

std::ostream &operator<<(std::ostream &os, const Tree &x) {
  x.display(os);
  return os;
}


}
