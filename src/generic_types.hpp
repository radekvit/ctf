/**
\file generic_types.hpp
\brief Defines types, STL classes and generic STL adapters and functions for
this project.
\author Radek Vít
 */
#ifndef CTF_GT_H
#define CTF_GT_H

#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>
#include <queue>
#include <stack>

namespace ctf {

/*-
TYPES
-*/
using std::string;

/*-
STL
-*/

using std::vector;
using std::list;
using std::map;
using std::set;
using std::queue;
using std::stack;

/**
 \brief STL stack with added functionality. Iterators don't lose validity when
 manipulating with stack.
 */
template <class T>
class tstack {
 protected:
  /**
  \brief Underlying STL list. Keeps iterators valid when modifying tstack.
  */
  std::list<T> list_;

 public:
  using container_type = tstack<T>;
  using value_type = T;
  using size_type = typename list<T>::size_type;
  using reference = tstack<T> &;
  using const_reference = const tstack<T> &;

  using iterator = typename list<T>::iterator;
  using const_iterator = typename list<T>::const_iterator;
  using reverse_iterator = typename list<T>::reverse_iterator;
  using const_reverse_iterator = typename list<T>::const_reverse_iterator;

  /**
  \brief Creates empty tstack.
  */
  tstack() = default;
  /**
  \brief Creates tstack from an initializer list. All constructor variants of
  std::list are applicable.
  \param[in] ilist Initializer list viable for std::list.
  */
  tstack(std::initializer_list<T> ilist) : list_(ilist) {}

  /**
  \returns a reference to the top element of the stack.
  \returns A reference to the top element of the stack.
  */
  T &top() noexcept { return list_.front(); }
  /**
  \brief Returns a constant reference to the top element of the stack.
  \returns A const reference to the top element of the stack.
  */
  const T &top() const noexcept { return list_.front(); }

  /**
  \brief Returns true if there are no elements on the stack.
  \returns True when the stack is empty. False otherwise.
  */
  bool empty() const noexcept { return list_.size() == 0; }
  /**
  \brief Returns the number of elements on the stack.
  \returns The number of elements on the stack.
  */
  size_type size() const noexcept { return list_.size(); }
  /**
  \brief Removes all elements from the stack.
  */
  void clear() noexcept { list_.clear(); }
  /**
  \brief Pushes an element to the stack.
  \param[in] t Element to be pushed to stack.
  */
  void push(const T &t) {  // TODO: if C++17, return reference
    list_.emplace_front(t);
  }
  /**
  \brief Pops the top element from the stack and returns it.
  \returns The element that was on the top of the stack before its removal.
  */
  T pop() noexcept {
    T temp{list_.front()};
    list_.pop_front();
    return temp;
  }
  /**
  \brief Searches for an element between a given position and the end.
  \param[in] target The reference element for the search.
  \param[in] it The first element of the search.
  \param[in] predicate Predicate to find the targer. Defaults to operator ==.
  \returns An iterator to the element defined by target and
  predicate. If no element fits the criteria, the returned iterator is
  equal to tstack::end().
  */
  iterator search(const T &target, iterator from,
                  std::function<bool(const T &, const T &)> predicate =
                      [](auto &lhs, auto &rhs) { return lhs == rhs; }) {
    iterator it;
    for (it = from; it != list_.end(); ++it) {
      if (predicate(*it, target))
        break;
    }
    return it;
  }
  /**
  \brief Searches for an element in a const tstack.
  \param[in] target The reference element for the search.
  \param[in] it The first element of the search.
  \param[in] predicate Predicate to find the target. Defaults to operator ==.
  \returns A const iterator to the element defined by target and
  predicate. If no element fits the criteria, the returned iterator is
  equal to tstack::cend().
  */
  const_iterator search(const T &target, const_iterator from,
                        std::function<bool(const T &, const T &)> predicate =
                            [](auto &lhs, auto &rhs) {
                              return lhs == rhs;
                            }) const {
    const_iterator it;
    for (it = from; it != list_.cend(); ++it) {
      if (predicate(*it, target))
        break;
    }
    return it;
  }
  /**
  \brief Searches for an element.
  \param[in] target The reference element for the search.
  \param[in] predicate Predicate to find the targer. Defaults to operator ==.
  \returns An iterator to the element defined by target and
  predicate. If no element fits the criteria, the returned iterator is
  equal to tstack::end().
  */
  iterator search(const T &target,
                  std::function<bool(const T &, const T &)> predicate =
                      [](auto &lhs, auto &rhs) { return lhs == rhs; }) {
    return search(target, begin(), predicate);
  }
  /**
  \brief Searches for an element in a const tstack.
  \param[in] target The reference element for the search.
  \param[in] predicate Predicate to find the target. Defaults to operator ==.
  \returns A const iterator to the element defined by target and
  predicate. If no element fits the criteria, the returned iterator is
  equal to tstack::cend().
  */
  const_iterator search(const T &target,
                        std::function<bool(const T &, const T &)> predicate =
                            [](auto &lhs, auto &rhs) {
                              return lhs == rhs;
                            }) const {
    return search(target, begin(), predicate);
  }
  /**
  \brief Replaces the element at the position given by an iterator with elements
  in the
  string.
  \param[in] it An iterator to the element that is to be removed. If the
  iterator equals tstack::end(), this does nothing.
  \param[in] string A string of elements to be pushed to tstack at the position
  given by iterator.
  \returns Iterator to the first element from string on the stack.

  The first element in the string will be closest to top of the stack.
  */
  template <class TS>
  iterator replace(iterator it, const TS &string) {
    if (it == list_.end())
      return it;
    auto insert = it;
    ++insert;
    for (auto &t : string) {
      list_.insert(insert, t);
    }
    list_.erase(it++);
    return it;
  }
  /**
  \brief Replaces an element defined by a target, its first possible position,
  and a predicate by a string of elements.
  \param[in] target The reference element for the search.
  \param[in] string A string of elements to be pushed to tstack instead of the
  given element.
  \param[in] predicate Predicate to find the target. Defaults to operator ==. If
  no element on the stack makes this predicate true, nothing happens.
  \returns An iterator to the element defined by target and
  predicate. If no element fits the criteria, the returned iterator is
  equal to tstack::end().

  The first element in the string will be closest to top of the
  stack.
  */
  template <class TS>
  iterator replace(const T &target, const TS &string, iterator from,
                   std::function<bool(const T &, const T &)> predicate =
                       [](auto &lhs, auto &rhs) { return lhs == rhs; }) {
    return replace(search(target, from, predicate), string);
  }
  /**
  \brief Replaces an element defined by target and a predicate by a string of
  elements.
  \param[in] target The reference element for the search.
  \param[in] string A string of elements to be pushed to tstack instead of the
  given element.
  \param[in] predicate Predicate to find the target. Defaults to operator ==. If
  no element on the stack makes this predicate true, nothing happens.
  \returns An iterator to the element defined by target and
  predicate. If no element fits the criteria, the returned iterator is
  equal to tstack::end().

  The first element in the string will be closest to top of the
  stack.
  */
  template <class TS>
  iterator replace(const T &target, const TS &string,
                   std::function<bool(const T &, const T &)> predicate =
                       [](auto &lhs, auto &rhs) { return lhs == rhs; }) {
    return replace(search(target, predicate), string);
  }
  /**
  \brief Swaps the contents of this tstack with another tstack.
  */
  void swap(tstack &other) noexcept { std::swap(list_, other.list_); }

  ///@{
  /**
  \brief Returns an iterator to the top element.
  \returns an iterator to the top element.

  If the stack is empty, the returned iterator will be equal to end().
  */
  iterator begin() noexcept { return list_.begin(); }
  const_iterator begin() const noexcept { return list_.begin(); }
  const_iterator cbegin() const noexcept { return list_.cbegin(); }
  ///@}

  ///@{
  /**
  \brief Returns an iterator to the element following the furthest element from
  the top.
  \returns an iterator to the element following the furthest element
  from the top.

  This element acts as a placeholder. Trying to access it results in undefined
  behavior.
  */
  iterator end() noexcept { return list_.end(); }
  const_iterator end() const noexcept { return list_.end(); }
  const_iterator cend() const noexcept { return list_.cend(); }
  ///@}

  ///@{
  /**
  \brief Returns a reverse iterator to the furthest element from the top.
  \returns A reverse iterator to the furthest element from the top.
  */
  reverse_iterator rbegin() noexcept { return list_.rbegin(); }
  const_reverse_iterator rbegin() const noexcept { return list_.rbegin(); }
  const_reverse_iterator crbegin() const noexcept { return list_.crbegin(); }
  ///@}

  ///@{
  /**
  \brief Returns a reverse iterator to the element preceding the top.
  \returns A reverse iterator to the element preceding the top.

  This element acts as a placeholder. Trying to access it results in undefined
  behavior.
  */
  reverse_iterator rend() noexcept { return list_.rend(); }
  const_reverse_iterator rend() const noexcept { return list_.rend(); }
  const_reverse_iterator crend() const noexcept { return list_.crend(); }
  ///@}

  /**
  \name Comparison operators
  \brief Lexicographic comparison of the underlying std::lists.
  \param[in] lhs Left tstack of the comparison.
  \param[in] rhs Right stack of the comparison.
  \returns True if the lexicographic comparison is true.
  */
  ///@{
  friend bool operator==(const tstack<T> &lhs, const tstack<T> &rhs) noexcept {
    return lhs.list_ == rhs.list_;
  }
  friend bool operator!=(const tstack<T> &lhs, const tstack<T> &rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator<(const tstack<T> &lhs, const tstack<T> &rhs) noexcept {
    return lhs.list_ < rhs.list_;
  }
  friend bool operator>(const tstack<T> &lhs, const tstack<T> &rhs) noexcept {
    return rhs.list_ < lhs.list_;
  }
  friend bool operator<=(const tstack<T> &lhs, const tstack<T> &rhs) noexcept {
    return lhs == rhs || lhs < rhs;
  }
  friend bool operator>=(const tstack<T> &lhs, const tstack<T> &rhs) noexcept {
    return rhs <= lhs;
  }
  ///@}
};

/*-
ALGORITHMS
-*/

using std::sort;
using std::unique;

/*-
FUNCTIONS
-*/

/**
\brief Makes contents of a container a set.
\param[in,out] container Container to be made into a set.

Sorts the container and removes duplicates.
*/
template <class T>
void make_set(T &container) {
  // sort the contents
  sort(container.begin(), container.end());
  // remove duplicates
  container.erase(unique(container.begin(), container.end()), container.end());
}

/**
\brief Element presence in a sorted container.
\returns True if element e is contained in sorted container c. False otherwise.
*/
template <class T, class CT>
bool is_in(const CT &c, const T &e) {
  auto it = std::lower_bound(c.begin(), c.end(), e);
  return it != c.end() && *it == e;
}

/**
\brief Set union of two sorted containers.
\returns A set union of the two containers.
*/
template <class T>
T set_union(const T &lhs, const T &rhs) {
  T r;
  std::set_union(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                 back_inserter(r));
  return r;
}

/**
\brief Set union between sorted containers.
\param[in,out] target Targer container. One half of the resulting set. Must be
sorted.
\param[in] addition One half of the resulting set. Must be sorted.
\returns True if the set union between the two containers changed the
target container. False otherwise.
*/
template <class CT>
bool modify_set(CT &target, const CT &addition) {
  auto before = target.size();
  target = set_union(target, addition);
  return before != target.size();
}

/*-
ADAPTERS
-*/

/**
 Adapter for non-const classes to reverse their regular iterator.
 */
template <class T>
class reverser {
  /**
  \brief Reference to the reversed container.
  */
  T &ref;

 public:
  reverser() = delete;
  /**
  \brief Constructs a reverser.
  \param[in] _t Container that is to be reversed.
  */
  reverser(T &_t) : ref(_t) {}

  auto begin() { return ref.rbegin(); }
  auto end() { return ref.rend(); }

  auto rbegin() { return ref.begin(); }
  auto rend() { return ref.end(); }
};
/**
Adapter for const classes to reverse their iterator.
*/
template <class T>
class const_reverser {
  /**
  \brief Const reference to the reversed containter.
  */
  const T &ref;

 public:
  const_reverser() = delete;
  /**
  \brief Constructs a const_reverser.
  \param[in] _t Container to be reversed.
  */
  const_reverser(const T &_t) : ref(_t) {}

  auto begin() const { return ref.rbegin(); }
  auto end() const { return ref.rend(); }
  auto rbegin() const { return ref.begin(); }
  auto rend() const { return ref.end(); }
};

/**
\brief Reverses a container.
\param[in] t Container to be reversed.
\returns Container reversing adapter.
*/
template <class T>
reverser<T> reverse(T &t) {
  return reverser<T>(t);
}
/**
\brief Reverses a const container.
\param[in] t Container to be reversed.
\returns Container reversing adapter.
*/
template <class T>
const const_reverser<T> reverse(const T &t) {
  return const_reverser<T>(t);
}

/**
\brief Constructs STL from different STL using iterators.
\param[in] it Container from which the target container is constructed.
\returns A container with all elements of the first container.
*/
template <class IT, class OT>
OT transform(const IT &it) {
  return OT{begin(it), end(it)};
}

}  // namespace ctf
#endif
/*** Enf of file generic_types.hpp ***/