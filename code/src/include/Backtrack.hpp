
#include <iostream>
#include <vector>


#include "DataSet.hpp"
#include "Partition.hpp"
#include "SparseSet.hpp"

#ifndef _PRIMER_BACKTRACK_HPP
#define _PRIMER_BACKTRACK_HPP
	
using namespace boost;
using namespace std;

namespace primer {

/**********************************************
* BacktrackingAlgorithm
**********************************************/
/// Representation of a list of examples
class BacktrackingAlgorithm {

private:
  /*!@name Parameters*/
  //@{
	/// Argument
	DataSet &data;
	
	
	/// store the parent of node i
	vector<int> parent; 
	
	/// store the feature tested at node i
	vector<int> feature;
	
	// internal nodes are popped front, potential nodes are back
	SparseSet leaf;
		
	/// store the number of examples of each class
	// vector<int> count[2];
	
	/// structure to partition the examples in the tree
	TreePartition P[2];
	
	/// buffers to compute the entropy
	vector<int> feature_count[2];
  //@}

public:
  /*!@name Constructors*/
  //@{
  explicit BacktrackingAlgorithm(DataSet& d, const int max_nodes);
  //@}

  /*!@name Accessors*/
  //@{
	
	void expend();
	
	void split(const int node, const int feature);
	
	double entropy(const int node, const int feature);
  //@}

  /*!@name Printing*/
  //@{
  // std::ostream &toCsv(std::ostream &os) const;
  std::ostream &display(std::ostream &os) const;
  //@}

  /*!@name Verification*/
  //@{
  // void verify();
  //@}
};


std::ostream &operator<<(std::ostream &os, const BacktrackingAlgorithm &x);
}

#endif // _PRIMER_BACKTRACK_HPP
