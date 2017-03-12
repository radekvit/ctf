/**
\file generic_types.h
\brief Defines types, STL classes and generic STL adapters and functions for
this project.
\author Radek Vít
 */
#ifndef CTF_GT_H
#define CTF_GT_H

#include <algorithm>
#include <functional>
#include <list>
#include <list>
#include <map>
#include <stack>
#include <string>
#include <vector>

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

/**
 \brief STL stack with added functionality. Iterators don't lose validity when
 manipulating with stack.
 */
template <class T> class tstack {
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
    std::list should be applicable.
    */
    tstack(std::initializer_list<T> ilist) : list_(ilist) {}

    /**
    \brief Returns a reference to the top element of the stack.
    */
    T &top() { return list_.front(); }
    /**
    \brief Returns a constant reference to the top element of the stack.
    */
    const T &top() const { return list_.front(); }

    /**
    \brief Returns true if there are no elements in the stack.
    */
    bool empty() const { return list_.size() == 0; }
    /**
    \brief Returns the number of elements in the stack.
    */
    size_type size() const { return list_.size(); }
    /**
    \brief Removes all elements from the stack.
    */
    void clear() noexcept { list_.clear(); }
    /**
    \brief Pushes an element to the stack.
    */
    void push(const T &t)
    { // TODO: if C++17, return reference
        list_.emplace_front(t);
    }
    /**
    \brief Pops the top element from the stack and returns it.
    */
    T pop()
    {
        T temp{list_.front()};
        list_.pop_front();
        return temp;
    }
    /**
    \brief Returns an iterator to the element defined by target and
    searchOperator. If no element fits the criteria, the returned iterator is
    equal to tstack::end().
    */
    iterator search(const T &target,
                    std::function<bool(const T &, const T &)> searchOperator =
                        [](auto lhs, auto rhs) { return lhs == rhs; })
    {
        iterator it;
        for (it = list_.begin(); it != list_.end(); ++it) {
            if (searchOperator(*it, target))
                break;
        }
        return it;
    }
    /**
    \brief Returns a constant iterator to the element defined by target and
    searchOperator. If no element fits the criteria, the returned iterator is
    equal to tstack::end().
    */
    const_iterator
    search(const T &target,
           std::function<bool(const T &, const T &)> searchOperator =
               [](auto lhs, auto rhs) { return lhs == rhs; }) const
    {
        const_iterator it;
        for (it = list_.cbegin(); it != list_.cend(); ++it) {
            if (searchOperator(*it, target))
                break;
        }
        return it;
    }
    /**
    \brief Replaces the element at the position given by it with elements in the
    string. The first element in the string will be closest to top of the stack.
    If the iterator equals tstack::end(), this does nothing.
    */
    template <class TS> iterator replace(iterator it, const TS &string)
    {
        if (it == list_.end())
            return it;
        for (auto &t : string) {
            list_.insert(it, t);
        }
        list_.erase(it++);
        return it;
    }
    /**
    \brief Replaces element defined by target and searchOperator by a string of
    elements. The first element in the string will be closest to top of the
    stack. If no such element is found, this does nothing.
    */
    template <class TS>
    iterator replace(const T &target, const TS &string,
                     std::function<bool(const T &, const T &)> searchOperator =
                         [](auto lhs, auto rhs) { return lhs == rhs; })
    {
        return replace(search(target, searchOperator), string);
    }
    /**
    \brief Swaps the contents of this tstack with another tstack.
    */
    void swap(tstack &other) { std::swap(list_, other.list_); }

    /**
    \brief Returns an iterator to the top element.
    */
    iterator begin() { return list_.begin(); }
    /**
    \brief Returns an iterator to the element following the furthest element
    from the top.
    */
    iterator end() { return list_.end(); }
    /**
    \brief Returns a constant iterator to the top element.
    */
    const_iterator begin() const { return list_.begin(); }
    /**
    \brief Returns a constant iterator to the element following the furthest
    element from the top.
    */
    const_iterator end() const { return list_.end(); }
    /**
    \brief Returns a constant iterator to the top element.
    */
    const_iterator cbegin() const { return list_.cbegin(); }
    /**
    \brief Returns a constant iterator to the element following the furthest
    element from the top.
    */
    const_iterator cend() const { return list_.cend(); }
    /**
    \brief Returns a reverse iterator to the furthest element from the top.
    */
    reverse_iterator rbegin() { return list_.rbegin(); }
    /**
    \brief Returns a reverse iterator to the element before the top element.
    */
    reverse_iterator rend() { return list_.rend(); }
    /**
    \brief Returns a constant reverse iterator to the furthest element from the
    top.
    */
    const_reverse_iterator rbegin() const { return list_.rbegin(); }
    /**
    \brief Returns a constant reverse iterator to the element before the top
    element.
    */
    const_reverse_iterator rend() const { return list_.rend(); }
    /**
    \brief Returns a constant reverse iterator to the furthest element from the
    top.
    */
    const_reverse_iterator crbegin() const { return list_.rbegin(); }
    /**
    \brief Returns a constant reverse iterator to the element before the top
    element.
    */
    const_reverse_iterator crend() const { return list_.rend(); }

    friend bool operator==(const tstack<T> &lhs, const tstack<T> &rhs)
    {
        return lhs.list_ == rhs.list_;
    }
    friend bool operator!=(const tstack<T> &lhs, const tstack<T> &rhs)
    {
        return !(lhs == rhs);
    }
    friend bool operator<(const tstack<T> &lhs, const tstack<T> &rhs)
    {
        return lhs.list_ < rhs.list_;
    }
    friend bool operator>(const tstack<T> &lhs, const tstack<T> &rhs)
    {
        return rhs.list_ < lhs.list_;
    }
    friend bool operator<=(const tstack<T> &lhs, const tstack<T> &rhs)
    {
        return lhs < rhs || lhs == rhs;
    }
    friend bool operator>=(const tstack<T> &lhs, const tstack<T> &rhs)
    {
        return lhs > rhs || lhs == rhs;
    }
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
*/
template <class T> void make_set(T &container)
{
    // sort the contents
    sort(container.begin(), container.end());
    // remove duplicates
    container.erase(unique(container.begin(), container.end()),
                    container.end());
}

/**
\brief Returns true if element e is contained in sorted container c.
*/
template <class T, class CT> bool is_in(const CT &c, const T &e)
{
    auto it = std::lower_bound(c.begin(), c.end(), e);
    return it != c.end() && it->name() == e.name();
}

/**
\brief Returns a set union of the two containers.
*/
template <class T> T set_union(const T &lhs, const T &rhs)
{
    T r;
    std::set_union(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                   back_inserter(r));
    return r;
}

/**
\brief Returns true if the set union between the two containers changed the
target container.
*/
template <class CT> bool modify_set(CT &target, const CT &addition)
{
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
template <class T> class reverser {
    T &ref;

public:
    reverser(T &_t) : ref(_t) {}

    auto begin() { return ref.rbegin(); }
    auto end() { return ref.rend(); }
    auto rbegin() { return ref.begin(); }
    auto rend() { return ref.end(); }
};
/**
Adapter for const classes to reverse their iterator.
*/
template <class T> class const_reverser {
    const T &ref;

public:
    const_reverser(const T &_t) : ref(_t) {}

    auto begin() const { return ref.rbegin(); }
    auto end() const { return ref.rend(); }
    auto rbegin() const { return ref.begin(); }
    auto rend() const { return ref.end(); }
};

/**
Reverses an STL.
*/
template <class T> reverser<T> reverse(T &t) { return reverser<T>(t); }
/**
Reverses a const STL
*/
template <class T> const const_reverser<T> reverse(const T &t)
{
    return const_reverser<T>(t);
}

/**
\brief Constructs STL from different STL using iterators.
*/
template <class T, template <class T> class IT, template <class T> class OT>
OT<T> transform(const IT<T> &it)
{
    return OT<T>{it.begin(), it.end()};
}

} // namespace ctf
#endif
/*** Enf of file generic_types.h ***/