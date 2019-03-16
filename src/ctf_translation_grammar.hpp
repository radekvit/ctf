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
    : _nonterminal(nonterminal)
    , _input(input)
    , _output(output)
    , _attributeActions(attributeActions)
    , _precedenceSymbol(Symbol::eof()) {
    check_nonterminals();

    // no actions provided
    if (_attributeActions.size() == 0) {
      create_empty_actions();
      return;
    }
    // attribute actions were provided, checking validity
    if (_attributeActions.size() != count_input_terminals()) {
      throw std::invalid_argument("Invalid attribute actions in Rule");
    }
    for (auto& target : _attributeActions) {
      if (target.size() > _output.size())
        throw std::invalid_argument(
          "More assigned actions than symbols in output when constructing class "
          "Rule.");
      for (auto i : target) {
        if (i > _output.size() || _output[i].nonterminal())
          throw std::invalid_argument(
            "Attribute target not an output terminal when constructing class "
            "Rule.");
      }
    }
    for (auto it = _input.crbegin(); it < _input.crend(); ++it) {
      if (!it->nonterminal()) {
        _precedenceSymbol = *it;
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
    : _nonterminal(nonterminal)
    , _input(input)
    , _output(output)
    , _attributeActions(attributeActions)
    , _precedenceSymbol(precedenceSymbol) {
    check_nonterminals();

    // no actions provided
    if (_attributeActions.size() == 0) {
      create_empty_actions();
      return;
    }
    // attribute actions were provided, checking validity
    if (_attributeActions.size() != count_input_terminals())
      throw std::invalid_argument("Invalid attributeActions in Rule.");
    for (auto& target : _attributeActions) {
      if (target.size() > _output.size())
        throw std::invalid_argument("More assigned actions than symbols in output in Rule.");
      for (auto i : target) {
        if (i > _output.size() || _output[i].nonterminal())
          throw std::invalid_argument("Attribute target not an output terminal in Rule.");
      }
    }
    if (!_precedenceSymbol.terminal()) {
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
    for (size_t i = 0; i < _attributeActions.size(); ++i, ++target) {
      while (_output[target].nonterminal())
        ++target;
      _attributeActions[i].insert(target);
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
    for (size_t i = 0; i < _attributeActions.size(); ++i, ++target) {
      while (_output[target].nonterminal())
        ++target;
      _attributeActions[i].insert(target);
    }
  }

  ~Rule() = default;

  Symbol& nonterminal() { return _nonterminal; }
  Symbol nonterminal() const { return _nonterminal; }

  vector<Symbol>& input() { return _input; }
  const vector<Symbol>& input() const { return _input; }

  vector<Symbol>& output() { return _output; }
  const vector<Symbol>& output() const { return _output; }

  vector<vector_set<size_t>>& actions() { return _attributeActions; }
  const vector<vector_set<size_t>>& actions() const { return _attributeActions; }

  Symbol precedence_symbol() const noexcept { return _precedenceSymbol; }

  /**
  \name Comparison operators
  \brief Lexicographic comparison of the three elements of Rules. Nonterminals
  have the highest priority, then input.

  \returns True if the comparison is true. False otherwise.
  */
  ///@{
  friend bool operator<(const Rule& lhs, const Rule& rhs) {
    if (lhs._nonterminal < rhs._nonterminal) {
      return true;
    } else if (lhs._nonterminal == rhs._nonterminal) {
      if (lhs._input < rhs._input) {
        return true;
      } else if (lhs._input == rhs._input) {
        return lhs._output < rhs._output;
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

  string to_string(symbol_string_fn to_str = ctf::to_string) const {
    string result = to_str(nonterminal()) + " -> (";
    for (auto& symbol : input()) {
      result += ' ';
      result += to_str(symbol);
    }
    result += " ), (";
    for (auto& symbol : output()) {
      result += ' ';
      result += to_str(symbol);
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
  Symbol _nonterminal;

  /**
  \brief A vector of input symbols.
  */
  vector<Symbol> _input;
  /**
  \brief A vector of output symbols.
  */
  vector<Symbol> _output;

  /**
  \brief Attribute copying from input to output when applying this rule.
  Implicitly
  created to copy no attributes.
  */
  vector<vector_set<size_t>> _attributeActions;

  Symbol _precedenceSymbol;

  /**
  \brief Checks if nonterminals are in same space in input and output strings.
  */
  void check_nonterminals() {
    vector<Symbol> inputNonterminals;
    vector<Symbol> outputNonterminals;
    for (auto& s : _input) {
      if (s.nonterminal())
        inputNonterminals.push_back(s);
    }
    for (auto& s : _output) {
      if (s.nonterminal())
        outputNonterminals.push_back(s);
    }
    if (inputNonterminals != outputNonterminals) {
      throw std::invalid_argument("Input and output nonterminals must match.");
    }
  }
  /**
  \brief Counts input nonterminals.
  */
  size_t count_input_terminals() const {
    size_t count = 0;
    for (auto& s : _input) {
      if (s.type() != Symbol::Type::NONTERMINAL)
        count++;
    }
    return count;
  }
  /**
  \brief Creates empty actions for terminal attributes.
  */
  void create_empty_actions() {
    _attributeActions = vector<vector_set<size_t>>(count_input_terminals(), vector_set<size_t>());
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
    : _terminals(1)
    , _nonterminals(2)
    , _rules({Rule(1_nt, {}), Rule(0_nt, {1_nt, Symbol::eof()})})
    , _startingSymbol(0_nt) {}
  /**
  \brief Constructs a TranslationGrammar, takes terminals and nonterminals
  from the rules' inputs and starting symbol.

  \param[in] rules A vector of rules.
  \param[in] starting_symbol The starting symbol. Precondition: This symbol
  */
  TranslationGrammar(const vector<Rule>& rules,
                     const Symbol starting_symbol,
                     const vector<PrecedenceSet>& precedences = {})
    : _rules(rules), _startingSymbol(starting_symbol), _precedences(precedences) {
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
    : _rules(rules), _startingSymbol(starting_symbol), _precedences(precedences) {
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
    : _terminals(terminals)
    , _nonterminals(nonterminals)
    , _rules(rules)
    , _startingSymbol(starting_symbol)
    , _precedences(precedences) {
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
    : _terminals(terminals)
    , _nonterminals(nonterminals)
    , _rules(rules)
    , _startingSymbol(starting_symbol)
    , _precedences(precedences) {
    init_from_all();
  }

  ~TranslationGrammar() = default;

  size_t& terminals() noexcept { return _terminals; }
  size_t terminals() const noexcept { return _terminals; }

  size_t& nonterminals() noexcept { return _nonterminals; }
  size_t nonterminals() const noexcept { return _nonterminals; }

  Symbol& starting_symbol() noexcept { return _startingSymbol; }
  Symbol starting_symbol() const noexcept { return _startingSymbol; }

  vector<Rule>& rules() & noexcept { return _rules; }
  const vector<Rule>& rules() const& noexcept { return _rules; }
  vector<Rule>&& rules() && noexcept { return std::move(_rules); }

  const Rule& starting_rule() const& noexcept { return _rules.back(); }
  Rule&& starting_rule() && noexcept { return std::move(_rules.back()); }

  tuple<Associativity, size_t> precedence(const Symbol symbol) const& noexcept {
    for (size_t i = 0; i < _precedences.size(); ++i) {
      if (_precedences[i].terminals.contains(symbol)) {
        return {_precedences[i].associativity, i};
      }
    }
    return {Associativity::NONE, std::numeric_limits<size_t>::max()};
  }

 protected:
  /**
  \brief A sorted set of all input terminals.
  */
  size_t _terminals;
  /**
  \brief A sorted set of all input nonterminals.
  */
  size_t _nonterminals;
  /**
  \brief An unsorted vector of all rules.
  */
  vector<Rule> _rules;
  /**
  \brief The starting symbol.
  */
  Symbol _startingSymbol;
  /**
  \brief The precedence and associativity of operators.
  */
  vector<PrecedenceSet> _precedences;

  void init_from_rules() {
    // the starting symbol must be a nonterminal
    if (!_startingSymbol.nonterminal())
      throw std::invalid_argument(
        "Starting symbol is not a nonterminal when constructing "
        "TranslationGrammar.");
    _nonterminals = _startingSymbol.id() + 1;
    _terminals = 1;

    // filling the terminal and nonterminal sets
    for (auto& r : _rules) {
      _nonterminals = std::max(_nonterminals, r.nonterminal().id() + 1);
      for (auto& s : r.input()) {
        switch (s.type()) {
          case Symbol::Type::NONTERMINAL:
            _nonterminals = std::max(_nonterminals, s.id() + 1);
            break;
          case Symbol::Type::TERMINAL:
            _terminals = std::max(_terminals, s.id() + 1);
            break;
          default:
            // ignore EOI, _terminals is already at least 1
            break;
        }  // switch
      }    // for all input
    }      // for all rules
    make_augmented();
    mark_rules();
  }

  void init_from_all() {
    if (!_startingSymbol.nonterminal())
      throw std::invalid_argument(
        "Starting symbol is not a nonterminal when constructing "
        "TranslationGrammar.");
    if (_terminals < 1)
      throw std::invalid_argument("All grammars must have at least one terminal (EOF)");

    for (auto& r : _rules) {
      if (r.nonterminal().id() >= _nonterminals)
        throw std::invalid_argument("Rule with production from nonterminal " +
                                    r.nonterminal().to_string() + ", no such nonterminal.");

      for (auto& s : r.input()) {
        switch (s.type()) {
          case Symbol::Type::NONTERMINAL:
            if (s.id() >= _nonterminals)
              throw std::invalid_argument("Rule with unknown nonterminal " + s.to_string() + ".");
            break;
          case Symbol::Type::TERMINAL:
          case Symbol::Type::EOI:
            if (s.id() >= _terminals)
              throw std::invalid_argument("Rule with unknown terminal " + s.to_string() + ".");
            break;
        }  // switch
      }    // for all input
    }      // for all rules
    make_augmented();
    mark_rules();
  }

  void mark_rules() {
    for (size_t i = 0; i < _rules.size(); ++i) {
      _rules[i].id = i;
    }
  }

  /**
   * Transforms a translation grammar into an augmented translation grammar.
   */
  void make_augmented() {
    // create a new unique starting symbol
    Symbol newStartingNonterminal = Nonterminal(_nonterminals);
    ++_nonterminals;
    // insert rule S' -> (S eof, S eof)
    _rules.push_back({newStartingNonterminal, {starting_symbol(), Symbol::eof()}});
    // make S' the new starting symbol
    _startingSymbol = newStartingNonterminal;
  }
};
}  // namespace ctf
#endif
/*** End of file translation_grammar.hpp ***/