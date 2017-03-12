#ifndef CTF_TG_H
#define CTF_TG_H

#include <base.h>
#include <functional>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace ctf {
class LLTable;

struct TranslationGrammar {
  class Rule {
   protected:
    Nonterminal nonterminal_;

    vector<Symbol> input_;
    vector<Symbol> output_;

    /**
    \brief Attribute copying from input to output in each rule. Implicitly
    created to copy in-order.
     */
    vector<vector<size_t>> attributeTargets;

    /**
    \brief Checks if nonterminals are in same space in input and output strings.
    */
    void check_nonterminals();
    /**
    \brief Counts input nonterminals.
    */
    size_t count_input_terminals() const {
      size_t count = 0;
      for (auto &s : input_) {
        if (s.type == Symbol::Type::TERMINAL) count++;
      }
      return count;
    }
    /**
    \brief Creates implicit targets for terminal attributes.
    */
    void create_implicit_targets() {
      attributeTargets =
          vector<vector<size_t>>(count_input_terminals(), vector<size_t>());
      auto oit = output_.begin();
      while (oit != output_.end() && oit->type != Symbol::Type::TERMINAL) {
        ++oit;
      }
      if (oit == output_.end()) return;
      int i = 0;
      for (auto it = input_.begin(); it < input_.end(); ++it) {
        if (it->type != Symbol::Type::TERMINAL) continue;
        attributeTargets[i].push_back(oit - output_.begin());
        ++i;
        while (oit != output_.end() && oit->type != Symbol::Type::TERMINAL) {
          ++oit;
        }
        if (oit == output_.end()) return;
      }
    }

   public:
    /**
    \brief Constructs a rule with implicit attribute targets.
    */
    Rule(const Nonterminal &_nonterminal, const vector<Symbol> &_input,
         const vector<Symbol> &_output
         )
        : nonterminal_(_nonterminal),
          input_(_input),
          output_(_output),
          attributeTargets(vector<vector<size_t>>()) {
      check_nonterminals();
      create_implicit_targets();
    }
    /**
    \brief Constructs a rule.
    */
    Rule(const Nonterminal &_nonterminal, const vector<Symbol> &_input,
         const vector<Symbol> &_output,
         const vector<vector<size_t>> &_attributeTargets)
        : nonterminal_(_nonterminal),
          input_(_input),
          output_(_output),
          attributeTargets(_attributeTargets) {
      check_nonterminals();
      //TODO better targets checks
      if (attributeTargets.size() != count_input_terminals())
        throw std::invalid_argument(
            "Invalid attributeTargets when "
            "constructing class "
            "TranslationGrammar::Rule.");
    }
    /**
    \brief Constructs a rule with same input and output. Attribute targets are implicit.
    */
    Rule(const Nonterminal &_nonterminal, const vector<Symbol> &_both)
        : Rule(_nonterminal, _both, _both) {}
    ~Rule() = default;
    void swap_sides() { std::swap(input_, output_); }

    Nonterminal &nonterminal() { return nonterminal_; }
    const Nonterminal &nonterminal() const { return nonterminal_; }
    vector<Symbol> &input() { return input_; }
    const vector<Symbol> &input() const { return input_; }
    vector<Symbol> &output() { return output_; }
    const vector<Symbol> &output() const { return output_; }

    vector<vector<size_t>> &targets() { return attributeTargets; }
    const vector<vector<size_t>> &targets() const { return attributeTargets; }

    friend bool operator<(const Rule &lhs, const Rule &rhs) {
      return lhs.nonterminal() < rhs.nonterminal() ? true
                                                   : lhs.input() < rhs.input();
    }
    friend bool operator>(const Rule &lhs, const Rule &rhs) {
      return rhs < lhs;
    }
    friend bool operator==(const Rule &lhs, const Rule &rhs) {
      return lhs.nonterminal() == rhs.nonterminal()
                 ? lhs.input() == rhs.input() && lhs.output() == rhs.output()
                 : false;
    }
    friend bool operator!=(const Rule &lhs, const Rule &rhs) {
      return !(lhs == rhs);
    }
  };

 protected:
  vector<Terminal> terminals_;
  vector<Nonterminal> nonterminals_;
  vector<Rule> rules_;
  Symbol starting_symbol_;

 public:
  /**
  \brief Constructs empty TranslationGrammar.
  */
  TranslationGrammar();
  TranslationGrammar(const vector<Terminal> &terminals,
                     const vector<Nonterminal> &nonterminals,
                     const vector<Rule> &rules, const Symbol &starting_symbol);
  ~TranslationGrammar() = default;

  static const vector<Symbol> EPSILON_STRING;

  void swap_sides() {
    //TODO swap attribute targets
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
} //namespace ctf
#endif
/*** End of file translation_grammar.h ***/