
#include <iostream>

#include <vector>

#ifndef _MINISCHEDULER_SPARSESET_HPP
#define _MINISCHEDULER_SPARSESET_HPP

/**********************************************
 * SparseSet
 **********************************************/
/// Sparse set representation

class SparseSet {
private:
  /*!@name Parameters*/
  //@{
  /// list of values
  std::vector<int> list_;
  size_t size_;
	// so that we can remove from the start
	size_t start_;

  /// values' indices
  std::vector<size_t> index_;
  //@}

public:
  /*!@name Constructors*/
  //@{
  explicit SparseSet(const size_t n = 0);
  explicit SparseSet(std::vector<size_t> &common);

  void reserve(const size_t n);

  /*!@name Accessors*/
  //@{
  bool safe_contain(const int elt) const;
  bool contain(const int elt) const;

  size_t capacity() const;
	size_t count() const;
	size_t start() const;
  size_t size() const;
  bool empty() const;

  int next(const int elt) const;

  int prev(const int elt) const;

  int operator[](const size_t idx) const;

  int &operator[](const size_t idx);
  //@}

  /*!@name List Manipulation*/
  //@{
  std::vector<int>::iterator begin();
  std::vector<int>::reverse_iterator rbegin();

  std::vector<int>::iterator end();
  std::vector<int>::reverse_iterator rend();

  std::vector<int>::const_iterator begin() const;
  std::vector<int>::const_reverse_iterator rbegin() const;

  std::vector<int>::const_iterator end() const;
  std::vector<int>::const_reverse_iterator rend() const;

  // std::vector<int>::iterator begin_not_in();
  // std::vector<int>::reverse_iterator rbegin_not_in();
  //
  // std::vector<int>::iterator end_not_in();
  // std::vector<int>::reverse_iterator rend_not_in();
  //
  // std::vector<int>::const_iterator begin_not_in() const;
  // std::vector<int>::const_reverse_iterator rbegin_not_in() const;
  //
  // std::vector<int>::const_iterator end_not_in() const;
  // std::vector<int>::const_reverse_iterator rend_not_in() const;

  void fill();

  void clear();

  void resize(const size_t n);

  // void move_up(const int elt, const int idx);

  void pop_back();

  void pop_front();

  int front() const;

  int back() const;
	
	template< typename R >
	int any(R& random_generator) const { return list_[start_ + (random_generator() % count())]; }
	

  void push_front(const int elt);
	void push_back(const int elt);
  void add(const int elt);
  void safe_add(const int elt);

  void pull_back(const int elt);
  void remove_back(const int elt);
  // void safe_remove(const int elt);
  void pull_front(const int elt);
  void remove_front(const int elt);

  int index(const int elt) const;

  void save(size_t&, size_t &);
  void restore(const size_t, const size_t);
  //@}
	
  /*!@name Miscellaneous*/
  //@{
  std::ostream &display(std::ostream &os) const;
};

std::ostream &operator<<(std::ostream &os, const SparseSet &x);

#endif // _MINISCHEDULER_SPARSESET_HPP
