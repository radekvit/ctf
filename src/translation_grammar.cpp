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
  vector<Nonterminal> inputNonterminals;
  vector<Nonterminal> outputNonterminals;
  for (auto &s : input_) {
    if (s.type == Symbol::Type::NONTERMINAL)
      inputNonterminals.push_back(s.nonterminal);
  }
  for (auto &s : output_) {
    if (s.type == Symbol::Type::NONTERMINAL)
      outputNonterminals.push_back(s.nonterminal);
  }
  if (inputNonterminals != outputNonterminals)
    throw std::invalid_argument("Input and output nonterminals must match.");
}

TranslationGrammar::TranslationGrammar(const vector<Terminal> &_terminals,
                                       const vector<Nonterminal> &_nonterminals,
                                       const vector<Rule> &_rules,
                                       const Symbol &starting_symbol)
    : terminals_(_terminals),
      nonterminals_(_nonterminals),
      rules_(_rules),
      starting_symbol_(starting_symbol) {
  make_set(terminals_);
  make_set(nonterminals_);
  make_set(rules_);
  if (starting_symbol_.type != Symbol::Type::NONTERMINAL)
    throw std::invalid_argument("Starting symbol must be a nonterminal");
  for (auto &r : rules_) {
    if (!is_in(nonterminals_, r.nonterminal()))
      throw std::invalid_argument("Rule with nonterminal " +
                                  r.nonterminal().name() +
                                  ", no such nonterminal.");
  }
}

size_t TranslationGrammar::nonterminal_index(const Nonterminal &nt) const {
  return std::lower_bound(nonterminals_.begin(), nonterminals_.end(), nt) -
         nonterminals_.begin();
}

size_t TranslationGrammar::terminal_index(const Terminal &t) const {
  return std::lower_bound(terminals_.begin(), terminals_.end(), t) -
         terminals_.begin();
}

#if 0
//this transforms Translation Grammar to LL grammar if possible; currently not used

Nonterminal create_new_nonterminal(const vector<Nonterminal> &nonterminals,
                                   const string &base, const char suffix)
{
    string name = base;
    do {
        name += suffix;
    } while (is_in(nonterminals, Nonterminal(name)));

    return Nonterminal(name);
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
        for (auto it = rules_.at(n).begin(); it != rules_.at(n).end(); ++it) {
            auto &r = *it;                        // get reference to rule
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

            Nonterminal newNonterminal =
                create_new_nonterminal(newNonterminals, n.name(), SUFFIX);
            Symbol newNonterminalSymbol(newNonterminal);
            newNonterminals.push_back(newNonterminal);

            // fix rule in newNonterminalRules
            vector<Symbol> input(newNonterminalRules[0].input());
            vector<Symbol> output(newNonterminalRules[0].output());
            if (output[0] != input[0])
                throw LLConversionException(
                    "This grammar cannot be converted.");
            input.erase(input.begin());             // remove nonterminal
            input.push_back(newNonterminalSymbol);  // append new nonterminal
            output.erase(output.begin());           // remove origin nonterminal
            output.push_back(newNonterminalSymbol); // append new nonterminal
            newNonterminalRules[0] = Rule(newNonterminal, input, output);
            // add epsilon rule to newNonterminalRules
            newNonterminalRules.push_back(
                Rule(newNonterminal, EPSILON_STRING,
                     EPSILON_STRING)); // add rule to epsilon

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
#endif
}