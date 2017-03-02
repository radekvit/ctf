#ifndef XVITRA00_TG_H
#define XVITRA00_TG_H

#include <base.h>
#include <functional>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace bp {
class LLTable;

struct TranslationGrammar {
    class Rule {
    public:
        using attribute_function =
            std::function<void(const vector<Symbol> &, vector<Symbol>)>;

    protected:
        Nonterminal nonterminal_;

        vector<Symbol> input_;
        vector<Symbol> output_;

        attribute_function attributeSetter_;

        // checks if nonterminals are in same space
        void check_nonterminals();

    public:
        Rule(const Nonterminal &_nonterminal, const vector<Symbol> &_input,
             const vector<Symbol> &_output,
             attribute_function attributeSetter = attribute_function())
            : nonterminal_(_nonterminal), input_(_input), output_(_output),
              attributeSetter_(attributeSetter)
        {
            check_nonterminals();
        }
        Rule(const Nonterminal &_nonterminal, const vector<Symbol> &_both)
            : Rule(_nonterminal, _both, _both, [](auto input, auto output) {
                  for (size_t i = 0; i < input.size(); ++i) {
                      output[i].terminal.attribute() =
                          input[i].terminal.attribute();
                  }
                  return;
              })
        {
        }
        ~Rule() = default;
        void swap_sides() { std::swap(input_, output_); }

        Nonterminal &nonterminal() { return nonterminal_; }
        const Nonterminal &nonterminal() const { return nonterminal_; }
        vector<Symbol> &input() { return input_; }
        const vector<Symbol> &input() const { return input_; }
        vector<Symbol> &output() { return output_; }
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
                       ? lhs.input() == rhs.input() &&
                             lhs.output() == rhs.output()
                       : false;
        }
        friend bool operator!=(const Rule &lhs, const Rule &rhs)
        {
            return !(lhs == rhs);
        }
    };

    using RuleMap = map<Nonterminal, vector<Rule>>;

protected:
    vector<Terminal> terminals_;
    vector<Nonterminal> nonterminals_;
    vector<Rule> rules_;
    Symbol starting_symbol_;

public:
    TranslationGrammar();
    TranslationGrammar(const vector<Terminal> &terminals,
                       const vector<Nonterminal> &nonterminals,
                       const vector<Rule> &rules,
                       const Symbol &starting_symbol);
    ~TranslationGrammar() = default;

    static const vector<Symbol> EPSILON_STRING;

    void swap_sides()
    {
        for (auto &r : rules_) {
            r.swap_sides();
        }
    }

    vector<Terminal> &terminals() { return terminals_; }
    const vector<Terminal> &terminals() const { return terminals_; }
    vector<Nonterminal> &nonterminals() { return nonterminals_; }
    const vector<Nonterminal> &nonterminals() const { return nonterminals_; }
    vector<Rule> &rules() { return rules_; }
    const vector<Rule> &rules() const { return rules_; }
    Symbol &starting_symbol() { return starting_symbol_; }
    const Symbol &starting_symbol() const { return starting_symbol_; }

    size_t nonterminal_index(const Nonterminal &nt) const;
    size_t terminal_index(const Terminal &t) const;
};
}
#endif