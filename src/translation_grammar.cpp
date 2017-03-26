/**
\file translation_grammar.cpp
\brief Implements TranslationGrammar and its subclasses' methods.
\author Radek Vít
*/
#include <ll_table.h>
#include <translation_grammar.h>
#include <algorithm>
#include <stdexcept>
namespace ctf {

/**
\brief Empty string.
*/
const vector<Symbol> TranslationGrammar::EPSILON_STRING = vector<Symbol>();

void TranslationGrammar::Rule::check_nonterminals() {
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
    else if (s.type() != Symbol::Type::TERMINAL)
      throw std::invalid_argument("Unknown symbol type in Rule output.");
  }
  if (inputNonterminals != outputNonterminals)
    throw std::invalid_argument("Input and output nonterminals must match.");
}

TranslationGrammar::TranslationGrammar(const vector<Rule> &_rules,
                                       const Symbol &starting_symbol)
    : rules_(_rules), starting_symbol_(starting_symbol) {
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

TranslationGrammar::TranslationGrammar(const vector<Symbol> &_terminals,
                                       const vector<Symbol> &_nonterminals,
                                       const vector<Rule> &_rules,
                                       const Symbol &starting_symbol)
    : terminals_(_terminals),
      nonterminals_(_nonterminals),
      rules_(_rules),
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

size_t TranslationGrammar::nonterminal_index(const Symbol &nt) const {
  return std::lower_bound(nonterminals_.begin(), nonterminals_.end(), nt) -
         nonterminals_.begin();
}

size_t TranslationGrammar::terminal_index(const Symbol &t) const {
  return std::lower_bound(terminals_.begin(), terminals_.end(), t) -
         terminals_.begin();
}

}  // namespace ctf
   /*** End of file translation_grammar.cpp ***/