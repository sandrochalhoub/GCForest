
#include <iostream>

#include <vector>

#ifndef __PARTITION_HPP
#define __PARTITION_HPP

/**********************************************
 * ListTree
 **********************************************/

using namespace std;

namespace primer {
	

class Part
{
	
	
private:
  /// 
	vector<int>& element;
  //@}
	
public:
    /*!@name Parameters*/
    //@{
		size_t begin_idx;
		size_t end_idx;

    /*!@name Constructors*/
    //@{
		explicit Part(vector<int>& elt);
		
		Part& operator=(const Part& p) noexcept;
		
    size_t count() const;
		
		vector<int>::iterator begin();
		vector<int>::iterator end();
		
		vector<int>::const_iterator begin() const;
		vector<int>::const_iterator end() const;
		
		template<typename boolean_function>
		void split(Part& l1, Part& l2, boolean_function condition);
		

    /*!@name Miscellaneous*/
    //@{
    std::ostream& display(std::ostream& os) const;
		//@}
};

std::ostream& operator<<(std::ostream& os, const Part& x);

class TreePartition
{
private:
	vector<int> element;
	vector<Part> part;
	
public:
	TreePartition();
	
	Part& operator[](const int i);
	const Part& operator[](const int i) const;
	
	template<typename RandIt>
	void copy(RandIt s, RandIt e);
	
	size_t addNode();
	
	template<typename boolean_function>
	void branch(const int node, boolean_function condition);
	
  /*!@name Miscellaneous*/
  //@{
  std::ostream& display(std::ostream& os) const;
	//@}
	
};


std::ostream& operator<<(std::ostream& os, const TreePartition& x);



template<typename boolean_function>
void Part::split(Part& l1, Part& l2, boolean_function condition) {
	auto i{begin()};
	auto j{end()};
	
	assert(begin() <= end());
	
	
	
	while(true) {
		
		cout << "split " << (i - begin()) << ".." << (j-begin()) << endl;
		// cout << "split " << *i << ".." << *(j-1) << endl;
		
		while(i < j and condition(*i)) ++i;
		if(i+1 >= j)
			break;
		
		while(j > i and not condition(*(--j)));
		if(i == j)
			break;
		
		cout << "swap " << *i << ".." << *(j-1) << endl;
		
		std::swap(*i,*j);
		
		assert(condition(*i));
		assert(not condition(*j));
		
		++i;
	}

	cout << "stop on " << (i - begin()) << ".." << (j-begin()) << endl;


	assert(i == end() or not condition(*i));
	assert(i == begin() or condition(*(i-1)));	



	


	l1.begin_idx = begin_idx;
	l1.end_idx = i - begin();

	l2.begin_idx = l1.end_idx;
	l2.end_idx = end_idx;
	
	cout << begin_idx << " " << l1.begin_idx << endl;
	cout << (i - begin()) << " " << l1.end_idx << endl;
	cout << l1.end_idx << " " << l2.begin_idx << endl;
	cout << end_idx << " " << l2.end_idx << endl;
	
	
	cout << "split result: " << count() << " -> " << l1.count() << " / " << l2.count() << endl;
	
	
	assert(l1.begin() <= l1.end());
	assert(l2.begin() <= l2.end());
	
}


template<typename RandIt>
void TreePartition::copy(RandIt s, RandIt e) {
	element.resize(std::distance(s,e));
	std::copy(s, e, element.begin());
}

template<typename boolean_function>
void TreePartition::branch(const int node, boolean_function condition) {
	auto c1{addNode()};
	auto c2{addNode()};
	part[node].split(part[c1], part[c2], condition);
}




}


#endif // __PARTITION_HPP
