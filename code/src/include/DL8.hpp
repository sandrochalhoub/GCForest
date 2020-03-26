
#include <iostream>
#include <random>
#include <vector>

#include "DataSet.hpp"
#include "Partition.hpp"
#include "SparseSet.hpp"
#include "Tree.hpp"
#include "utils.hpp"

#ifndef _PRIMER_DL8_HPP
#define _PRIMER_DL8_HPP

using namespace boost;
using namespace std;

namespace primer {

/**********************************************
* BacktrackingAlgorithm
**********************************************/
/// Representation of a list of examples
class DL8 {

private:
  /*!@name Parameters*/
  //@{
  /// Argument
  DataSet &data;
  DTOptions &options;
  vector<vector<int>> example[2];

	// 
	size_t ub_depth;

  // internal nodes are popped front, potential nodes and leaves are back
  SparseSet branch;

  /// structure to partition the examples in the tree
  TreePartition P[2];

  /// best solution
  vector<int> node_error;
  vector<int> node_feature;
	vector<int> parent;
	
	/// for branching
	vector<vector<int>> sorted_feature;

  mt19937 random_generator;

  size_t search_size;


public:

  /*!@name Constructors*/
  //@{
  explicit DL8(DataSet &d, DTOptions &o);
  void resize(const int num_nodes);
  void seed(const int s);
  //@}

  /*!@name Accessors*/
  //@{
  /// the current capacity of the data-structure
  size_t size() const;

  double accuracy() const;

  size_t error() const;
		
	size_t error(const int i) const;

	size_t optimize();


	/// branch contain the positive tests (front), the negative tests (back) and the
	/// available tests (in)
	/// partition s is the one pointed to by the branch
	/// n is the number of nodes with feature tests
	void recurse(SparseSet &branch, int &n, const int d, const int s);

  /*!@name Printing*/
  //@{
	void print_new_best() const;
	
  void verify();
  // std::ostream &toCsv(std::ostream &os) const;
  std::ostream &display(std::ostream &os) const;
  //@}
};

std::ostream &operator<<(std::ostream &os, const DL8 &x);
}

#endif // _PRIMER_BACKTRACK_HPP
