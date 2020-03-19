
#include <iostream>
#include <vector>

#include "DataSet.hpp"

#ifndef __TREE_HPP
#define __TREE_HPP

/**********************************************
* ListTree
**********************************************/

using namespace std;

namespace primer {

class Tree
{

private:
  ///
  vector<int> left_child;
	vector<int> child[2];
	vector<int> feature;
  //@}

public:
  /*!@name Constructors*/
  //@{
	explicit Tree();
	
	int getFeature(const int node) const;
	
	bool predict(const instance& x) const;
	
	int getLeaf(const instance& x) const;

	int predict(const DataSet& data) const;
	
	int getChild(const int x, const bool t) const;
	
	void addNode(const int p, const int f);
	//@}

  /*!@name Miscellaneous*/
  //@{
  std::ostream &display(std::ostream &os) const;
  //@}
};

std::ostream& operator<<(std::ostream& os, const Tree& x);


}


#endif // __TREE_HPP
