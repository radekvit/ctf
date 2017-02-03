#ifndef XVITRA00_TG_H
#define XVITRA00_TG_H

#include <base.h>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace bp {
class LLTable;

class TranslationGrammar {
public:
    class Rule {
    private:
        Nonterminal nonterminal_;

        vector<Symbol> input_; // input of length at least 1
        vector<Symbol> output_;

        // checks if nonterminals are in same space
        void check_nonterminals();

    public:
        Rule(const Nonterminal &_nonterminal, const vector<Symbol> &_input,
             const vector<Symbol> &_output)
            : nonterminal_(_nonterminal), input_(_input), output_(_output)
        {
            if (input_.size() == 0)
                input_.push_back(Symbol::EPSILON);
            if (output_.size() == 0)
                output_.push_back(Symbol::EPSILON);
            check_nonterminals();
        }
        Rule(const Nonterminal &_nonterminal, const vector<Symbol> &_both)
            : Rule(_nonterminal, _both, _both)
        {
        }
        ~Rule() = default;
        void swap_sides() { std::swap(input_, output_); }

        const Nonterminal &nonterminal() const { return nonterminal_; }
        const vector<Symbol> &input() const { return input_; }
        const vector<Symbol> &output() const { return output_; }

        friend bool operator<(const Rule &lhs, const Rule &rhs)
        {
            return lhs.nonterminal() < rhs.nonterminal()
                       ? true
                       : lhs.input() < rhs.input();
        }
        friend bool operator>(const Rule &lhs, const Rule &rhs)
        {
            return rhs < lhs;
        }
        friend bool operator==(const Rule &lhs, const Rule &rhs)
        {
            return lhs.nonterminal() == rhs.nonterminal()
                       ? lhs.input() == rhs.input()
                       : false;
        }
        friend bool operator!=(const Rule &lhs, const Rule &rhs)
        {
            return !(lhs == rhs);
        }
    };

    using RuleMap = map<Nonterminal, vector<Rule>>;

private:
    vector<Terminal> terminals_;
    vector<Nonterminal> nonterminals_;
    RuleMap rules_;

    Symbol starting_symbol_;

    TranslationGrammar(const vector<Terminal> &terminals,
                       const vector<Nonterminal> &nonterminals,
                       const RuleMap &rules, const Symbol &starting_symbol);

    size_t nonterm_index(const Nonterminal &nt);
    void create_empty(vector<bool> &empty);
    void create_first(const vector<bool> &empty,
                      vector<vector<Terminal>> &first);
    bool modify_first(vector<Terminal> &target, const Symbol &symbol,
                      const vector<vector<Terminal>> &first);
    bool modify_set(vector<Terminal> &target, const vector<Terminal> &addition);
    void create_follow(const vector<bool> &empty,
                       const vector<vector<Terminal>> &first,
                       vector<vector<Terminal>> &follow);
    bool rule_follow(const Rule &r, const vector<bool> &empty,
                     const vector<vector<Terminal>> &first,
                     vector<vector<Terminal>> &follow);
    void create_predict(const vector<bool> &empty,
                        const vector<vector<Terminal>> &first,
                        const vector<vector<Terminal>> &follow,
                        vector<vector<Terminal>> &predict);
    LLTable create_ll(const vector<vector<Terminal>> &predict);

    static TranslationGrammar factorize(const TranslationGrammar &);
    static TranslationGrammar remove_left_recursion(const TranslationGrammar &);

public:
    static const vector<Symbol> EPSILON_RULE_STRING;

    TranslationGrammar();
    TranslationGrammar(const vector<Terminal> &terminals,
                       const vector<Nonterminal> &nonterminals,
                       const vector<Rule> &rules,
                       const Symbol &starting_symbol);
    ~TranslationGrammar() = default;

    void swap_sides()
    {
        for (auto &n : nonterminals_)
            for (auto &r : rules_[n]) {
                r.swap_sides();
            }
    }

    const vector<Terminal> &terminals() const { return terminals_; }
    const vector<Nonterminal> &nonterminals() const { return nonterminals_; }
    const RuleMap &rules() const { return rules_; }
    const Symbol &starting_symbol() const { return starting_symbol_; }

    LLTable create_ll_table();

    static TranslationGrammar make_LL(const TranslationGrammar &);

    void print(std::ostream &o);
};
}
#endif