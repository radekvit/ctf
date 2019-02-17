/**
\file translation_grammar.hpp
\brief Defines class TranslationGrammar and its subclasses and implements their
methods.
\author Radek Vít
*/
#ifndef CTF_TRANSLATION_GRAMMAR_H
#define CTF_TRANSLATION_GRAMMAR_H

#include "ctf_base.hpp"

#include <functional>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace ctf {
enum class Associativity : unsigned char {
  NONE = 0x0,
  LEFT = 0x1,
  RIGHT = 0x2,
};

struct PrecedenceSet {
  Associativity associativity;
  vector_set<Symbol> terminals;
};

class Rule {
 public:
  /**
  \brief Constructs a rule.

  \param[in] nonterminal Starting symbol.
  \param[in] input Vector of input symbols. The nonterminals must match output
  in their count and order.
  \param[in] output Vector of output symbols. The nonterminals must match
  input in their count and order.
  \param[in] attributeActions Destinations of token attributes for each
  terminal in the input. Implicitly no attribute actions take place.
  The numbers say which output symbols are the targets for each input token's
  attribute.
  */
  Rule(const Symbol nonterminal,
       const vector<Symbol>& input,
       const vector<Symbol>& output,
       const vector<vector_set<size_t>>& attributeActions = {})
      : nonterminal_(nonterminal)
      , input_(input)
      , output_(output)
      , attributeActions_(attributeActions)
      , precedenceSymbol_(Symbol::eof()) {
    check_nonterminals();

    // no actions provided
    if (attributeActions_.size() == 0) {
      create_empty_actions();
      return;
    }
    // attribute actions were provided, checking validity
    if (attributeActions_.size() != count_input_terminals())
      throw std::invalid_argument("Invalid attributeActions_ in Rule");
    for (auto& target : attributeActions_) {
      if (target.size() > output_.size())
        throw std::invalid_argument(
            "More assigned actions than symbols in output when constructing class "
            "Rule.");
      for (auto i : target) {
        if (i > output_.size() || output_[i].nonterminal())
          throw std::invalid_argument(
              "Attribute target not an output terminal when constructing class "
              "Rule.");
      }
    }
    for (auto it = input_.crbegin(); it < input_.crend(); ++it) {
      if (!it->nonterminal()) {
        precedenceSymbol_ = *it;
        break;
      }
    }
  }
  /**
  \brief Constructs a rule.

  \param[in] nonterminal Starting symbol.
  \param[in] input Vector of input symbols. The nonterminals must match output
  in their count and order.
  \param[in] output Vector of output symbols. The nonterminals must match
  input in their count and order.
  \param[in] attributeActions Destinations of token attributes for each
  terminal in the input. Implicitly no attribute actions take place.
  The numbers say which output symbols are the targets for each input token's
  attribute.
  */
  Rule(const Symbol nonterminal,
       const vector<Symbol>& input,
       const vector<Symbol>& output,
       const vector<vector_set<size_t>>& attributeActions,
       bool,
       const Symbol precedenceSymbol)
      : nonterminal_(nonterminal)
      , input_(input)
      , output_(output)
      , attributeActions_(attributeActions)
      , precedenceSymbol_(precedenceSymbol) {
    check_nonterminals();

    // no actions provided
    if (attributeActions_.size() == 0) {
      create_empty_actions();
      return;
    }
    // attribute actions were provided, checking validity
    if (attributeActions_.size() != count_input_terminals())
      throw std::invalid_argument("Invalid attributeActions in Rule.");
    for (auto& target : attributeActions_) {
      if (target.size() > output_.size())
        throw std::invalid_argument("More assigned actions than symbols in output in Rule.");
      for (auto i : target) {
        if (i > output_.size() || output_[i].nonterminal())
          throw std::invalid_argument("Attribute target not an output terminal in Rule.");
      }
    }
    if (!precedenceSymbol_.terminal()) {
      throw std::invalid_argument("Precedence symbol must be a terminal.");
    }
  }
  /**
  \brief Constructs a rule with same input and output with specified attribute
  actions.

  \param[in] nonterminal Starting symbol.
  \param[in] both Vector of input symbols and at the same time output symbols.

  Attribute targets are created to match all terminals to themselves.
  */
  Rule(const Symbol nonterminal, const vector<Symbol>& both) : Rule(nonterminal, both, both, {}) {
    // implicit target for each terminal is the identical output terminal
    size_t target = 0;
    // attribute actions have the same size as the number of terminals
    for (size_t i = 0; i < attributeActions_.size(); ++i, ++target) {
      while (output_[target].nonterminal())
        ++target;
      attributeActions_[i].insert(target);
    }
  }
  /**
  \brief Constructs a rule with same input and output with specified attribute
  actions.

  \param[in] nonterminal Starting symbol.
  \param[in] both Vector of input symbols and at the same time output symbols.

  Attribute targets are created to match all terminals to themselves.
  */
  Rule(const Symbol nonterminal, const vector<Symbol>& both, bool, const Symbol precedenceSymbol)
      : Rule(nonterminal, both, both, {}, true, precedenceSymbol) {
    // implicit target for each terminal is the identical output terminal
    size_t target = 0;
    // attribute actions have the same size as the number of terminals
    for (size_t i = 0; i < attributeActions_.size(); ++i, ++target) {
      while (output_[target].nonterminal())
        ++target;
      attributeActions_[i].insert(target);
    }
  }

  ~Rule() = default;

  Symbol& nonterminal() { return nonterminal_; }
  Symbol nonterminal() const { return nonterminal_; }

  vector<Symbol>& input() { return input_; }
  const vector<Symbol>& input() const { return input_; }

  vector<Symbol>& output() { return output_; }
  const vector<Symbol>& output() const { return output_; }

  vector<vector_set<size_t>>& actions() { return attributeActions_; }
  const vector<vector_set<size_t>>& actions() const { return attributeActions_; }

  Symbol precedence_symbol() const noexcept { return precedenceSymbol_; }

  /**
  \name Comparison operators
  \brief Lexicographic comparison of the three elements of Rules. Nonterminals
  have the highest priority, then input.

  \returns True if the comparison is true. False otherwise.
  */
  ///@{
  friend bool operator<(const Rule& lhs, const Rule& rhs) {
    if (lhs.nonterminal_ < rhs.nonterminal_) {
      return true;
    } else if (lhs.nonterminal_ == rhs.nonterminal_) {
      if (lhs.input_ < rhs.input_) {
        return true;
      } else if (lhs.input_ == rhs.input_) {
        return lhs.output_ < rhs.output_;
      } else
        return false;
    } else
      return false;
  }
  friend bool operator==(const Rule& lhs, const Rule& rhs) {
    return lhs.nonterminal() == rhs.nonterminal() && lhs.input() == rhs.input() &&
           lhs.output() == rhs.output();
  }
  friend bool operator!=(const Rule& lhs, const Rule& rhs) { return !(lhs == rhs); }
  friend bool operator>(const Rule& lhs, const Rule& rhs) { return rhs < lhs; }
  friend bool operator<=(const Rule& lhs, const Rule& rhs) { return lhs < rhs || lhs == rhs; }
  friend bool operator>=(const Rule& lhs, const Rule& rhs) { return rhs <= lhs; }
  ///@}

  string to_string() const {
    string result = nonterminal().to_string() + " -> (";
    for (auto&& symbol : input()) {
      result += ' ';
      result += symbol.to_string();
    }
    result += " ), (";
    for (auto&& symbol : output()) {
      result += ' ';
      result += symbol.to_string();
    }
    result += " )";
    return result;
  }

  explicit operator string() const { return to_string(); }

  size_t id = -1;

 protected:
  /**
  \brief Starting nonterminal for this rule.

  A rule with a symbol other than nonterminal may serve as a special marker
  rule for TranslationControl subclasses.
  */
  Symbol nonterminal_;

  /**
  \brief A vector of input symbols.
  */
  vector<Symbol> input_;
  /**
  \brief A vector of output symbols.
  */
  vector<Symbol> output_;

  /**
  \brief Attribute copying from input to output when applying this rule.
  Implicitly
  created to copy no attributes.
  */
  vector<vector_set<size_t>> attributeActions_;

  Symbol precedenceSymbol_;

  /**
  \brief Checks if nonterminals are in same space in input and output strings.
  */
  void check_nonterminals() {
    vector<Symbol> inputNonterminals;
    vector<Symbol> outputNonterminals;
    for (auto& s : input_) {
      if (s.nonterminal())
        inputNonterminals.push_back(s);
    }
    for (auto& s : output_) {
      if (s.nonterminal())
        outputNonterminals.push_back(s);
    }
    if (inputNonterminals != outputNonterminals)
      throw std::invalid_argument("Input and output nonterminals must match.");
  }
  /**
  \brief Counts input nonterminals.
  */
  size_t count_input_terminals() const {
    size_t count = 0;
    for (auto& s : input_) {
      if (s.type() != Symbol::Type::NONTERMINAL)
        count++;
    }
    return count;
  }
  /**
  \brief Creates empty actions for terminal attributes.
  */
  void create_empty_actions() {
    attributeActions_ = vector<vector_set<size_t>>(count_input_terminals(), vector_set<size_t>());
  }
};

/**
\brief Translation grammar.
*/
class TranslationGrammar {
 public:
  using Rule = ctf::Rule;
  /**
  \brief A single rule. Is identified by a starting nonterminal and two vectors
  of input and output symbols.

  Defines attribute actions during syntax analysis for input terminals.
  Rules with starting symbols that are not nonterminals may serve as special
  markers for TranslationControl subclasses.
  */

  /**
  \brief Constructs an empty TranslationGrammar. Implicit starting symbol is
  0_nt.
  */
  TranslationGrammar()
      : terminals_(1)
      , nonterminals_(2)
      , rules_({Rule(1_nt, {}), Rule(0_nt, {1_nt, Symbol::eof()})})
      , starting_symbol_(0_nt) {}
  /**
  \brief Constructs a TranslationGrammar, takes terminals and nonterminals
  from the rules' inputs and starting symbol.

  \param[in] rules A vector of rules.
  \param[in] starting_symbol The starting symbol. Precondition: This symbol
  */
  TranslationGrammar(const vector<Rule>& rules,
                     const Symbol starting_symbol,
                     const vector<PrecedenceSet>& precedences = {})
      : rules_(rules), starting_symbol_(starting_symbol), precedences_(precedences) {
    init_from_rules();
  }
  /**
  \brief Constructs a TranslationGrammar from rvalue references, takes terminals and nonterminals
  from the rules' inputs and starting symbol.

  \param[in] rules A vector of rules.
  \param[in] starting_symbol The starting symbol. Precondition: This symbol
  */
  TranslationGrammar(vector<Rule>&& rules,
                     const Symbol starting_symbol,
                     vector<PrecedenceSet>&& precedences = {})
      : rules_(rules), starting_symbol_(starting_symbol), precedences_(precedences) {
    init_from_rules();
  }

  /**
  \brief Constructs a TranslationGrammar.

  \param[in] terminals A vector of all terminals. Duplicates will be erased.
  \param[in] nonterminals A vector of all nonterminals. Duplicates will be
  erased.
  \param[in] rules A vector of all rules.
  \param[in] starting_symbol The starting symbol.

  Checks rules for validity with supplied terminals and nonterminals.
  */
  TranslationGrammar(size_t nonterminals,
                     size_t terminals,
                     const vector<Rule>& rules,
                     const Symbol starting_symbol,
                     const vector<PrecedenceSet>& precedences = {})
      : terminals_(terminals)
      , nonterminals_(nonterminals)
      , rules_(rules)
      , starting_symbol_(starting_symbol)
      , precedences_(precedences) {
    init_from_all();
  }

  /**
  \brief Constructs a TranslationGrammar from rvalue references.

  \param[in] terminals A vector of all terminals. Duplicates will be erased.
  \param[in] nonterminals A vector of all nonterminals. Duplicates will be
  erased.
  \param[in] rules A vector of all rules.
  \param[in] starting_symbol The starting symbol.

  Checks rules for validity with supplied terminals and nonterminals.
  */
  TranslationGrammar(size_t nonterminals,
                     size_t terminals,
                     vector<Rule>&& rules,
                     const Symbol starting_symbol,
                     vector<PrecedenceSet>&& precedences = {})
      : terminals_(terminals)
      , nonterminals_(nonterminals)
      , rules_(rules)
      , starting_symbol_(starting_symbol)
      , precedences_(precedences) {
    init_from_all();
  }

  ~TranslationGrammar() = default;

  size_t& terminals() noexcept { return terminals_; }
  size_t terminals() const noexcept { return terminals_; }

  size_t& nonterminals() noexcept { return nonterminals_; }
  size_t nonterminals() const noexcept { return nonterminals_; }

  Symbol& starting_symbol() noexcept { return starting_symbol_; }
  Symbol starting_symbol() const noexcept { return starting_symbol_; }

  vector<Rule>& rules() & noexcept { return rules_; }
  const vector<Rule>& rules() const& noexcept { return rules_; }
  vector<Rule>&& rules() && noexcept { return std::move(rules_); }

  const Rule& starting_rule() const& noexcept { return rules_.back(); }
  Rule&& starting_rule() && noexcept { return std::move(rules_.back()); }

  tuple<Associativity, size_t> precedence(const Symbol symbol) const& noexcept {
    for (size_t i = 0; i < precedences_.size(); ++i) {
      if (precedences_[i].terminals.contains(symbol)) {
        return {precedences_[i].associativity, i};
      }
    }
    return {Associativity::NONE, std::numeric_limits<size_t>::max()};
  }

 protected:
  /**
  \brief A sorted set of all input terminals.
  */
  size_t terminals_;
  /**
  \brief A sorted set of all input nonterminals.
  */
  size_t nonterminals_;
  /**
  \brief An unsorted vector of all rules.
  */
  vector<Rule> rules_;
  /**
  \brief The starting symbol.
  */
  Symbol starting_symbol_;
  /**
  \brief The precedence and associativity of operators.
  */
  vector<PrecedenceSet> precedences_;

  void init_from_rules() {
    // the starting symbol must be a nonterminal
    if (!starting_symbol_.nonterminal())
      throw std::invalid_argument(
          "Starting symbol is not a nonterminal when constructing "
          "TranslationGrammar.");
    nonterminals_ = starting_symbol_.id() + 1;
    terminals_ = 1;

    // filling the terminal and nonterminal sets
    for (auto& r : rules_) {
      nonterminals_ = std::max(nonterminals_, r.nonterminal().id() + 1);
      for (auto& s : r.input()) {
        switch (s.type()) {
          case Symbol::Type::NONTERMINAL:
            nonterminals_ = std::max(nonterminals_, s.id() + 1);
            break;
          case Symbol::Type::TERMINAL:
            terminals_ = std::max(terminals_, s.id() + 1);
            break;
          default:
            // ignore EOI, terminals_ is already at least 1
            break;
        }  // switch
      }    // for all input
    }      // for all rules
    make_augmented();
    mark_rules();
  }

  void init_from_all() {
    if (!starting_symbol_.nonterminal())
      throw std::invalid_argument(
          "Starting symbol is not a nonterminal when constructing "
          "TranslationGrammar.");
    if (terminals_ < 1)
      throw std::invalid_argument("All grammars must have at least one terminal (EOF)");

    for (auto& r : rules_) {
      if (r.nonterminal().id() >= nonterminals_)
        throw std::invalid_argument("Rule with production from nonterminal " +
                                    r.nonterminal().to_string() + ", no such nonterminal.");

      for (auto& s : r.input()) {
        switch (s.type()) {
          case Symbol::Type::NONTERMINAL:
            if (s.id() >= nonterminals_)
              throw std::invalid_argument("Rule with unknown nonterminal " + s.to_string() + ".");
            break;
          case Symbol::Type::TERMINAL:
          case Symbol::Type::EOI:
            if (s.id() >= terminals_)
              throw std::invalid_argument("Rule with unknown terminal " + s.to_string() + ".");
            break;
        }  // switch
      }    // for all input
    }      // for all rules
    make_augmented();
    mark_rules();
  }

  void mark_rules() {
    for (size_t i = 0; i < rules_.size(); ++i) {
      rules_[i].id = i;
    }
  }

  /**
   * Transforms a translation grammar into an augmented translation grammar.
   */
  void make_augmented() {
    // create a new unique starting symbol
    Symbol newStartingNonterminal = Nonterminal(nonterminals_);
    ++nonterminals_;
    // insert rule S' -> (S eof, S eof)
    rules_.push_back({newStartingNonterminal, {starting_symbol(), Symbol::eof()}});
    // make S' the new starting symbol
    starting_symbol_ = newStartingNonterminal;
  }
};
}  // namespace ctf
#endif
/*** End of file translation_grammar.hpp ***/