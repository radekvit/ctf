#include <algorithm>
#include <ll_table.h>
#include <stdexcept>
#include <translation_grammar.h>
namespace bp {

const TranslationGrammar::Symbol TranslationGrammar::Symbol::EPSILON{Type::EPSILON};

void TranslationGrammar::Rule::check_nonterminals()
{
    vector<Nonterminal> inputNonterminals;
    vector<Nonterminal> outputNonterminals;
    for(auto &s: input_)
    {
        if(s.type == Symbol::Type::NONTERMINAL)
            inputNonterminals.push_back(s.nonterminal);
    }
    for(auto &s: output_)
    {
        if(s.type == Symbol::Type::NONTERMINAL)
            outputNonterminals.push_back(s.nonterminal);
    }
    if(inputNonterminals != outputNonterminals)
        throw std::invalid_argument("Input and output nonterminals must match.");
}

const vector<TranslationGrammar::Symbol>
    TranslationGrammar::EPSILON_RULE_STRING{Symbol(Symbol::Type::EPSILON)};

TranslationGrammar::TranslationGrammar(const vector<Terminal> &terminals,
                                       const vector<Nonterminal> &nonterminals,
                                       const vector<Rule> &_rules,
                                       const Symbol &starting_symbol)
    : terminals_(terminals), nonterminals_(nonterminals),
      starting_symbol_(starting_symbol)
{
    sort(terminals_.begin(), terminals_.end());
    sort(nonterminals_.begin(), nonterminals_.end());
    vector<Rule> nrules(_rules);
    sort(nrules.begin(), nrules.end());
    if (starting_symbol_.type != Symbol::Type::NONTERMINAL)
        throw std::invalid_argument("Starting symbol must be a nonterminal");
    for (auto &r : nrules) {
        if (!is_in(nonterminals_, r.nonterminal()))
            throw std::invalid_argument("No nonterminal exists" +
                                        r.nonterminal().name());
        rules_[r.nonterminal()].push_back(r);
    }
}

    TranslationGrammar::TranslationGrammar(const vector<Terminal> &terminals,
                       const vector<Nonterminal> &nonterminals,
                       const RuleMap &rules,
                       const Symbol &starting_symbol):
        terminals_(terminals), nonterminals_(nonterminals), rules_(rules), starting_symbol_(starting_symbol)
{
    sort(terminals_.begin(), terminals_.end());
    sort(nonterminals_.begin(), nonterminals_.end());
    if (starting_symbol_.type != Symbol::Type::NONTERMINAL)
        throw std::invalid_argument("Starting symbol must be a nonterminal");
    for (auto &rv : rules_) {
        for(auto &r: rv.second)
        {
            if (!is_in(nonterminals_, r.nonterminal()))
                throw std::invalid_argument("No nonterminal exists" +
                                            r.nonterminal().name());
        }
        sort(rv.second.begin(), rv.second.end()); 
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
                if (r.input()[0].type != Symbol::Type::NONTERMINAL ||
                    !empty[nonterm_index(r.input()[0].nonterminal)])
                    continue;
                for (auto nit = r.input().begin() + 1;
                     nit != r.input().end() &&
                     nit->type == Symbol::Type::NONTERMINAL &&
                     empty[nonterm_index(nit->nonterminal)];
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
                if (modify_set(follow[i],
                               follow[nonterm_index(r.nonterminal())]))
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
            vector<Terminal> rfollow = follow[nonterm_index(r.nonterminal())];
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
                    if (row.back() != invalid_value)
                        throw NotLLException("Multiple rules found for a "
                                             "single cell of LL table.");
                    row.back() = i;
                }
            }
        }
        offset += rules_[n].size();
    }
    return LLTable(rows);
}

TranslationGrammar::Nonterminal create_new_nonterminal(const vector<TranslationGrammar::Nonterminal> &nonterminals,
                                   const string &base, const char suffix)
{
    string name = base;
    do {
        name += suffix;
    } while (is_in(nonterminals, TranslationGrammar::Nonterminal(name)));

    return TranslationGrammar::Nonterminal(name);
}

TranslationGrammar
TranslationGrammar::remove_left_recursion(const TranslationGrammar &tg)
{
    static const char SUFFIX = '\'';
    const vector<Terminal> &terminals_ = tg.terminals();
    const vector<Nonterminal> &nonterminals_ = tg.nonterminals();
    const RuleMap &rules_ = tg.rules();
    const Symbol &starting_symbol_ = tg.starting_symbol();
    vector<Nonterminal> newNonterminals(nonterminals_);
    RuleMap noLrRules;
    // TODO implement
    for (auto &n : nonterminals_) {
        bool leftRecursion = false;
        size_t recursionIndex = 0;
        for (auto it = rules_.at(n).begin();
             it != rules_.at(n).end(); ++it) {
            auto &r = *it;                              // get reference to rule
            size_t i = it - rules_.at(n).begin(); // get index of rule
            // rule with left recursion found
            if (r.nonterminal() == r.input()[0]) {
                leftRecursion = true;
                recursionIndex = i;
                // only one such rule is anticipated; does not work with more
                break;
            }
        }
        // remove left recursion by appending new nonterminal to all other rules
        // and creating right recursion in place
        if (leftRecursion) {
            // rules for old nonterminal
            vector<Rule> inPlaceRules(rules_.at(n));
            // rules for new terminal
            vector<Rule> newNonterminalRules{rules_.at(n)[recursionIndex]};
            // remove left recursive rule from inPlaceRules
            inPlaceRules.erase(inPlaceRules.begin() + recursionIndex);

            Nonterminal newNonterminal = create_new_nonterminal(newNonterminals, n.name(), SUFFIX);
            Symbol newNonterminalSymbol(newNonterminal);
            newNonterminals.push_back(newNonterminal);

            // fix rule in newNonterminalRules
            vector<Symbol> input(newNonterminalRules[0].input());
            vector<Symbol> output(newNonterminalRules[0].output());
            if (output[0] != input[0])
                throw LLConversionException("This grammar cannot be converted.");
            input.erase(input.begin());             // remove nonterminal
            input.push_back(newNonterminalSymbol);  // append new nonterminal
            output.erase(output.begin());           // remove origin nonterminal
            output.push_back(newNonterminalSymbol); // append new nonterminal
            newNonterminalRules[0] = Rule(newNonterminal, input, output);
            // add epsilon rule to newNonterminalRules
            newNonterminalRules.push_back(
                Rule(newNonterminal, EPSILON_RULE_STRING,
                     EPSILON_RULE_STRING)); // add rule to epsilon

            for (auto &r : inPlaceRules) // append new nonterminal
            {
                vector<Symbol> input(r.input());
                input.push_back(newNonterminalSymbol);
                vector<Symbol> output(r.output());
                output.push_back(newNonterminalSymbol);
                Rule tmp(r.nonterminal(), input, output);
                std::swap(r, tmp);
            }
            noLrRules[inPlaceRules[0].nonterminal()] = inPlaceRules;
            noLrRules[newNonterminal] = newNonterminalRules;
        } else {
            noLrRules[n] = rules_.at(n);
        }
    }
    return TranslationGrammar(terminals_, newNonterminals, noLrRules,
                              starting_symbol_);
}

TranslationGrammar TranslationGrammar::factorize(const TranslationGrammar &tg)
{
    return tg;
}

// tries to transform this translation grammar to ll translation grammar
TranslationGrammar TranslationGrammar::make_LL(const TranslationGrammar &tg)
{
    // factorization
    TranslationGrammar factorized = TranslationGrammar::factorize(tg);
    // left recursion removal
    return TranslationGrammar::remove_left_recursion(factorized);
}

void TranslationGrammar::print(std::ostream &o)
{
    o << "(\n\t{";
    for(auto it = nonterminals_.begin(); it != nonterminals_.end(); ++it)
    {
        o  << it->name();
        if(it != nonterminals_.end() - 1)
            o << ", ";
    }
    o << "},\n\t{";
    for(auto it = terminals_.begin(); it != terminals_.end(); ++it)
    {
        o << it->name();
        if(it != terminals_.end() - 1)
            o << ", ";
    }
    o << "},\n\t{\n";
    bool first = true;
    for (auto &n:nonterminals_)
    {

        for(auto it = rules_[n].begin(); it != rules_[n].end(); ++it)
        {
            if(!first)
            {
                o << ",";
                o << "\n";
            }
            o << "\t\t" << it->nonterminal().name() << "\t=>\t(";
            for(auto &s: it->input())
            {
                s.print(o);
            }
            o << ", ";
            for(auto &s: it->output())
            {
                s.print(o);
            }
            o << ")";
            first = false;
        }
    }
    o << "\n\t},\n\t" << starting_symbol_.nonterminal.name() << "\n)\n";
    o.flush();
}

}