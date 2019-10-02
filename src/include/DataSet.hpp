
#include <iostream>
#include <vector>

#include <boost/dynamic_bitset.hpp>

#include "SparseSet.hpp"

#ifndef _MINISCHEDULER_DATASET_HPP
#define _MINISCHEDULER_DATASET_HPP

using namespace boost;

namespace primer {
/**********************************************
 * DataSet
 **********************************************/
/// Example base representation


class DataSet {
public:
  /*!@name Parameters*/
  //@{
  /// list of values
	std::vector<std::string> feature_label;
	
	SparseSet example[2];
	SparseSet explanations[2];
  std::vector<dynamic_bitset<>> X;
  //@}

public:
  /*!@name Constructors*/
  //@{
  explicit DataSet() {
	}
  explicit DataSet(const int nfeatures) {
		for(auto i{1}; i<=nfeatures; ++i)
		feature_label.push_back("f_" + std::to_string(i));
	}
  void reserve(const size_t n) {
  	X.reserve(n);
		example[1].reserve(n);
		example[0].reserve(n);
  }

  /*!@name Accessors*/
  //@{
	dynamic_bitset<>& operator[](const size_t idx) {
		return X[idx];
	}
	void addFeature(string& f) {
		feature_label.push_back(f);
	}
	void add(dynamic_bitset<>& x, const bool y) {
		example[y].safe_add(X.size());
		X.push_back(x);
	}
	size_t numFeature() const {
		return feature_label.size();
	}
	size_t size() const {
		return X.size();
	}
	int NOT(const int f) const {
		return (f+numFeature()/2) % numFeature();
	}
  //@}

  /*!@name List Manipulation*/
  //@{
	// keeps only n positive examples, chosen randomly with a uniform distribution
	template< class random > 
	void uniform_sample(const int c, const size_t n, random generator)
	{
		while(example[c].size() > n) {
			// auto e{generator() % example[c].size()};
			// std::cout << example[c] << " rem " << example[c][e] << std::endl;
			example[c].remove_back( example[c][generator() % example[c].size()] );
		}
	}
  //@}
	
  /*!@name Miscellaneous*/
  //@{
  std::ostream &display(std::ostream &os) const {
  	for(auto i : example[1]) 
  		os << X[i] << ": +" << std::endl;
  	for(auto i : example[0]) 
  		os << X[i] << ": -" << std::endl;
		return os;
  }
	
  std::ostream &display_example(std::ostream &os, const int e) const {
		if(X[e][0])
			os << feature_label[0];
		else if(X[e][numFeature()/2])
			os << feature_label[NOT(0)];
		
		for(auto f{1}; f<numFeature()/2; ++f)
		{
			if(X[e][f])
				os << ", " << feature_label[f];
			else if(X[e][NOT(f)])
				os << ", " << feature_label[NOT(f)];
		}
		
		return os;
  }
	
	
	
	void addExplanation(dynamic_bitset<>& impl, const bool y, vector<int> entailed)
	{
		
		cout << " add expl " << impl << endl;
		
		entailed.clear();
		// remove all explained examples
		for(auto e : example[y]) {
			
			cout << "compare " << e << " = " << X[e] << endl;  
			
			if(impl.is_subset_of(X[e])) {
				cout << " -> entailed!\n"; 
				entailed.push_back(e);
			}
		}
		for(auto e : entailed)
			example[y].remove_front(e);
		
		// add the explanation instead
		add(impl, y);
		
	}
	
	
	void computeRules(Options opt)
	{
		auto c{1};
		
		dynamic_bitset<> buffer;
		
		dynamic_bitset<> candidates;
		
		dynamic_bitset<> implicant;
		
		vector<int> removed;
		
		
		buffer.resize(numFeature());
		
		// int num_initial[2] = {example[0].size(), example[1].size()};
		
		
		// auto i{example[c][0]};
		
		auto last_example{X.size()-1};
		
		while(true) {
			
			auto i{example[c].front()};
			if(i > last_example) {
				c = 1-c; 
				i = example[c].front();
				if(i > last_example)
					break;
			}
			
			if(opt.verbosity >= Options::YACKING)
				cout << "compute a rule from the " << (c ? "positive" : "negative") << " example " << i << ":" << X[i] << endl;
			
		
			implicant.clear();
			candidates.clear();
			implicant.resize(numFeature(), false);
			candidates.resize(numFeature(), true);
		
		
			for(auto j : example[1-c])
			{
				if(!implicant.is_subset_of(X[j]))
				{
					if(opt.verbosity >= Options::YACKING)
						cout << "skip " << j << " = " << X[j] << " b/c it is already covered\n";
					continue;
				}
				
				
				buffer = X[i];
				buffer -= X[j];
			
				if(opt.verbosity >= Options::YACKING)
					cout << X[i] << " \\ " << j << ":" << X[j] << " = " << buffer ;
			
			
				if(buffer.none())
				{
					if(opt.verbosity >= Options::YACKING)
						cout << " inconsistent example\?\?!\n" ;
					continue;
				}
				
			
				if(!candidates.intersects(buffer)) {
					auto f{candidates.find_first()};
					implicant.set(f);
					candidates.clear();
					candidates.resize(numFeature(), true);
				} 
				candidates &= buffer;
			
				if(opt.verbosity >= Options::YACKING)
					cout << " -> " << candidates << " " << implicant << endl;
			}
		
			auto f{candidates.find_first()};
			implicant.set(f);
		
			if(opt.verbosity >= Options::NORMAL)
				cout << " -> " << implicant << " (" << implicant.count() << ")"<< endl << endl;
		
			addExplanation(implicant, c, removed);
			
			
			if(opt.verbosity >= Options::NORMAL)
				for(auto e : removed)
					cout << " - remove " << X[e] << endl;
		
			c = 1 - c;
		}
		
		
		
	}
	
	
};

// template< typename T >
std::ostream &operator<<(std::ostream &os, const DataSet &x) {
	return x.display(os);
}

}

#endif // _MINISCHEDULER_DATASET_HPP
