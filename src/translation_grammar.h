/**
\file translation_grammar.h
\brief Defines class TranslationGrammar and its subclasses and implements their
methods.
\author Radek Vít
*/
#ifndef CTF_TG_H
#define CTF_TG_H

#include <functional>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "base.h"

namespace ctf {
class LLTable;

class TranslationGrammar {
 public:
  class Rule {
   protected:
    Symbol nonterminal_;

    vector<Symbol> input_;
    vector<Symbol> output_;

    /**
    \brief Attribute copying from input to output in each rule. Implicitly
    created to copy in-order.
     */
    vector<vector<size_t>> attributeActions;

    /**
    \brief Checks if nonterminals are in same space in input and output strings.
    */
    void check_nonterminals() {
      nonterminal_.type() = Symbol::Type::NONTERMINAL;
      vector<Symbol> inputNonterminals;
      vector<Symbol> outputNonterminals;
      for (auto &s : input_) {
        if (s.type() == Symbol::Type::NONTERMINAL)
          inputNonterminals.push_back(s);
        else if (s.type() != Symbol::Type::TERMINAL &&
                 s.type() != Symbol::Type::SPECIAL)
          throw std::invalid_argument("Unknown symbol type in Rule input.");
      }
      for (auto &s : output_) {
        if (s.type() == Symbol::Type::NONTERMINAL)
          outputNonterminals.push_back(s);
        else if (s.type() != Symbol::Type::TERMINAL &&
                 s.type() != Symbol::Type::SPECIAL)
          throw std::invalid_argument("Unknown symbol type in Rule output.");
      }
      if (inputNonterminals != outputNonterminals)
        throw std::invalid_argument(
            "Input and output nonterminals must match.");
    }
    /**
    \brief Counts input nonterminals.
    */
    size_t count_input_terminals() const {
      size_t count = 0;
      for (auto &s : input_) {
        if (s.type() == Symbol::Type::TERMINAL)
          count++;
      }
      return count;
    }
    /**
    \brief Creates implicit actions for terminal attributes.
    */
    void create_implicit_actions() {
      attributeActions =
          vector<vector<size_t>>(count_input_terminals(), vector<size_t>());
      auto oit = output_.begin();
      while (oit != output_.end() && oit->type() != Symbol::Type::TERMINAL) {
        ++oit;
      }
      if (oit == output_.end())
        return;
      int i = 0;
      for (auto it = input_.begin(); it < input_.end(); ++it) {
        if (it->type() != Symbol::Type::TERMINAL)
          continue;
        attributeActions[i].push_back(oit - output_.begin());
        ++i;
        while (oit != output_.end() && oit->type() != Symbol::Type::TERMINAL) {
          ++oit;
        }
        if (oit == output_.end())
          return;
      }
    }

   public:
    /**
    \brief Constructs a rule with implicit attribute actions.
    */
    Rule(const Symbol &_nonterminal, const vector<Symbol> &_input,
         const vector<Symbol> &_output)
        : nonterminal_(_nonterminal),
          input_(_input),
          output_(_output),
          attributeActions(vector<vector<size_t>>()) {
      check_nonterminals();
      create_implicit_actions();
    }
    /**
    \brief Constructs a rule.
    */
    Rule(const Symbol &_nonterminal, const vector<Symbol> &_input,
         const vector<Symbol> &_output,
         const vector<vector<size_t>> &_attributeActions)
        : nonterminal_(_nonterminal),
          input_(_input),
          output_(_output),
          attributeActions(_attributeActions) {
      check_nonterminals();

      if (attributeActions.size() != count_input_terminals())
        throw std::invalid_argument(
            "Invalid attributeActions when "
            "constructing class "
            "TranslationGrammar::Rule.");
      for (auto &target : attributeActions) {
        if (target.size() > output_.size())
          throw std::invalid_argument(
              "More assigned actions than symbols in output when "
              "constructing class TranslationGrammar::Rule.");
        for (auto i : target) {
          if (i > output_.size() || output_[i].type() != Symbol::Type::TERMINAL)
            throw std::invalid_argument(
                "Attribute target not an output terminal when constructing "
                "class TranslationGrammar::Rule.");
        }
      }
    }
    /**
    \brief Constructs a rule with same input and output. Attribute actions are
    implicit.
    */
    Rule(const Symbol &_nonterminal, const vector<Symbol> &_both)
        : Rule(_nonterminal, _both, _both) {}
    ~Rule() = default;
    void swap_sides() { std::swap(input_, output_); }

    Symbol &nonterminal() { return nonterminal_; }
    const Symbol &nonterminal() const { return nonterminal_; }
    vector<Symbol> &input() { return input_; }
    const vector<Symbol> &input() const { return input_; }
    vector<Symbol> &output() { return output_; }
    const vector<Symbol> &output() const { return output_; }

    vector<vector<size_t>> &actions() { return attributeActions; }
    const vector<vector<size_t>> &actions() const { return attributeActions; }

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
  vector<Symbol> terminals_;
  vector<Symbol> nonterminals_;
  vector<Rule> rules_;
  Symbol starting_symbol_;

 public:
  /**
  \brief Constructs empty TranslationGrammar.
  */
  TranslationGrammar();
  /**
  \brief Constructs a TranslationGrammar, takes terminals and nonterminals from
  rules and starting symbol.
  */
  TranslationGrammar(const vector<Rule> &rules, const Symbol &starting_symbol)
      : rules_(rules), starting_symbol_(starting_symbol) {
    make_set(rules_);
    /* add nonterminals and terminals */
    starting_symbol_.type() = Symbol::Type::NONTERMINAL;
    nonterminals_.push_back(starting_symbol_);
    for (auto &r : rules_) {
      nonterminals_.push_back(r.nonterminal());
      for (auto &s : r.input()) {
        switch (s.type()) {
          case Symbol::Type::NONTERMINAL:
            nonterminals_.push_back(s);
            break;
          case Symbol::Type::TERMINAL:
            terminals_.push_back(s);
          default:
            // ignore all other types
            break;
        }  // switch
      }    // for all input
    }      // for all rules
    make_set(terminals_);
    make_set(nonterminals_);
  }
  /**
  \brief Constructs a TranslationGrammar
  */
  TranslationGrammar(const vector<Symbol> &terminals,
                     const vector<Symbol> &nonterminals,
                     const vector<Rule> &rules, const Symbol &starting_symbol)
      : terminals_(terminals),
        nonterminals_(nonterminals),
        rules_(rules),
        starting_symbol_(starting_symbol) {
    make_set(terminals_);
    make_set(nonterminals_);
    make_set(rules_);
    for (auto &t : terminals_) {
      t.type() = Symbol::Type::TERMINAL;
    }
    for (auto &nt : nonterminals_) {
      nt.type() = Symbol::Type::NONTERMINAL;
    }
    starting_symbol_.type() = Symbol::Type::NONTERMINAL;
    for (auto &r : rules_) {
      if (!is_in(nonterminals_, r.nonterminal()))
        throw std::invalid_argument("Rule with nonterminal " +
                                    r.nonterminal().name() +
                                    ", no such nonterminal.");
      for (auto &s : r.input()) {
        switch (s.type()) {
          case Symbol::Type::TERMINAL:
            if (!is_in(terminals_, s))
              throw std::invalid_argument("Rule with unknown terminal " +
                                          s.name() + ".");
            break;
          case Symbol::Type::NONTERMINAL:
            if (!is_in(terminals_, s))
              throw std::invalid_argument("Rule with unknown nonterminal " +
                                          s.name() + ".");
            break;
          default:
            // should never happen
            break;
        }  // switch
      }    // for all input
    }      // for all rules
  }
  ~TranslationGrammar() = default;

  static const vector<Symbol> &EPSILON_STRING() {
    static vector<Symbol> es{};
    return es;
  };

  vector<Symbol> &terminals() { return terminals_; }
  const vector<Symbol> &terminals() const { return terminals_; }
  vector<Symbol> &nonterminals() { return nonterminals_; }
  const vector<Symbol> &nonterminals() const { return nonterminals_; }
  vector<Rule> &rules() { return rules_; }
  const vector<Rule> &rules() const { return rules_; }
  Symbol &starting_symbol() { return starting_symbol_; }
  const Symbol &starting_symbol() const { return starting_symbol_; }

  /**
  \brief Returns a nonterminal's index.
  */
  size_t nonterminal_index(const Symbol &nt) const {
    return std::lower_bound(nonterminals_.begin(), nonterminals_.end(), nt) -
           nonterminals_.begin();
  }
  /**
  \brief Returns a terminal's index.
  */
  size_t terminal_index(const Symbol &t) const {
    return std::lower_bound(terminals_.begin(), terminals_.end(), t) -
           terminals_.begin();
  }
};
}  // namespace ctf
#endif
/*** End of file translation_grammar.h ***/