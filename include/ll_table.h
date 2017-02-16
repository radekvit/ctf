#ifndef XVITRA00_LL_H
#define XVITRA00_LL_H

#include <base.h>
#include <generic_types.h>
#include <translation_grammar.h>

namespace bp {

class LLTable {
public:
    using col = size_t;
    using row = vector<col>;

protected:
    vector<row> rows_;

public:
    LLTable() = default;
    LLTable(const LLTable &) = default;
    LLTable(LLTable &&) = default;
    LLTable(vector<row> rows) : rows_(rows) {}
    ~LLTable() = default;
};
}

#endif