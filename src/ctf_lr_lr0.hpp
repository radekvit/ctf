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

  string to_string() const {
    string result = rule().nonterminal().to_string() + " -> (";
    size_t i = 0;
    for (; i < mark(); ++i) {
      result += ' ';
      result += rule().input()[i].to_string();
    }
    result += " .";
    for (; i < rule().input().size(); ++i) {
      result += ' ';
      result += rule().input()[i].to_string();
    }
    result += " )";
    return result;
  }

  explicit operator string() const { return to_string(); }

 private:
  const Rule* _rule;
  size_t _mark;
};

using State = vector_set<Item>;

}  // namespace ctf::lr0

namespace ctf {
class LR0StateMachine {
 public:
  explicit LR0StateMachine(const TranslationGrammar& grammar) {
    auto& startingRule = grammar.starting_rule();

    _states.push_back(lr0::Item{startingRule, 0}.closure(grammar));
    _transitions.push_back({});

    for (size_t i = 0; i < _states.size(); ++i) {
      auto&& state = _states[i];
      // get all nonempty closures
      auto&& state_transitions = next_states(grammar, state);
      // add _transitions
      for (auto&& transitionPair : state_transitions) {
        const Symbol& symbol = transitionPair.first;
        auto&& state = transitionPair.second;
        size_t j = insert_state(state);
        _transitions[i][symbol] = j;
      }
    }
  }

  const vector<lr0::State>& states() const { return _states; }
  const vector<unordered_map<Symbol, size_t>>& transitions() const { return _transitions; }

 protected:
  unordered_map<Symbol, lr0::State> next_states(const TranslationGrammar& grammar,
                                                const lr0::State& state) {
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
    auto it = std::find(_states.begin(), _states.end(), state);
    if (it == _states.end()) {
      // insert
      _states.push_back(state);
      _transitions.push_back({});
      return _states.size() - 1;
    } else {
      return it - _states.begin();
    }
  }

  vector<lr0::State> _states;
  vector<unordered_map<Symbol, size_t>> _transitions;
};
}  // namespace ctf

#endif