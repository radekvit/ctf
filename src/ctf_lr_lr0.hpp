/**
\file ctf_lr_lr0.hpp
\brief Defines LR(0) items.
\author Radek Vít
*/
#ifndef CTF_LR_LR0_HPP
#define CTF_LR_LR0_HPP

#include "ctf_base.hpp"
#include "ctf_table_sets.hpp"
#include "ctf_translation_grammar.hpp"

namespace ctf::lr0 {
/**
\brief Represents a LR(0) item. References a rule and contains the marker location.
*/
class Item {
 public:
  using Rule = TranslationGrammar::Rule;
  /**
  \brief Construct an item from a rule and a marker location.
  */
  Item(const Rule& rule, std::size_t mark) : _rule(&rule), _mark(mark) {}
  Item(const Item& item) = default;
  Item(Item&& item) = default;

  Item& operator=(const Item& other) = default;
  Item& operator=(Item&& other) = default;
  /**
  \brief Computes the closure of this LR(0) item.

  \param[in] grammar The translation grammar for this closure.

  \returns A LR(0) closure of this item.
  */
  vector_set<Item> closure(const TranslationGrammar& grammar) const {
    vector_set<Item> closure = {*this};
    // item with the mark at the last position or a mark before the
    if (_mark == _rule->input().size() ||
        _rule->input()[_mark].type() != Symbol::Type::NONTERMINAL) {
      return closure;
    }

    vector_set<Symbol> expandedNonterminals;
    vector_set<Item> items{closure};
    vector_set<Item> newItems;
    while (!items.empty()) {
      // expand all new items for nonterminals we haven't expanded yet
      for (auto& item : items) {
        const auto& input = item.rule().input();
        if (item.mark() != input.size() && input[item.mark()].nonterminal() &&
            !expandedNonterminals.contains(input[item.mark()])) {
          const auto& nonterminal = input[item.mark()];
          expandedNonterminals.insert(nonterminal);

          for (auto& rule : grammar.rules()) {
            if (rule.nonterminal() == nonterminal) {
              newItems.insert({rule, 0});
              closure.insert({rule, 0});
            }
          }
        }
      }
      items.swap(newItems);
      newItems.clear();
    }
    return closure;
  }
  /**
  \brief Returns the represented rule.
  */
  const Rule& rule() const noexcept { return *_rule; }
  /**
  \brief Returns the marked location in the rule's input.
  */
  std::size_t mark() const noexcept { return _mark; }

  /**
  \brief Returns true if this item has the mark at its last position.
  */
  bool reduce() const noexcept { return mark() == rule().input().size(); }
  /**
  \brief Returns false if this item has the mark at its last position.
  */
  bool has_next() const noexcept { return mark() < rule().input().size(); }
  /**
  \brief Returns the item with the mark at the next position.
  */
  Item next() const noexcept { return Item(rule(), mark() + 1); }

  /**
  \brief Lexicographically compares the items. Marks have higher priority.
  */
  friend bool operator<(const Item& lhs, const Item& rhs) {
    return lhs.mark() > rhs.mark() || (lhs.mark() == rhs.mark() && lhs._rule < rhs._rule);
  }
  /**
  \brief Compares the items for identity.
  */
  friend bool operator==(const Item& lhs, const Item& rhs) {
    return lhs._mark == rhs._mark && lhs._rule == rhs._rule;
  }
  /**
  \brief Returns a string representation of the item.

  \param[in] to_str The Symbol string representation function.
  */
  string to_string(symbol_string_fn to_str = ctf::to_string) const {
    string result = to_str(rule().nonterminal()) + " -> (";
    std::size_t i = 0;
    for (; i < mark(); ++i) {
      result += ' ';
      result += to_str(rule().input()[i]);
    }
    result += " .";
    for (; i < rule().input().size(); ++i) {
      result += ' ';
      result += to_str(rule().input()[i]);
    }
    result += " )";
    return result;
  }
  /**
  \brief Returns a string representation of the item.
  */
  explicit operator string() const { return to_string(); }

 private:
  /**
  \brief The grammar rule portion of the item. Is always set to a valid rule.
  */
  const Rule* _rule;
  /**
  \brief The location of the mark. The mark is always before the symbol at that position.
  */
  std::size_t _mark;
};

}  // namespace ctf::lr0

#endif

/*** End of file ctf_lr_lr0.hpp ***/
