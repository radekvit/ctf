#ifndef XVITRA00_GT_H
#define XVITRA00_GT_H

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace bp {

using vector = std::vector;
using string = std::string;
using Value = uint64_t;
using map = std::map;
using LetterMap = map<Value, string>;
using ReverseLetterMap = map<string, Value>;
using std::sort;

template <class T> T set_union(const T &lhs, const T &rhs)
{
    T r;
    std::set_union(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                   back_inserter(r));
    return r;
}

template <class T> bool is_in(const vector<T> &v, const T &e)
{
    auto it = std::lower_bound(v.begin(), v.end(), e);
    return it != v.end() && it->name() == e.name();
}
}

#endif