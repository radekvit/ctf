#ifndef XVITRA00_TG_H
#define XVITRA00_TG_H

#include <generic_types.h>
#include <utility>

namespace bp {
class LLTable;

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
        Terminal(const string &name) : name_(name) {}
        Terminal(const string &name, const string &str)
            : name_(name), attribute_(str)
        {
        }
        ~Terminal() = default;

        const string &name() const { return name_; }

        static Terminal EOI() { return Terminal("EOI"); }

        friend bool operator<(const TranslationGrammar::Terminal &lhs,
                              const TranslationGrammar::Terminal &rhs)
        {
            return lhs.name() < rhs.name();
        }

        friend bool operator==(const TranslationGrammar::Terminal &lhs,
                               const TranslationGrammar::Terminal &rhs)
        {
            return lhs.name() == rhs.name();
        }
    };

    class Nonterminal {
        string name_;

    public:
        Nonterminal(const string &name) : name_(name) {}
        ~Nonterminal() = default;

        const string &name() const { return name_; }

        friend bool operator<(const TranslationGrammar::Nonterminal &lhs,
                              const TranslationGrammar::Nonterminal &rhs)
        {
            return lhs.name() < rhs.name();
        }

        friend bool operator==(const TranslationGrammar::Nonterminal &lhs,
                               const TranslationGrammar::Nonterminal &rhs)
        {
            return lhs.name() == rhs.name();
        }
    };

    struct Symbol {
        enum class Type {
            TERMINAL,
            NONTERMINAL,
            EPSILON,
        } type;

        Terminal terminal;
        Nonterminal nonterminal;
        Symbol(Type _type, Value _value);
        ~Symbol() = default;
    };

    class Rule {
    private:
        Symbol nonterm_;

        vector<Symbol> input_; // input of length at least 1
        vector<Symbol> output_;

    public:
        Rule(const Symbol &nonterm, const vector<Symbol> input,
             const vector<Symbol> output_);
        ~Rule() = default;
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
    const RuleMap &rules() const { return rules_; }

    LLTable create_ll_table();
};
}
#endif