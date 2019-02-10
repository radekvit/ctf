/**
\file translation_grammar.hpp
\brief Defines class TranslationGrammar and its subclasses and implements their
methods.
\author Radek Vít
*/
#ifndef CTF_TRANSLATION_GRAMMAR_H
#define CTF_TRANSLATION_GRAMMAR_H

#include <functional>
#include <ostream>
#include <stdexcept>
#include <utility>
#include "ctf_base.hpp"

namespace ctf {

/**
\brief Translation grammar.
*/
class TranslationGrammar {
 public:
  /**
  \brief A single rule. Is identified by a starting nonterminal and two vectors
  of input and output symbols.

  Defines attribute actions during syntax analysis for input terminals.
  Rules with starting symbols that are not nonterminals may serve as special
  markers for TranslationControl subclasses.
  */
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
    Rule(const Symbol& nonterminal,
         const vector<Symbol>& input,
         const vector<Symbol>& output,
         const vector<vector_set<size_t>>& attributeActions = {})
        : nonterminal_(nonterminal)
        , input_(input)
        , output_(output)
        , attributeActions_(attributeActions) {
      check_nonterminals();

      // no actions provided
      if (attributeActions_.size() == 0) {
        create_empty_actions();
        return;
      }
      // attribute actions were provided, checking validity
      if (attributeActions_.size() != count_input_terminals())
        throw std::invalid_argument(
            "Invalid attributeActions_ when constructing class TranslationGrammar::Rule.");
      for (auto& target : attributeActions_) {
        if (target.size() > output_.size())
          throw std::invalid_argument(
              "More assigned actions than symbols in output when constructing class "
              "TranslationGrammar::Rule.");
        for (auto i : target) {
          if (i > output_.size() || output_[i].type() == Symbol::Type::NONTERMINAL)
            throw std::invalid_argument(
                "Attribute target not an output terminal when constructing class "
                "TranslationGrammar::Rule.");
        }
      }
    }
    /**
    \brief Constructs a rule with same input and output with specified attribute
    actions.

    \param[in] nonterminal Starting symbol.
    \param[in] both Vector of input symbols and at the same time output symbols.

    Attribute targets are created to match all terminals to themselves.
    */
    Rule(const Symbol& nonterminal, const vector<Symbol>& both) : Rule(nonterminal, both, both) {
      // implicit target for each terminal is the identical output terminal
      size_t target = 0;
      // attribute actions have the same size as the number of terminals
      for (size_t i = 0; i < attributeActions_.size(); ++i, ++target) {
        while (output_[target].type() == Symbol::Type::NONTERMINAL)
          ++target;
        attributeActions_[i].insert(target);
      }
    }

    ~Rule() = default;

    Symbol& nonterminal() { return nonterminal_; }
    const Symbol& nonterminal() const { return nonterminal_; }

    vector<Symbol>& input() { return input_; }
    const vector<Symbol>& input() const { return input_; }

    vector<Symbol>& output() { return output_; }
    const vector<Symbol>& output() const { return output_; }

    vector<vector_set<size_t>>& actions() { return attributeActions_; }
    const vector<vector_set<size_t>>& actions() const { return attributeActions_; }

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

    /**
    \brief Checks if nonterminals are in same space in input and output strings.
    */
    void check_nonterminals() {
      vector<Symbol> inputNonterminals;
      vector<Symbol> outputNonterminals;
      for (auto& s : input_) {
        if (s.type() == Symbol::Type::NONTERMINAL)
          inputNonterminals.push_back(s);
      }
      for (auto& s : output_) {
        if (s.type() == Symbol::Type::NONTERMINAL)
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
  \brief Constructs an empty TranslationGrammar. Implicit starting symbol is
  "S'"_nt.
  */
  TranslationGrammar()
      : terminals_({Symbol::eof()})
      , nonterminals_({"S"_nt, "S'"_nt})
      , rules_({Rule("S'"_nt, {"S"_nt, Symbol::eof()})})
      , starting_symbol_("S'"_nt) {}
  /**
  \brief Constructs a TranslationGrammar, takes terminals and nonterminals
  from the rules' inputs and starting symbol.

  \param[in] rules A vector of rules. Duplicates will be erased, even with
  different atttribute actions.
  \param[in] starting_symbol The starting symbol.
  */
  TranslationGrammar(const vector<Rule>& rules, const Symbol& starting_symbol)
      : rules_(rules), starting_symbol_(starting_symbol) {
    // the starting symbol must be a nonterminal
    if (starting_symbol_.type() != Symbol::Type::NONTERMINAL)
      throw std::invalid_argument(
          "Starting symbol is not a nonterminal when constructing "
          "TranslationGrammar.");
    nonterminals_.push_back(starting_symbol_);

    // filling the terminal and nonterminal sets
    for (auto& r : rules_) {
      nonterminals_.push_back(r.nonterminal());
      for (auto& s : r.input()) {
        switch (s.type()) {
          case Symbol::Type::NONTERMINAL:
            nonterminals_.push_back(s);
            break;
          case Symbol::Type::TERMINAL:
            terminals_.push_back(s);
            break;
          default:
            // ignore all other types
            break;
        }  // switch
      }    // for all input
    }      // for all rules
    make_set(nonterminals_);
    make_augmented();
    mark_rules();
  }
  /**
  \brief Constructs a TranslationGrammar.

  \param[in] terminals A vector of all terminals. Duplicates will be erased.
  \param[in] nonterminals A vector of all nonterminals. Duplicates will be
  erased.
  \param[in] rules A vector of all rules. Duplicates will be erased, even with
  different atttribute actions.
  \param[in] starting_symbol The starting symbol.

  Checks rules for validity with supplied terminals and nonterminals.
  */
  TranslationGrammar(const vector<Symbol>& nonterminals,
                     const vector<Symbol>& terminals,
                     const vector<Rule>& rules,
                     const Symbol& starting_symbol)
      : terminals_(terminals)
      , nonterminals_(nonterminals)
      , rules_(rules)
      , starting_symbol_(starting_symbol) {
    make_set(terminals_);
    make_set(nonterminals_);

    for (auto& t : terminals_) {
      if (t == Symbol::eof())
        throw std::invalid_argument("EOF in terminals when constructing TranslationGrammar.");
      if (t.type() != Symbol::Type::TERMINAL)
        throw std::invalid_argument(
            "Symbol with type other than TERMINAL in terminals when "
            "constructing TranslationGrammar.");
    }

    for (auto& nt : nonterminals_) {
      if (nt.type() != Symbol::Type::NONTERMINAL)
        throw std::invalid_argument(
            "Symbol with type other than NONTERMINAL in nonterminals when "
            "constructing TranslationGrammar.");
    }

    if (starting_symbol_.type() != Symbol::Type::NONTERMINAL)
      throw std::invalid_argument(
          "Starting symbol is not a nonterminal when constructing "
          "TranslationGrammar.");
    if (!is_in(nonterminals_, starting_symbol_))
      throw std::invalid_argument(
          "Starting symbol is not in nonterminals when constructing "
          "TranslationGrammar.");

    for (auto& r : rules_) {
      if (!is_in(nonterminals_, r.nonterminal()))
        throw std::invalid_argument("Rule with production from nonterminal " +
                                    r.nonterminal().name() + ", no such nonterminal.");

      for (auto& s : r.input()) {
        switch (s.type()) {
          case Symbol::Type::TERMINAL:
            if (!is_in(terminals_, s))
              throw std::invalid_argument("Rule with unknown terminal " + s.name() + ".");
            break;
          case Symbol::Type::NONTERMINAL:
            if (!is_in(nonterminals_, s))
              throw std::invalid_argument("Rule with unknown nonterminal " + s.name() + ".");
            break;
          default:
            // should not matter
            break;
        }  // switch
      }    // for all input
    }      // for all rules
    make_set(nonterminals_);
    make_augmented();
    mark_rules();
  }

  ~TranslationGrammar() = default;

  /**
  \brief Returns an empty string of symbols.
  */
  static const vector<Symbol>& EPSILON_STRING() {
    static vector<Symbol> es{};
    return es;
  };

  vector<Symbol>& terminals() { return terminals_; }
  const vector<Symbol>& terminals() const { return terminals_; }

  vector<Symbol>& nonterminals() { return nonterminals_; }
  const vector<Symbol>& nonterminals() const { return nonterminals_; }

  vector<Rule>& rules() { return rules_; }
  const vector<Rule>& rules() const { return rules_; }

  const Rule& starting_rule() const { return rules_.back(); }

  Symbol& starting_symbol() { return starting_symbol_; }
  const Symbol& starting_symbol() const { return starting_symbol_; }

  /**
  \brief Returns a nonterminal's index.

  \param[in] nt Nonterminal Symbol to be searched.

  \returns The index of the nonterminal.

  Undefined behavior if the nonterminal is not in nonterminals_.
  */
  size_t nonterminal_index(const Symbol& nt) const {
    return std::lower_bound(nonterminals_.begin(), nonterminals_.end(), nt) - nonterminals_.begin();
  }
  /**
  \brief Returns a terminal's index.

  \param[in] t Terminal Symbol to be searched.

  \returns The index of the terminal.

  Undefined behavior if the terminal in not in terminals_.
  */
  size_t terminal_index(const Symbol& t) const {
    return std::lower_bound(terminals_.begin(), terminals_.end(), t) - terminals_.begin();
  }

 protected:
  /**
  \brief A sorted set of all input terminals.
  */
  vector<Symbol> terminals_;
  /**
  \brief A sorted set of all input nonterminals.
  */
  vector<Symbol> nonterminals_;
  /**
  \brief An unsorted vector of all rules.
  */
  vector<Rule> rules_;
  /**
  \brief The starting symbol.
  */
  Symbol starting_symbol_;

  void mark_rules() {
    for (size_t i = 0; i < rules_.size(); ++i) {
      rules_[i].id = i;
    }
  }

  /**
   * Transforms a translation grammar into an augmented translation grammar.
   */
  void make_augmented() {
    // create new unique starting symbol
    std::string newStartingName = starting_symbol().name();
    Symbol newStartingNonterminal = Symbol::eof();
    auto it = nonterminals_.begin();
    do {
      newStartingName += '\'';
      newStartingNonterminal = Nonterminal(newStartingName);
      it = std::lower_bound(nonterminals_.begin(), nonterminals_.end(), newStartingNonterminal);
    } while (it != nonterminals_.end() && *it == newStartingNonterminal);
    // insert rule S' -> (S eof, S eof)
    rules_.push_back({newStartingNonterminal, {starting_symbol(), Symbol::eof()}});
    // add S' to nonterminals
    nonterminals_.push_back(newStartingNonterminal);
    terminals_.push_back(Symbol::eof());
    make_set(terminals_);
    make_set(nonterminals_);
    // make S' the new starting symbol
    starting_symbol_ = newStartingNonterminal;
  }
};
}  // namespace ctf
#endif
/*** End of file translation_grammar.hpp ***/