#ifndef CTF_LR_LR0_HPP
#define CTF_LR_LR0_HPP

#include "ctf_base.hpp"
#include "ctf_table_sets.hpp"
#include "ctf_translation_grammar.hpp"

namespace ctf::lr0 {

class Item {
 public:
  using Rule = TranslationGrammar::Rule;

  Item(const Rule& rule, size_t mark) : _rule(&rule), _mark(mark) {}
  Item(const Item& item) = default;
  Item(Item&& item) = default;

  Item& operator=(const Item& other) = default;
  Item& operator=(Item&& other) = default;

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
      for (auto&& item : items) {
        const auto& input = item.rule().input();
        if (item.mark() != input.size() && input[item.mark()].nonterminal() &&
            !expandedNonterminals.contains(input[item.mark()])) {
          const auto& nonterminal = input[item.mark()];
          expandedNonterminals.insert(nonterminal);

          for (auto&& rule : grammar.rules()) {
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

  const Rule& rule() const noexcept { return *_rule; }
  size_t mark() const noexcept { return _mark; }

  bool reduce() const noexcept { return mark() == rule().input().size(); }
  bool has_next() const noexcept { return mark() < rule().input().size(); }

  Item next() const noexcept { return Item(rule(), mark() + 1); }

  // only valid comparing items with the same rule sources
  friend bool operator<(const Item& lhs, const Item& rhs) {
    return lhs.mark() > rhs.mark() || (lhs.mark() == rhs.mark() && lhs._rule < rhs._rule);
  }

  friend bool operator==(const Item& lhs, const Item& rhs) {
    return lhs._mark == rhs._mark && lhs._rule == rhs._rule;
  }

  string to_string(symbol_string_fn to_str = ctf::to_string) const {
    string result = to_str(rule().nonterminal()) + " -> (";
    size_t i = 0;
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

  explicit operator string() const { return to_string(); }

 private:
  const Rule* _rule;
  size_t _mark;
};

}  // namespace ctf::lr0

#endif