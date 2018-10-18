#ifndef CTF_LR_LR0_HPP
#define CTF_LR_LR0_HPP

#include "ctf_base.hpp"
#include "ctf_table_sets.hpp"
#include "ctf_translation_grammar.hpp"

namespace ctf::lr0 {

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

using State = set<Item>;

}  // namespace ctf::lr0

namespace ctf {
class LR0StateMachine {
 public:
  explicit LR0StateMachine(const TranslationGrammar& grammar) {
    auto& startingRule = grammar.augmented_starting_rule();

    states_.push_back(lr0::Item{startingRule, 0}.closure(grammar));
    transitions_.push_back({});

    for (size_t i = 0; i < states_.size(); ++i) {
      auto&& state = states_[i];
      // get all nonempty closures
      auto&& statetransitions_ = next_states_(grammar, state);
      // add transitions_
      for (auto&& transitionPair : statetransitions_) {
        const Symbol& symbol = transitionPair.first;
        auto&& state = transitionPair.second;
        size_t j = insert_state(state);
        transitions_[i][symbol] = j;
      }
    }
  }

  const vector<lr0::State>& states() const { return states_; }
  const vector<unordered_map<Symbol, size_t>>& transitions() const {
    return transitions_;
  }

 protected:
  unordered_map<Symbol, lr0::State> next_states_(
      const TranslationGrammar& grammar, const lr0::State& state) {
    unordered_map<Symbol, lr0::State> result;

    for (auto&& item : state) {
      if (item.mark() == item.rule().input().size()) {
        continue;
      }
      auto&& symbol = item.rule().input()[item.mark()];
      lr0::Item newItem{item.rule(), item.mark() + 1};
      result[symbol] = set_union(result[symbol], newItem.closure(grammar));
    }
    return result;
  }

  size_t insert_state(const lr0::State& state) {
    auto it = std::find(states_.begin(), states_.end(), state);
    if (it == states_.end()) {
      // insert
      states_.push_back(state);
      transitions_.push_back({});
      return states_.size() - 1;
    } else {
      return it - states_.begin();
    }
  }

  vector<lr0::State> states_;
  vector<unordered_map<Symbol, size_t>> transitions_;
};
}  // namespace ctf

#endif