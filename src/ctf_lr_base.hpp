#ifndef CTF_LR_BASE_H
#define CTF_LR_BASE_H

#include "ctf_base.hpp"
#include "ctf_table_sets.hpp"
#include "ctf_translation_grammar.hpp"

namespace ctf {

class Item {
 public:
  using Rule = TranslationGrammar::Rule;

  Item(const Rule& rule, size_t mark) : rule_(&rule), mark_(mark) {}

  const set<Item>& closure(const TranslationGrammar& grammar) const {
    // already calculated closure
    if (closure_.size() != 0) {
      return closure_;
    }
    closure_ = {*this};
    // item with the mark at the last position or a mark before the
    if (mark_ == rule_->input().size() ||
        rule_->input()[mark_].type() == Symbol::Type::TERMINAL) {
      return closure_;
    }

    set<Symbol> expandedNonterminals;
    set<Item> items{closure_};
    set<Item> newItems;
    while (!items.empty()) {
      // expand all new items for nonterminals we haven't expanded yet
      for (auto&& item : items) {
        const auto& input = item.rule().input();
        if (item.mark() != input.size() &&
            input[item.mark()].type() == Symbol::Type::NONTERMINAL &&
            !expandedNonterminals.contains(input[item.mark()])) {
          const auto& nonterminal = input[item.mark()];
          expandedNonterminals.insert(nonterminal);

          for (auto&& rule : grammar.rules()) {
            if (rule.nonterminal() == nonterminal) {
              newItems.insert({rule, 0});
              closure_.insert({rule, 0});
            }
          }
        }
      }
      items.swap(newItems);
      newItems.clear();
    }
    return closure_;
  }

  void clear_closure() const noexcept { closure_.clear(); }

  const Rule& rule() const noexcept { return *rule_; }
  size_t mark() const noexcept { return mark_; }

  friend bool operator<(const Item& lhs, const Item& rhs) {
    return *lhs.rule_ < *rhs.rule_ ||
           (*lhs.rule_ == *rhs.rule_ && lhs.mark_ < rhs.mark_);
  }

  friend bool operator==(const Item& lhs, const Item& rhs) {
    return lhs.mark_ == rhs.mark_ &&
           (lhs.rule_ == rhs.rule_ || *lhs.rule_ == *rhs.rule_);
  }

 private:
  const Rule* rule_;
  size_t mark_;

  mutable set<Item> closure_;
};

inline unordered_map<Symbol, set<Item>> symbol_skip_closures(
    const TranslationGrammar& grammar, const set<Item>& state) {
  unordered_map<Symbol, set<Item>> result;

  for (auto&& item : state) {
    if (item.mark() == item.rule().input().size()) {
      continue;
    }
    auto&& symbol = item.rule().input()[item.mark()];
    Item newItem{item.rule(), item.mark() + 1};
    result[symbol] = set_union(result[symbol], newItem.closure(grammar));
  }
  return result;
}

}  // namespace ctf

#endif