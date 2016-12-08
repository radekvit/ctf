#ifndef XVITRA00_TG_H
#define XVITRA00_TG_H

#include <generic_types.h>
#include <utility>

namespace tg {

class TranslationGrammar {
public:
    class LookupFailure {
        string msg_;

    public:
        string what() { return msg_; }
    };

    class Terminal {
        string name_;
        string attribute_;

    public:
        Terminal(string &name, string &str = "") : name_(name), attribute_(str)
        {
        }

        const string &name() const { return name_; }

        bool operator<(const Terminal &lhs, const Terminal &rhs)
        {
            return lhs.name() < rhs.name();
        }

        bool operator==(const Terminal &lhs, const Terminal &rhs)
        {
            return lhs.name() == rhs.name();
        }

        static Terminal EOI() { return Terminal("EOI"); }
    };

    class Nonterminal {
        string name_;

    public:
        Nonterminal(string &name_) : name_(name) {}

        const string &name() const { return name_; }

        bool operator<(const Nonterminal &lhs, const Nonterminal &rhs)
        {
            return lhs.name() < rhs.name();
        }

        bool operator==(const Nonterminal &lhs, const Nonterminal &rhs)
        {
            return lhs.name() == rhs.name();
        }
    };

    struct Symbol {
        enum class Type {
            TERMINAL,
            NONTERMINAL,
            EPSILON,
            OPENING_BRACKET, // allow precedence analysis
        } type;
        union {
            Terminal terminal;
            Nonterminal nonterminal;
        } value;
    };

    class Rule {
    private:
        Symbol nonterm_;

        vector<Symbol> input_; // input of length at least 1
        vector<Symbol> output_;

    public:
        void swap_sides() { std::swap(input_, output_); }

        const Symbol &nonterm() const { return nonterm_; }
        const vector<Symbol> &input() const { return input_; }
        const vector<Symbol> &output() const { return output_; }
    };

    using RuleMap = map<Nonterminal, vector<Rule>>;

private:
    LetterMap mapper_;
    ReverseLetterMap revmapper_;

    vector<Terminal> terminals_;
    vector<Nonterminal> nonterminals_;
    RuleMap rules_;

    Symbol starting_symbol_;

public:
    TranslationGrammar();
    ~TranslationGrammar();

    void swap_sides()
    {
        for (auto &n : nonterminals_)
            for (auto &r : rules_[n]) {
                r.swap_sides();
            }
    }
    string map_value(Value v);
    const &vector<Rule> rules() const { return rules_; }

    const Rule &
};
}
#endif