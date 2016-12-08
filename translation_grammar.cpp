#include <algorithm>
#include <translation_grammar.h>
namespace bp {

size_t TranslationGrammar::nonterm_index(const Nonterminal &nt)
{
    return std::lower_bound(nonterminals_.begin(), nonterminals_.end(), nt) -
           nonterminals_.begin();
}

constexpr void TranslationGrammar::create_ll_table()
{
    vector<bool> empty;
    vector<vector<Terminal>> first;
    vector<vector<Terminal>> follow;
    vector<vector<Terminal>> predict;

    create_empty(empty);
    create_first(empty, first);
    create_follow(empty, first, follow);
    create_predict(empty, first, follow);
    return create_ll(predict);
}

void TranslationGrammar::create_empty(vector<bool> &empty)
{
    for (auto &n : nonterminals_) {
        bool isempty = false;
        for (auto &r : rules_[n]) {
            if (r.input()[0].type == Rule::type::EPSILON) {
                isempty = true;
                break;
            }
        }
        empty.push_back(isempty);
    }

    bool changed;
    do {
        changed = false;
        for (auto it = nonterminals_.begin(); it != nonterminals_.end(); ++it) {
            auto &n = *it;
            size_t i = it - nonterminals_.begin();
            if (empty[i])
                continue;
            for (auto &r : rules_[n]) {
                bool isempty = true;
                for (auto &s : r.input()) {
                    switch (s.type) {
                    case Symbol::Type::NONTERMINAL:
                        if (empty[nonterm_index(s.value.Nonterminal)] == false)
                            isempty = false;
                        break;
                    case Symbol::Type::TERMINAL:
                        isempty = false;
                        break;
                    case Symbol::Type::EPSILON:
                        break;
                    default:
                        // not defined
                    }
                }
                if (isempty) {
                    empty[i] = true;
                    changed = true;
                }
            }
        }
    } while (changed);
}

void TranslationGrammar::create_first(const vector<bool> &empty,
                                      vector<vector<Terminal>> &first)
{
    for (auto &n : nonterminals_) {
        first.push_back(vector<Terminal>());
    }
    bool changed;
    do {
        changed = false;
        for (auto it = nonterminals_.begin(); it != nonterminals_.end(); ++it) {
            auto &n = *it;
            size_t i = it - nonterminals_.begin();
            auto &rfirst = first[i];
            for (auto &r : rules_[n]) {
                if (modify_first(rfirst, r.input()[0]))
                    changed = true;
                for (auto nit = r.input().begin() + 1;
                     it != r.input().end() && empty[nonterm_index(*nit)];
                     ++it) {
                    if (modify_first(rfirst, *nit))
                        changed = true;
                }
            }
        }
    } while (changed);
}

bool TranslationGrammar::modify_first(vector<Terminal> &target,
                                      const Symbol &symbol)
{
    switch (symbol.type) {
    case Symbol::Type::TERMINAL:
        if (!is_in(target, symbol.value.terminal)) {
            target.push_back(symbol.value.terminal);
            sort(target.begin(), target.end());
            return true;
        }
        break;
    case Symbol::Type::NONTERMINAL:
        if (modify_set(target, first[nonterm_index(symbol.value.nonterminal)]))
            return true;
    }
    break;
default:
    break;
}
return false;
}

bool TranslationGrammar::modify_set(vector<Terminal> &target,
                                    const vector<Terminal> &addition)
{
    auto before = target.size();
    target = set_union(target, addition);
    return before != target.size();
}

void TranslationGrammar::create_follow(const vector<bool> &empty,
                                       const vector<vector<Terminal>> &first,
                                       vector<vector<Terminal>> &follow)
{
    for (auto &n : nonterminals_) {
        follow.push_back(vector<Terminal>());
    }
    follow[nonterm_index(starting_symbol_.value.nonterminal)].push_back(
        Terminal::EOI());

    bool changed;
    do {
        changed = false;
        for (auto &n : nonterminals_) {
            for (auto &r : rules_) {
                if (rule_follow(r, empty, first, follow))
                    changed = true;
            }
        }
    } while (changed);
}

bool TranslationGrammar::rule_follow(const Rule &r, const vector<bool> &empty,
                                     const vector<vector<Terminal>> &first,
                                     vector<vector<Terminal>> &follow)
{
    bool changed = false;
    for (auto it = r.input().begin(); it < r.input().end(); ++it) {
        if (it->type == Symbol::Type::NONTERMINAL) {
            size_t i = nonterm_index(*it);
            bool isepsilon = true;
            vector<Terminal> groupfirst;
            for (auto it2 = it + 1; it != r.input().end(); ++it2) {
                size_t i2 = 0;
                if (it2->type == Symbol::Type::NONTERMINAL) {
                    i2 = nonterm_index(it2->value.nonterminal);
                    groupfirst = set_union(groupfirst, first[i2]); // set first
                } else if (it2->type == Symbol::Type::TERMINAL) {
                    if (!is_in(groupfirst, it2->value.terminal)) {
                        groupfirst.push_back(it2->value.terminal);
                        sort(groupfirst.begin(), groupfirst.end());
                    }
                }

                if (it2->type == Symbol::Type::TERMINAL ||
                    (it2->type == Symbol::Type::NONTERMINAL &&
                     empty[i2] == false)) {
                    isepsilon = false;
                }
            }
            if (isepsilon) {
                if(modify_set(follow[i], follow[nonterm_index(r.nonterm().value.nonterminal]))
                    changed = true;
            }
            if (it != r.input().end() - 1) // at least one symbol
            {
                if (modify_set(follow[i], groupfirst))
                    changed = true;
            }
        }
    }
    return changed;
}

void create_predict(vector<vector<Terminal>> &predict)
{
    for (auto &n : nonterminals_) {
        for (auto &r : rules_[n]) {
            vector<Terminal> groupfirst;
            vector<Terminal> rfollow =
                follow[nonterm_index(r.nonterm().value.nonterminal)];
            bool isEmpty = true;
            for (auto &s : r.input()) {
                modify_first(groupfirst, s);
                if (s.type == Symbol::Type::TERMINAL) {
                    isEmpty = false;
                    break;
                } else if (s.type == Symbol::Type::NONTERMINAL) {
                    if (empty[nonterm_index(s.value.nonterminal)] == false) {
                        isEmpty = false;
                        break;
                    }
                }
            }
            predict.push_back(groupfirst);

            if (isEmpty) {
                predict.back() = set_intersection(predict.back(), rfollow);
            }
        }
    }
}

LLTable create_ll(const vector<bool> &empty,
                  const vector<vector<Terminal>> &first,
                  const vector<vector<Terminal>> &follow)
{
    vector<LLTable::row> rows;
    size_t offset = 0;
    for (auto it = nonterminals_.begin(); it != nonterminals_.end(); ++it) {
        size_t i = it - nonterminals_begin();
        auto &n = *it;
        const LLTable::col invalid_value = rules_[n].size();
        rows.push_back(LLTable::row());
        LLTable::row &row = rows.back();
        for (auto &t : terminals_) {
            row.push_back(invalid_value);
            for (auto it2 = rules_[n].begin(); it2 != rules_[n].end(); ++it) {
                size_t i2 = it2 = rules_[n].begin();
                auto &r = *it2;
                if (is_in(predict[i2 + offset], t)) {
                    row.back() = i2;
                    break; // assuming no other rule has it
                }
            }
        }
        offset += rules_[n].size();
    }
    return LLTable(rows);
}
}