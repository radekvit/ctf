#include <algorithm>
#include <ll_table.h>
#include <stdexcept>
#include <translation_grammar.h>
namespace bp {

TranslationGrammar::TranslationGrammar(const vector<Terminal> &terminals,
                                       const vector<Nonterminal> &nonterminals,
                                       vector<Rule> &rules,
                                       const Symbol &starting_symbol)
    : terminals_(terminals), nonterminals_(nonterminals),
      starting_symbol_(starting_symbol)
{
    sort(terminals_.begin(), terminals_.end());
    sort(nonterminals_.begin(), nonterminals_.end());
    sort(rules.begin(), rules.end());
    if (starting_symbol_.type != Symbol::Type::NONTERMINAL)
        throw std::invalid_argument("Starting symbol must be a nonterminal");
    for (auto &r : rules) {
        if (!is_in(nonterminals_, r.nonterm()))
            throw std::invalid_argument("No nonterminal exists" +
                                        r.nonterm().name());
        rules_[r.nonterm()].push_back(r);
    }
}

size_t TranslationGrammar::nonterm_index(const Nonterminal &nt)
{
    return std::lower_bound(nonterminals_.begin(), nonterminals_.end(), nt) -
           nonterminals_.begin();
}

LLTable TranslationGrammar::create_ll_table()
{
    vector<bool> empty;
    vector<vector<Terminal>> first;
    vector<vector<Terminal>> follow;
    vector<vector<Terminal>> predict;

    create_empty(empty);
    create_first(empty, first);
    create_follow(empty, first, follow);
    create_predict(empty, first, follow, predict);
    return create_ll(predict);
}

void TranslationGrammar::create_empty(vector<bool> &empty)
{
    for (auto &n : nonterminals_) {
        bool isempty = false;
        for (auto &r : rules_[n]) {
            if (r.input()[0].type == Symbol::Type::EPSILON) {
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
                        if (empty[nonterm_index(s.nonterminal)] == false)
                            isempty = false;
                        break;
                    case Symbol::Type::TERMINAL:
                        isempty = false;
                        break;
                    case Symbol::Type::EPSILON:
                        break;
                    default:
                        break; // not defined
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
        (void)n;
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
                if (modify_first(rfirst, r.input()[0], first))
                    changed = true;
                if(r.input()[0].type != Symbol::Type::NONTERMINAL || !empty[nonterm_index(r.input()[0].nonterminal)])
                    continue;
                for (auto nit = r.input().begin() + 1;
                     nit != r.input().end() &&
                     nit->type == Symbol::Type::NONTERMINAL &&
                     empty[nonterm_index(
                         nit->nonterminal)];
                     ++nit) {
                    if (modify_first(rfirst, *nit, first))
                        changed = true;
                }
            }
        }
    } while (changed);
}

bool TranslationGrammar::modify_first(vector<Terminal> &target,
                                      const Symbol &symbol,
                                      const vector<vector<Terminal>> &first)
{
    switch (symbol.type) {
    case Symbol::Type::TERMINAL:
        if (!is_in(target, symbol.terminal)) {
            target.push_back(symbol.terminal);
            sort(target.begin(), target.end());
            return true;
        }
        break;
    case Symbol::Type::NONTERMINAL:
        if (modify_set(target, first[nonterm_index(symbol.nonterminal)]))
            return true;
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
        (void)n;
        follow.push_back(vector<Terminal>());
    }
    follow[nonterm_index(starting_symbol_.nonterminal)].push_back(
        Terminal::EOI());

    bool changed;
    do {
        changed = false;
        for (auto &n : nonterminals_) {
            for (auto &r : rules_[n]) {
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
            size_t i = nonterm_index(it->nonterminal);
            bool isepsilon = true;
            vector<Terminal> groupfirst;
            for (auto it2 = it + 1; it2 != r.input().end(); ++it2) {
                size_t i2 = 0;
                if (it2->type == Symbol::Type::NONTERMINAL) {
                    i2 = nonterm_index(it2->nonterminal);
                    groupfirst = set_union(groupfirst, first[i2]); // set first
                } else if (it2->type == Symbol::Type::TERMINAL) {
                    if (!is_in(groupfirst, it2->terminal)) {
                        groupfirst.push_back(it2->terminal);
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
                if (modify_set(follow[i], follow[nonterm_index(r.nonterm())]))
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

void TranslationGrammar::create_predict(const vector<bool> &empty,
                                        const vector<vector<Terminal>> &first,
                                        const vector<vector<Terminal>> &follow,
                                        vector<vector<Terminal>> &predict)
{
    for (auto &n : nonterminals_) {
        for (auto &r : rules_[n]) {
            vector<Terminal> groupfirst;
            vector<Terminal> rfollow = follow[nonterm_index(r.nonterm())];
            bool isEmpty = true;
            for (auto &s : r.input()) {
                modify_first(groupfirst, s, first);
                if (s.type == Symbol::Type::TERMINAL) {
                    isEmpty = false;
                    break;
                } else if (s.type == Symbol::Type::NONTERMINAL) {
                    if (empty[nonterm_index(s.nonterminal)] == false) {
                        isEmpty = false;
                        break;
                    }
                }
            }
            predict.push_back(groupfirst);

            if (isEmpty) {
                predict.back() = set_union(predict.back(), rfollow);
            }
        }
    }
}

LLTable TranslationGrammar::create_ll(const vector<vector<Terminal>> &predict)
{
    vector<LLTable::row> rows;
    size_t offset = 0;
    for (auto &n : nonterminals_) {
        const LLTable::col invalid_value = rules_[n].size();
        rows.push_back(LLTable::row());
        LLTable::row &row = rows.back();
        for (auto &t : terminals_) {
            row.push_back(invalid_value);
            for (auto it = rules_[n].begin(); it != rules_[n].end(); ++it) {
                size_t i = it - rules_[n].begin();

                if (is_in(predict[i + offset], t)) {
                    row.back() = i;
                    break; // assuming no other rule has it
                }
            }
        }
        offset += rules_[n].size();
    }
    return LLTable(rows);
}
}