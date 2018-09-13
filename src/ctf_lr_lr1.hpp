#ifndef CTF_LR_LR1_HPP
#define CTF_LR_LR1_HPP

#include "ctf_base.hpp"
#include "ctf_table_sets.hpp"
#include "ctf_translation_grammar.hpp"

namespace ctf::lr1 {
class LR1Item {
 public:
  using Rule = TranslationGrammar::Rule;

  LR1Item(const Rule& rule, size_t mark, const Symbol& lookahead)
      : rule_(&rule), mark_(mark), lookahead_(&lookahead) {}

  const set<LR1Item>& closure(const TranslationGrammar& grammar,
                              const empty_t& empty,
                              const first_t& first) const {
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

    set<LR1Item> items{closure_};
    set<LR1Item> newItems;
    while (!items.empty()) {
      // expand all new items
      for (auto&& item : items) {
        const auto& input = item.rule().input();
        if (item.mark() != input.size() &&
            input[item.mark()].type() == Symbol::Type::NONTERMINAL) {
          const auto& nonterminal = input[item.mark()];

          for (auto&& rule : grammar.rules()) {
            if (rule.nonterminal() == nonterminal) {
              // all symbols after the dotted nonterminal
              vector<Symbol> lookahead{rule.input().begin() + item.mark() + 1,
                                       rule.input().end()};
              lookahead.push_back(item.lookahead());
              for (auto&& symbol :
                   string_first(grammar, empty, first, lookahead)) {
                newItems.insert({rule, 0, symbol});
                closure_.insert({rule, 0, symbol});
              }
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
  const Symbol& lookahead() const noexcept { return *lookahead_; }

  friend bool operator<(const LR1Item& lhs, const LR1Item& rhs) {
    return *lhs.rule_ < *rhs.rule_ ||
           (*lhs.rule_ == *rhs.rule_ &&
            ((lhs.mark_ < rhs.mark_) ||
             (lhs.mark_ == rhs.mark_ && *lhs_.lookahead_ < *rhs.lookahead_)));
  }

  friend bool operator==(const LR1Item& lhs, const LR1Item& rhs) {
    return lhs.mark_ == rhs.mark_ &&
           (lhs.rule_ == rhs.rule_ || *lhs.rule_ == *rhs.rule_) &&
           (lhs.lookahead_ == rhs.lookahead_ ||
            *lhs.lookahead_ == *rhs.lookahead_);
  }

 private:
  const Rule* rule_;
  size_t mark_;
  const Symbol* lookahead_;

  mutable set<LR1Item> closure_;
};

inline unordered_map<Symbol, set<LR1Item>> symbol_skip_closures(
    const TranslationGrammar& grammar,
    const empty_t& empty,
    const first_t& first,
    const set<LR1Item>& state) {
  unordered_map<Symbol, set<LR1Item>> result;

  for (auto&& item : state) {
    if (item.mark() == item.rule().input().size()) {
      continue;
    }
    auto&& symbol = item.rule().input()[item.mark()];
    LR1Item newItem{item.rule(), item.mark() + 1, item.lookahead()};
    result[symbol] =
        set_union(result[symbol], newItem.closure(grammar, empty, first));
  }
  return result;
}

}  // namespace ctf::lr1
#endif