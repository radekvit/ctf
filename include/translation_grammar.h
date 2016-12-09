#ifndef XVITRA00_TG_H
#define XVITRA00_TG_H

#include <generic_types.h>
#include <utility>
#include <stdexcept>

namespace bp {
class LLTable;

class TranslationGrammar {
public:
    class NotLLException : public std::logic_error {
        using std::logic_error::logic_error;
    };

    class LLConversionException : public std::logic_error {
        using std::logic_error::logic_error;        
    };

    class Terminal {
        string name_;
        string attribute_;

    public:
        Terminal() = default;
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
        Nonterminal() = default;
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
        Symbol(Type _type) : type(_type) {}
        Symbol(Terminal _terminal) : type(Type::TERMINAL), terminal(_terminal)
        {
        }
        Symbol(Nonterminal _nonterminal)
            : type(Type::NONTERMINAL), nonterminal(_nonterminal)
        {
        }
        ~Symbol() = default;

        friend bool operator<(const Symbol &lhs, const Symbol &rhs)
        {
            if (lhs.type != rhs.type)
                return false;
            switch (lhs.type) {
            case Symbol::Type::TERMINAL:
                return lhs.terminal < rhs.terminal;
            case Symbol::Type::NONTERMINAL:
                return lhs.nonterminal < rhs.nonterminal;
            default:
                return false;
            }
        }

        friend bool operator==(const Symbol &lhs, const Symbol &rhs)
        {
            if (lhs.type != rhs.type)
                return false;
            switch (lhs.type) {
            case Symbol::Type::TERMINAL:
                return lhs.terminal == rhs.terminal;
            case Symbol::Type::NONTERMINAL:
                return lhs.nonterminal == rhs.nonterminal;
            default:
                return true;
            }
        }

        friend bool operator!=(const Symbol &lhs, const Symbol &rhs)
        {
            return !(lhs == rhs);
        }
    };

    class Rule {
    private:
        Nonterminal nonterminal_;

        vector<Symbol> input_; // input of length at least 1
        vector<Symbol> output_;

    public:

        Rule(const Nonterminal &_nonterminal, const vector<Symbol> &_input,
             const vector<Symbol> &_output)
            : nonterminal_(_nonterminal), input_(_input), output_(_output)
        {
        }
        ~Rule() = default;
        void swap_sides() { std::swap(input_, output_); }

        const Nonterminal &nonterminal() const { return nonterminal_; }
        const vector<Symbol> &input() const { return input_; }
        const vector<Symbol> &output() const { return output_; }

        friend bool operator<(const Rule &lhs, const Rule &rhs)
        {
            return lhs.nonterminal() < rhs.nonterminal() ? true
                                                 : lhs.input() < rhs.input();
        }
        friend bool operator>(const Rule &lhs, const Rule &rhs)
        {
            return rhs < lhs;
        }
        friend bool operator==(const Rule &lhs, const Rule &rhs)
        {
            return lhs.nonterminal() == rhs.nonterminal() ? lhs.input() == rhs.input()
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
    static const vector<Symbol> EPSILON_RULE_STRING;

    TranslationGrammar();
    TranslationGrammar(const vector<Terminal> &terminals,
                       const vector<Nonterminal> &nonterminals,
                       const vector<Rule> &rules, const Symbol &starting_symbol);
    ~TranslationGrammar() = default;

    void swap_sides()
    {
        for (auto &n : nonterminals_)
            for (auto &r : rules_[n]) {
                r.swap_sides();
            }
    }

    const vector<Terminal> &terminals() const {return terminals_; }
    const vector<Nonterminal> &nonterminals() const {return nonterminals_;}
    const RuleMap &rules() const { return rules_; }
    const Symbol &starting_symbol() const {return starting_symbol_;}

    LLTable create_ll_table();

    static TranslationGrammar make_LL(const TranslationGrammar &tg);
};
}
#endif