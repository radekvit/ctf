#ifndef XVITRA00_GT_H
#define XVITRA00_GT_H

#include <algorithm>
#include <functional>
#include <list>
#include <list>
#include <map>
#include <stack>
#include <string>
#include <vector>

namespace bp {

using std::vector;
using std::string;
using std::list;
using Value = uint64_t;
using std::map;
using LetterMap = map<Value, string>;
using ReverseLetterMap = map<string, Value>;
using std::sort;

template <class T> bool is_in(const vector<T> &v, const T &e)
{
    auto it = std::lower_bound(v.begin(), v.end(), e);
    return it != v.end() && it->name() == e.name();
}

template <class T> T set_union(const T &lhs, const T &rhs)
{
    T r;
    std::set_union(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                   back_inserter(r));
    return r;
}

template <class T> bool modify_set(vector<T> &target, const vector<T> &addition)
{
    auto before = target.size();
    target = set_union(target, addition);
    return before != target.size();
}

template <class T> class reverser {
    T &ref;

public:
    reverser(T &_t) : ref(_t) {}

    auto begin() { return ref.rbegin(); }
    auto end() { return ref.rend(); }
};

template <class T> class const_reverser {
    const T &ref;

public:
    const_reverser(const T &_t) : ref(_t) {}

    auto begin() { return ref.rbegin(); }
    auto end() { return ref.rend(); }
};

template <class T> reverser<T> reverse(T &t) { return reverser<T>(t); }

template <class T> const_reverser<T> reverse(const T &t)
{
    return const_reverser<T>(t);
}

template <class T> class tstack {
protected:
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

    tstack() = default;
    tstack(std::initializer_list<T> ilist) : list_(ilist) {}

    T &top() { return list_.front(); }
    const T &top() const { return list_.front(); }

    bool empty() const { return list_.size() == 0; }

    size_type size() const { return list_.size(); }

    void clear() noexcept { list_.clear(); }

    void push(const T &t)
    { // TODO: if C++17, return reference
        list_.emplace_front(t);
    }

    T pop()
    {
        T temp{list_.front()};
        list_.pop_front();
        return temp;
    }

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

    void replace(iterator it, const vector<T> string)
    {
        if (it == list_.end())
            return;
        for (auto &t : string) {
            list_.insert(it, t);
        }
        list_.erase(it);
    }

    void replace(const T &target, const vector<T> &string,
                 std::function<bool(const T &, const T &)> searchOperator =
                     [](auto lhs, auto rhs) { return lhs == rhs; })
    {
        replace(search(target, searchOperator), string);
    }

    void swap(tstack &other) { std::swap(list_, other.list_); }

    iterator begin() { return list_.begin(); }
    iterator end() { return list_.end(); }
    const_iterator begin() const { return list_.begin(); }
    const_iterator end() const { return list_.end(); }
    const_iterator cbegin() const { return list_.begin(); }
    const_iterator cend() const { return list_.end(); }
    reverse_iterator rbegin() { return list_.rbegin(); }
    reverse_iterator rend() { return list_.rend(); }
    const_reverse_iterator rbegin() const { return list_.rbegin(); }
    const_reverse_iterator rend() const { return list_.rend(); }
    const_reverse_iterator crbegin() const { return list_.rbegin(); }
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
}
#endif