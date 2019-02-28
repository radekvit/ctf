/**
\file generic_types.hpp
\brief Defines types, STL classes and generic STL adapters and functions used in
this project.
\author Radek Vít
 */
#ifndef CTF_GENERIC_TYPES_H
#define CTF_GENERIC_TYPES_H

#include <algorithm>
#include <cassert>
#include <functional>
#include <list>
#include <map>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ctf {

/*-
TYPES
-*/
using std::string;

/*-
STL
-*/

using std::deque;
using std::list;
using std::map;
using std::pair;
using std::queue;
using std::stack;
using std::unordered_map;
using std::vector;
using std::tuple;

/**
\brief A set implementation optimized for smaller sets.
The guarantee for log(N) insertion is not fulfilled contrary to std::set,
but for all applications in ctf, this implementation should be substantially
faster.

The API is the same as std::set.
*/
template <typename T, class Compare = std::less<T>, class Equals = std::equal_to<T>>
class vector_set {
 public:
  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;

  using iterator = typename vector<T>::iterator;
  using const_iterator = typename vector<T>::const_iterator;
  using reverse_iterator = typename vector<T>::reverse_iterator;
  using const_reverse_iterator = typename vector<T>::const_reverse_iterator;

  struct insert_return_type {
    bool inserted;
    iterator it;
  };

  vector_set() : _compare(Compare()), _equals(Equals()) {}
  explicit vector_set(const Compare& compare) : _compare(compare), _equals(Equals()) {}
  vector_set(std::initializer_list<T> il,
             const Compare& compare = Compare(),
             const Equals& equals = Equals())
    : _elements(il), _compare(compare), _equals(equals) {
    std::sort(_elements.begin(), _elements.end(), _compare);
    auto newEnd = std::unique(_elements.begin(), _elements.end(), _equals);
    _elements.erase(newEnd, _elements.end());
  }

  vector_set(const vector_set&) = default;
  vector_set(vector_set&&) = default;

  vector_set& operator=(const vector_set&) = default;
  vector_set& operator=(vector_set&&) = default;
  vector_set& operator=(std::initializer_list<T> il) {
    _elements = il;
    std::sort(_elements.begin(), _elements.end(), Compare());
    auto newEnd = std::unique(_elements.begin(), _elements.end());
    _elements.erase(newEnd, _elements.end());

    return *this;
  }

  iterator begin() { return _elements.begin(); }
  const_iterator begin() const { return _elements.cbegin(); }
  const_iterator cbegin() const { return _elements.cbegin(); }

  iterator end() { return _elements.end(); }
  const_iterator end() const { return _elements.cend(); }
  const_iterator cend() const { return _elements.cend(); }

  reverse_iterator rbegin() { return _elements.rbegin(); }
  const_reverse_iterator rbegin() const { return _elements.crbegin(); }
  const_reverse_iterator crbegin() const { return _elements.crbegin(); }

  reverse_iterator rend() { return _elements.rend(); }
  const_reverse_iterator rend() const { return _elements.crend(); }
  const_reverse_iterator crend() const { return _elements.crend(); }

  bool empty() const noexcept { return _elements.empty(); }
  size_type size() const noexcept { return _elements.size(); }
  size_type max_size() const noexcept { return _elements.max_size(); }

  void clear() noexcept { _elements.clear(); }

  void shrink_to_fit() { _elements.shrink_to_fit(); }

  T& operator[](size_t i) & noexcept { return _elements[i]; }
  const T& operator[](size_t i) const& noexcept { return _elements[i]; }
  T&& operator[](size_t i) && noexcept { return std::move(_elements[i]); }

  insert_return_type insert(const T& element) {
    auto it = lower_bound(element);
    if (it == _elements.end() || !_equals(*it, element)) {
      return {true, _elements.insert(it, element)};
    }
    return {false, _elements.end()};
  }

  insert_return_type insert(T&& element) {
    auto it = lower_bound(element);
    if (it == _elements.end() || !_equals(*it, element)) {
      return {true, _elements.insert(it, element)};
    }
    return {false, _elements.end()};
  }

  bool erase(const T& element) noexcept {
    auto it = lower_bound(element);
    if (it == _elements.end() || !_equals(*it, element)) {
      return false;
    }
    _elements.erase(it);
    return true;
  }

  void erase(iterator it) { _elements.erase(it); }

  void swap(vector_set& other) {
    _elements.swap(other._elements);
    std::swap(_compare, other._compare);
  }

  bool contains(const T& element) const noexcept {
    auto it = lower_bound(element);
    return it != _elements.end() && _equals(*it, element);
  }

  size_t count(const T& element) noexcept { return find(element) ? 1 : 0; }

  iterator find(const T& element) noexcept {
    auto it = lower_bound(element);
    if (it != end() && _equals(*it, element))
      return it;
    return end();
  }

  const_iterator find(const T& element) const noexcept {
    auto it = lower_bound(element);
    if (it != end() && _equals(*it, element))
      return it;
    return end();
  }

  iterator lower_bound(const T& element) noexcept {
    return std::lower_bound(begin(), end(), element, _compare);
  }

  const_iterator lower_bound(const T& element) const noexcept {
    return std::lower_bound(begin(), end(), element, _compare);
  }

  iterator upper_bound(const T& element) noexcept {
    return std::upper_bound(begin(), end(), element, _compare);
  }

  const_iterator upper_bound(const T& element) const noexcept {
    return std::upper_bound(begin(), end(), element, _compare);
  }

  friend bool operator==(const vector_set& lhs, const vector_set& rhs) {
    return lhs._elements == rhs._elements;
  }
  friend bool operator!=(const vector_set& lhs, const vector_set& rhs) {
    return lhs._elements != rhs._elements;
  }
  friend bool operator<(const vector_set& lhs, const vector_set& rhs) {
    return lhs._elements < rhs._elements;
  }
  friend bool operator<=(const vector_set& lhs, const vector_set& rhs) {
    return lhs._elements <= rhs._elements;
  }
  friend bool operator>=(const vector_set& lhs, const vector_set& rhs) {
    return lhs._elements >= rhs._elements;
  }
  friend bool operator>(const vector_set& lhs, const vector_set& rhs) { return rhs < lhs; }

  friend bool subset(const vector_set& lhs, const vector_set& rhs) {
    for (auto&& e : lhs._elements) {
      if (!rhs.contains(e)) {
        return false;
      }
    }
    return true;
  }
  friend bool proper_subset(const vector_set& lhs, const vector_set& rhs) {
    return lhs._elements.size() < rhs._elements.size() && subset(lhs, rhs);
  }
  friend bool disjoint(const vector_set& lhs, const vector_set& rhs) {
    return set_intersection(lhs, rhs).size() == 0;
  }

  friend vector_set set_union(const vector_set& lhs, const vector_set& rhs) {
    vector<T> vec;
    vec.reserve(lhs.size() + rhs.size());
    std::set_union(lhs.begin(),
                   lhs.end(),
                   rhs.begin(),
                   rhs.end(),
                   std::back_inserter<vector<T>>(vec),
                   lhs._compare);
    return vector_set(vec, lhs._compare, lhs._equals);
  }

  friend vector_set set_intersection(const vector_set& lhs, const vector_set& rhs) {
    vector<T> vec;
    vec.reserve(lhs.size());
    std::set_intersection(lhs.begin(),
                          lhs.end(),
                          rhs.begin(),
                          rhs.end(),
                          std::back_inserter<vector<T>>(vec),
                          lhs._compare);
    return vector_set(vec, lhs._compare, lhs._equals);
  }

  bool modify_set_union(const vector_set& other) {
    size_type oldSize = size();
    *this = set_union(*this, other);
    return oldSize != size();
  }

  vector_set split(size_t i) {
    vector<T> vec = {_elements.begin() + i, _elements.end()};
    _elements.erase(_elements.begin() + i, _elements.end());
    return vector_set(vec, _compare, _equals);
  }

 private:
  vector<T> _elements;

  Compare _compare;
  Equals _equals;

  vector_set(vector<T>& vec, Compare compare, Equals equals)
    : _elements(vec), _compare(compare), _equals(equals) {}
  vector_set(vector<T>&& vec, Compare&& compare, Equals&& equals)
    : _elements(vec), _compare(compare), _equals(equals) {}
};

class bit_set {
 protected:
  using storage_type = size_t;
  static_assert(std::is_unsigned<storage_type>::value, "storage_type must be unsigned");
  friend struct ::std::hash<bit_set>;

 public:
  class reference {
    friend class bit_set;
    using storage_type = bit_set::storage_type;

   public:
    reference& operator=(bool t) noexcept {
      storage_type e = *_element;
      // reset bit
      e &= ~(static_cast<storage_type>(0x1) << _offset);
      // set
      *_element = e | static_cast<storage_type>(t) << _offset;
      return *this;
    }
    reference& operator=(const reference& r) noexcept {
      *this = static_cast<bool>(r);
      return *this;
    }

    operator bool() const noexcept { return (*_element >> _offset) & 0x1; }

    bool operator~() const noexcept { return !((*_element >> _offset) & 0x1); }

    reference& flip() noexcept { return *this = ~*this; }

   private:
    reference(storage_type* p, size_t offset) noexcept : _element(p), _offset(offset) {}

    storage_type* _element;
    size_t _offset;
  };

  explicit bit_set(size_t bits)
    : _storage(bits != 0 ? (bits - 1) / bitsPerStorage + 1 : 0, 0), _capacity(bits) {}

  friend bool operator==(const bit_set& lhs, const bit_set& rhs) {
    assert(lhs._storage.capacity() == rhs._storage.capacity());
    for (size_t i = 0; i < lhs._storage.size(); ++i) {
      if (lhs._storage[i] != rhs._storage[i])
        return false;
    }
    return true;
  }
  friend bool operator!=(const bit_set& lhs, const bit_set& rhs) {
    assert(lhs._storage.capacity() == rhs._storage.capacity());
    for (size_t i = 0; i < lhs._storage.size(); ++i) {
      if (lhs._storage[i] == rhs._storage[i])
        return false;
    }
    return true;
  }

  bool operator[](size_t i) const noexcept { return get_value(i); }
  reference operator[](size_t i) noexcept { return get_reference(i); }

  bool test(size_t i) const {
    if (i >= capacity()) {
      throw std::out_of_range("bit_set::test(): out of range.");
    }
    return get_value(i);
  }

  bool all() const noexcept {
    for (auto& cell : _storage) {
      if (cell != std::numeric_limits<storage_type>::max()) {
        return false;
      }
    }
    return true;
  }

  bool any() const noexcept { return !none(); }

  bool none() const noexcept {
    for (auto& cell : _storage) {
      if (cell != 0) {
        return false;
      }
    }
    return true;
  }

  bool empty() const noexcept { return none(); }

  size_t count() const noexcept {
    size_t result = 0;
    size_t j = 0;
    const storage_type* s = &(_storage[0]);
    storage_type test = *s;
    for (size_t i = 0; i < capacity(); ++i) {
      if (j == bitsPerStorage) {
        j = 0;
        ++s;
        test = *s;
      }
      result += static_cast<bool>(test & (static_cast<storage_type>(0x1) << (bitsPerStorage - 1)));
      test <<= 1;
      ++j;
    }
    return result;
  }

  size_t size() const noexcept { return count(); }
  size_t capacity() const noexcept { return _capacity; }

  bit_set& operator&=(const bit_set& rhs) noexcept {
    for (size_t i = 0; i < _storage.size(); ++i) {
      _storage[i] &= rhs._storage[i];
    }
    return *this;
  }
  bit_set& operator|=(const bit_set& rhs) noexcept {
    for (size_t i = 0; i < _storage.size(); ++i) {
      _storage[i] |= rhs._storage[i];
    }
    return *this;
  }
  bit_set& operator^=(const bit_set& rhs) noexcept {
    for (size_t i = 0; i < _storage.size(); ++i) {
      _storage[i] ^= rhs._storage[i];
    }
    correct_trailing();
    return *this;
  }
  bit_set operator~() {
    bit_set result(*this);
    for (size_t i = 0; i < _storage.size(); ++i) {
      result._storage[i] = ~_storage[i];
    }
    result.correct_trailing();
    return result;
  }

  string to_string(string (*string_fn)(size_t) = std::to_string) const {
    bool any = false;
    string result = "{ ";
    for (size_t i = 0; i < capacity(); ++i) {
      if ((*this)[i]) {
        any = true;
        result += string_fn(i) + ", ";
      }
    }
    if (!any) {
      return "{}";
    }
    result.pop_back();
    result.pop_back();
    result += " }";
    return result;
  }

  bool set_union(const bit_set& rhs) noexcept {
    assert(capacity() == rhs.capacity());
    bool changed = false;
    for (size_t i = 0; i < _storage.size(); ++i) {
      storage_type old = _storage[i];
      _storage[i] |= rhs._storage[i];
      changed |= _storage[i] != old;
    }
    return changed;
  }

  bool set_intersection(const bit_set& rhs) noexcept {
    assert(capacity() == rhs.capacity());
    bool changed = false;
    for (size_t i = 0; i < _storage.size(); ++i) {
      changed |= _storage[i] != rhs._storage[i];
      _storage[i] &= rhs._storage[i];
    }
    return changed;
  }

  // precondition: both sets are the same size
  friend bit_set operator|(const bit_set& lhs, const bit_set& rhs) {
    bit_set result(lhs);
    result |= rhs;
    return result;
  }
  friend bit_set operator&(const bit_set& lhs, const bit_set& rhs) {
    bit_set result(lhs);
    result &= rhs;
    return result;
  }
  friend bit_set operator^(const bit_set& lhs, const bit_set& rhs) {
    bit_set result(lhs);
    result ^= rhs;
    return result;
  }

 protected:
  static constexpr size_t bitsPerStorage = sizeof(storage_type) * 8;

  std::vector<storage_type> _storage;
  size_t _capacity;

  reference get_reference(size_t i) {
    return reference(&(_storage[i / bitsPerStorage]), bitsPerStorage - (i % bitsPerStorage + 1));
  }

  bool get_value(size_t i) const noexcept {
    return ((_storage[i / bitsPerStorage]) >> (bitsPerStorage - (i % bitsPerStorage + 1))) & 0x1;
  }

  void correct_trailing() noexcept {
    if (capacity() == 0)
      return;
    // mask trailing bits
    storage_type mask = std::numeric_limits<storage_type>::max()
                        << (_storage.size() * bitsPerStorage - capacity());
    _storage.back() &= mask;
  }

  size_t hash() const noexcept {
    size_t seed = capacity();
    for (auto& i : _storage) {
      seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};

/**
 \brief Translation stack. Similar to STL stack with extra search and replace
 operations.
 */
template <class T>
class tstack {
 public:
  using container_type = tstack<T>;
  using value_type = T;
  using size_type = typename list<T>::size_type;
  using reference = tstack<T>&;
  using const_reference = const tstack<T>&;

  using iterator = typename list<T>::iterator;
  using const_iterator = typename list<T>::const_iterator;
  using reverse_iterator = typename list<T>::reverse_iterator;
  using const_reverse_iterator = typename list<T>::const_reverse_iterator;

  /**
  \brief Creates empty tstack.
  */
  tstack() = default;
  /**
  \brief Creates tstack from initializer list.

  \param[in] ilist Initializer list containing all elements. The first element
  is at top of the stack.
  */
  tstack(std::initializer_list<T> ilist) : _list(ilist) {}
  tstack(const tstack&) = default;
  tstack(tstack&&) = default;

  tstack& operator=(const tstack&) = default;
  tstack& operator=(tstack&&) = default;
  /**
  \brief Is the tstack empty predicate.
  \returns True when the tstack is empty. False otherwise.
  */
  bool empty() const noexcept { return _list.empty(); }
  /**
  \brief Get the number of elements on the tstack.
  \returns The number of elements on the tstack.
  */
  size_type size() const noexcept { return _list.size(); }
  /**
  \brief Removes all elements from the tstack.
  */
  void clear() noexcept { _list.clear(); }
  /**
  \brief Pushes an element to the tstack.
  \param[in] args Arguments for the construction of T.
  */
  template <typename... Args>
  void push(Args&&... args) {
    _list.emplace_front(std::forward<Args>(args)...);
  }
  /**
  \brief Get a reference to the top element of the tstack.
  \returns A reference to the top element of the tstack.
  */
  T& top() noexcept { return _list.front(); }
  /**
  \brief Get a constant reference to the top element of the tstack.
  \returns A const reference to the top element of the tstack.
  */
  const T& top() const noexcept { return _list.front(); }
  /**
  \brief Pops the top element from the tstack and returns it.
  \returns The element that was on the top of the tstack before its removal.
  */
  T pop() noexcept {
    T temp{std::move(_list.front())};
    _list.pop_front();
    return std::move(temp);
  }
  /**
  \brief Get a reference to the bottom element of the tstack.
  \returns A reference to the bottom element of the tstack.
  */
  T& bottom() noexcept { return _list.back(); }
  /**
  \brief Get a constant reference to the bottom element of the tstack.
  \returns A const reference to the bottom element of the tstack.
  */
  const T& bottom() const noexcept { return _list.back(); }
  /**
  \brief Pops the bottom element from the tstack and returns it.
  \returns The element that was on the bottom of the tstack before its removal.
  */
  T pop_bottom() noexcept {
    T temp{std::move(_list.front())};
    _list.pop_back();
    return std::move(temp);
  }
  /**
  \brief Searches for an element between a given position and the end.

  \param[in] target The reference element for the search.
  \param[in] it The first element of the search.
  \param[in] predicate Predicate to find the target. Defaults to operator ==.

  \returns An iterator to the element defined by target and
  predicate. If no element fits the criteria, the returned iterator is
  equal to tstack::end().
  */
  iterator search(const T& target,
                  iterator from,
                  bool (*predicate)(const T&, const T&) = [](auto& lhs, auto& rhs) {
                    return lhs == rhs;
                  }) {
    iterator it;
    for (it = from; it != _list.end(); ++it) {
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
  const_iterator search(const T& target,
                        const_iterator from,
                        bool (*predicate)(const T&, const T&) = [](auto& lhs, auto& rhs) {
                          return lhs == rhs;
                        }) const {
    const_iterator it;
    for (it = from; it != _list.cend(); ++it) {
      if (predicate(*it, target))
        break;
    }
    return it;
  }
  /**
  \brief Searches for an element.

  \param[in] target The reference element for the search.
  \param[in] predicate Predicate to find the target. Defaults to operator ==.

  \returns An iterator to the element defined by target and
  predicate. If no element fits the criteria, the returned iterator is
  equal to tstack::end().
  */
  iterator search(const T& target,
                  bool (*predicate)(const T&, const T&) = [](auto& lhs, auto& rhs) {
                    return lhs == rhs;
                  }) {
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
  const_iterator search(const T& target,
                        bool (*predicate)(const T&, const T&) = [](auto& lhs, auto& rhs) {
                          return lhs == rhs;
                        }) const {
    return search(target, begin(), predicate);
  }
  /**
  \brief Searches for an element between a given position and the beginning.

  \param[in] target The reference element for the search.
  \param[in] it The first element of the backward search.
  \param[in] predicate Predicate to find the target. Defaults to operator ==.

  \returns An iterator to the element defined by target and
  predicate. If no element fits the criteria, the returned iterator is
  equal to tstack::end().
  */
  iterator search_last(const T& target,
                       iterator from,
                       bool (*predicate)(const T&, const T&) = [](auto& lhs, auto& rhs) {
                         return lhs == rhs;
                       }) {
    iterator it;
    for (it = from; it != _list.begin(); --it) {
      if (predicate(*it, target))
        return it;
    }
    if (predicate(*it, target))
      return it;
    return _list.end();
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
  const_iterator search_last(const T& target,
                             const_iterator from,
                             bool (*predicate)(const T&, const T&) = [](auto& lhs, auto& rhs) {
                               return lhs == rhs;
                             }) const {
    const_iterator it;
    for (it = from; it != _list.cbegin(); --it) {
      if (predicate(*it, target))
        return it;
    }
    if (predicate(*it, target))
      return it;
    return _list.end();
  }
  /**
  \brief Searches for an element.

  \param[in] target The reference element for the search.
  \param[in] predicate Predicate to find the target. Defaults to operator ==.

  \returns An iterator to the element defined by target and predicate. If no
  element fits the criteria, the returned iterator is equal to tstack::end().
  */
  iterator search_last(const T& target,
                       bool (*predicate)(const T&, const T&) = [](auto& lhs, auto& rhs) {
                         return lhs == rhs;
                       }) {
    if (begin() == end())
      return end();
    return search(target, --end(), predicate);
  }
  /**
  \brief Searches for an element in a const tstack.

  \param[in] target The reference element for the search.
  \param[in] predicate Predicate to find the target. Defaults to operator ==.

  \returns A const iterator to the element defined by target and predicate. If
  no element fits the criteria, the returned iterator is equal to
  tstack::cend().
  */
  const_iterator search_last(const T& target,
                             bool (*predicate)(const T&, const T&) = [](auto& lhs, auto& rhs) {
                               return lhs == rhs;
                             }) const {
    if (begin() == end())
      return end();
    return search(target, --end(), predicate);
  }

  struct ReplaceResult {
    iterator begin;
    iterator end;

    friend bool operator==(const ReplaceResult& lhs, const ReplaceResult& rhs) {
      return lhs.begin == rhs.begin && lhs.end == rhs.end;
    }
  };
  /**
  \brief Replaces the element at the position given by an iterator with elements
  in the string.

  \param[in] it An iterator to the element that is to be removed. If the
  iterator equals tstack::end(), this does nothing.
  \param[in] string A string of elements to be pushed to tstack at the position
  given by iterator.

  \returns The iterator range that the inserted string occupies.

  The first element in the string will be closest to top of the tstack.
  */
  template <class TS>
  ReplaceResult replace(iterator it, const TS& string) {
    if (it == _list.end())
      return {it, it};
    auto insert = it;
    ++insert;
    for (auto& t : string) {
      _list.insert(insert, t);
    }
    _list.erase(it++);
    return {it, insert};
  }
  /**
  \brief Replaces the first element matched by the given target and predicate
  with a string of elements.

  \param[in] target The reference element for the search.
  \param[in] string A string of elements to be pushed to tstack instead of the
  matched element. \param[in] predicate The predicate used to match the target.
  Defaults to operator ==.

  \returns An iterator to the element defined by target and
  predicate. If no element fits the criteria, the returned iterator is range
  equal to tstack::end().

  If no element on the tstack is matched, nothing happens.
  The first element in the string will be closest to top of the tstack after
  this operation.
  */
  template <class TS>
  iterator replace(const T& target,
                   const TS& string,
                   bool (*predicate)(const T&, const T&) = [](auto& lhs, auto& rhs) {
                     return lhs == rhs;
                   }) {
    return replace(search(target, predicate), string).begin;
  }
  /**
  \brief Replaces the first element from the given iterator matched by the given
  target and predicate with a string of elements.

  \param[in] target The reference element for the search.
  \param[in] string A string of elements to be pushed to tstack instead of the
  given element.
  \param[in] predicate Predicate to find the target. Defaults to operator ==. If
  no element on the tstack makes this predicate true, nothing happens.

  \returns An iterator to the element defined by target and
  predicate. If no element fits the criteria, the returned iterator is
  equal to tstack::end().

  The first element in the string will be closest to top of the
  tstack after this operation.
  */
  template <class TS>
  iterator replace(const T& target,
                   const TS& string,
                   iterator from,
                   bool (*predicate)(const T&, const T&) = [](auto& lhs, auto& rhs) {
                     return lhs == rhs;
                   }) {
    return replace(search(target, from, predicate), string).begin;
  }
  /**
  \brief Replaces the last element matched by the given target and predicate
  with a string of elements.

  \param[in] target The reference element for the search.
  \param[in] string A string of elements to be pushed to tstack instead of the
  given element.
  \param[in] predicate Predicate to find the target. Defaults to operator ==. If
  no element on the tstack makes this predicate true, nothing happens.

  \returns An iterator to the element defined by target and
  predicate. If no element fits the criteria, the returned iterator is
  equal to tstack::end().

  The first element in the string will be closest to top of the
  tstack after this operation.
  */ // TODO fix docs
  template <class TS>
  iterator replace_last(const T& target,
                        const TS& string,
                        bool (*predicate)(const T&, const T&) = [](auto& lhs, auto& rhs) {
                          return lhs == rhs;
                        }) {
    return replace(search_last(target, predicate), string).end;
  }
  /**
  \brief Replaces the first element from the given iterator matched by the given
  target and predicate with a string of elements.

  \param[in] target The reference element for the search.
  \param[in] string A string of elements to be pushed to tstack instead of the
  given element.
  \param[in] predicate Predicate to find the target. Defaults to operator ==. If
  no element on the tstack makes this predicate true, nothing happens.

  \returns An iterator to the element defined by target and
  predicate. If no element fits the criteria, the returned iterator is
  equal to tstack::end().

  The first element in the string will be closest to top of the
  tstack after this operation.
  */
  template <class TS>
  iterator replace_last(const T& target,
                        const TS& string,
                        iterator from,
                        bool (*predicate)(const T&, const T&) = [](auto& lhs, auto& rhs) {
                          return lhs == rhs;
                        }) {
    return replace(search_last(target, from, predicate), string).end;
  }
  /**
  \brief Swaps the contents of this tstack with another tstack.

  \param[in] other The other tstack to be swapped.
  */
  void swap(tstack& other) noexcept { std::swap(_list, other._list); }

  ///@{
  /**
  \brief Returns an iterator to the top element.

  \returns an iterator to the top element.

  If the tstack is empty, the returned iterator will be equal to end().
  */
  iterator begin() noexcept { return _list.begin(); }
  const_iterator begin() const noexcept { return _list.begin(); }
  const_iterator cbegin() const noexcept { return _list.cbegin(); }
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
  iterator end() noexcept { return _list.end(); }
  const_iterator end() const noexcept { return _list.end(); }
  const_iterator cend() const noexcept { return _list.cend(); }
  ///@}

  ///@{
  /**
  \brief Returns a reverse iterator to the furthest element from the top.

  \returns A reverse iterator to the furthest element from the top.
  */
  reverse_iterator rbegin() noexcept { return _list.rbegin(); }
  const_reverse_iterator rbegin() const noexcept { return _list.rbegin(); }
  const_reverse_iterator crbegin() const noexcept { return _list.crbegin(); }
  ///@}

  ///@{
  /**
  \brief Returns a reverse iterator to the element preceding the top.

  \returns A reverse iterator to the element preceding the top.

  This element acts as a placeholder. Trying to access it results in undefined
  behavior.
  */
  reverse_iterator rend() noexcept { return _list.rend(); }
  const_reverse_iterator rend() const noexcept { return _list.rend(); }
  const_reverse_iterator crend() const noexcept { return _list.crend(); }
  ///@}

  /**
  \name Comparison operators
  \brief Lexicographic comparison of the underlying std::lists.

  \param[in] lhs Left tstack of the comparison.
  \param[in] rhs Right tstack of the comparison.

  \returns True if the lexicographic comparison of the two tstacks is true.
  */
  ///@{
  friend bool operator==(const tstack<T>& lhs, const tstack<T>& rhs) noexcept {
    return lhs._list == rhs._list;
  }
  friend bool operator!=(const tstack<T>& lhs, const tstack<T>& rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator<(const tstack<T>& lhs, const tstack<T>& rhs) noexcept {
    return lhs._list < rhs._list;
  }
  friend bool operator>(const tstack<T>& lhs, const tstack<T>& rhs) noexcept {
    return rhs._list < lhs._list;
  }
  friend bool operator<=(const tstack<T>& lhs, const tstack<T>& rhs) noexcept {
    return lhs == rhs || lhs < rhs;
  }
  friend bool operator>=(const tstack<T>& lhs, const tstack<T>& rhs) noexcept { return rhs <= lhs; }
  ///@}

 protected:
  /**
  \brief Underlying list.
  */
  std::list<T> _list;
};

/*-
ALGORITHMS
-*/

using std::sort;
using std::unique;

/*-
ADAPTERS
-*/

namespace impl {
/**
 Adapter for non-const classes to reverse their regular iterator.
 */
template <class T>
class reverser {
 public:
  reverser() = delete;
  /**
  \brief Constructs a reverser.

  \param[in] _t Container that is to be reversed.
  */
  reverser(T& _t) : ref(_t) {}

  auto begin() { return ref.rbegin(); }
  auto end() { return ref.rend(); }

  auto rbegin() { return ref.begin(); }
  auto rend() { return ref.end(); }

 private:
  /**
  \brief Reference to the reversed container.
  */
  T& ref;
};
/**
Adapter for const classes to reverse their iterator.
*/
template <class T>
class const_reverser {
 public:
  const_reverser() = delete;
  /**
  \brief Constructs a const_reverser.

  \param[in] _t Container to be reversed.
  */
  const_reverser(const T& _t) : ref(_t) {}

  auto begin() const { return ref.rbegin(); }
  auto end() const { return ref.rend(); }
  auto rbegin() const { return ref.begin(); }
  auto rend() const { return ref.end(); }

 private:
  /**
  \brief Const reference to the reversed containter.
  */
  const T& ref;
};
}  // namespace impl

/**
\brief Reverses a container.

\param[in] t Container to be reversed.

\returns Container reversing adapter.
*/
template <class T>
impl::reverser<T> reverse(T& t) {
  return impl::reverser<T>(t);
}
/**
\brief Reverses a const container.

\param[in] t Container to be reversed.

\returns Container reversing adapter.
*/
template <class T>
const impl::const_reverser<T> reverse(const T& t) {
  return impl::const_reverser<T>(t);
}

/**
\brief Constructs STL from different STL using iterators.

\param[in] it Container from which the target container is constructed.

\returns A container with all elements of the first container.

The source container must provide begin() and end() operations and the target
container must be constructible from an iterator range.
*/
template <class IT, class OT>
OT transform(const IT& it) {
  return OT{begin(it), end(it)};
}

inline constexpr int c_streq(const char* a, const char* b) {
  while (*a != '\0' && *b != '\0') {
    if (*a != *b)
      return false;
    ++a;
    ++b;
  }
  return *a == *b;
}

}  // namespace ctf

namespace std {
template <>
struct hash<ctf::bit_set> {
  size_t operator()(const ctf::bit_set& s) const noexcept { return s.hash(); }
};
}  // namespace std
#endif
/*** Enf of file generic_types.hpp ***/