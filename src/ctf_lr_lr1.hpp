#ifndef CTF_LR_LR1_HPP
#define CTF_LR_LR1_HPP

#include <iostream>
#include "ctf_base.hpp"
#include "ctf_lr_lr0.hpp"
#include "ctf_table_sets.hpp"
#include "ctf_translation_grammar.hpp"

namespace ctf::lr1 {

struct LookaheadSource {
  size_t state = 0;
  size_t item = 0;

  friend bool operator<(const LookaheadSource& lhs, const LookaheadSource& rhs) {
    return lhs.state < rhs.state || (lhs.state == rhs.state && lhs.item < rhs.item);
  }

  friend bool operator==(const LookaheadSource& lhs, const LookaheadSource& rhs) {
    return lhs.state == rhs.state && lhs.item == rhs.item;
  }

  string to_string() const {
    using namespace std::literals;
    return "("s + std::to_string(state) + ", " + std::to_string(item) + ")";
  }
  explicit operator string() const { return to_string(); }
};

using LookaheadSet = TerminalSet;
}  // namespace ctf::lr1

namespace std {
template <>
struct hash<ctf::lr1::LookaheadSource> {
  using argument_type = ctf::lr1::LookaheadSource;
  using result_type = size_t;
  result_type operator()(argument_type const& s) const noexcept {
    return std::hash<size_t>{}((s.state << 2) * s.item);
  }
};
}  // namespace std

namespace ctf::lr1 {
class Item {
 public:
  using Rule = TranslationGrammar::Rule;
  using LR0Item = ctf::lr0::Item;

  Item(const LR0Item& item, const TranslationGrammar& tg)
    : _item(item), _generatedLookaheads(tg.terminals()) {}
  Item(LR0Item&& item, const TranslationGrammar& tg)
    : _item(item), _generatedLookaheads(tg.terminals()) {}

  Item(const LR0Item& item,
       const vector_set<LookaheadSource>& lookaheads,
       const LookaheadSet& generatedLookaheads)
    : _item(item), _lookaheads(lookaheads), _generatedLookaheads(generatedLookaheads) {}

  Item(const LR0Item& item,
       const vector_set<LookaheadSource>& lookaheads,
       LookaheadSet&& generatedLookaheads)
    : _item(item), _lookaheads(lookaheads), _generatedLookaheads(generatedLookaheads) {}

  Item(const Item& item) = default;
  Item(Item&& item) = default;

  Item& operator=(const Item& other) = default;
  Item& operator=(Item&& other) = default;

  const Rule& rule() const noexcept { return _item.rule(); }

  size_t mark() const noexcept { return _item.mark(); }

  const LR0Item& lr0_item() const& noexcept { return _item; }

  LR0Item&& lr0_item() && noexcept { return std::move(_item); }

  LookaheadSet& lookaheads() & noexcept { return _generatedLookaheads; }
  const LookaheadSet& lookaheads() const& noexcept { return _generatedLookaheads; }
  LookaheadSet&& lookaheads() && noexcept { return std::move(_generatedLookaheads); }

  vector_set<LookaheadSource>& lookahead_sources() & noexcept { return _lookaheads; }
  const vector_set<LookaheadSource>& lookahead_sources() const& noexcept { return _lookaheads; }
  vector_set<LookaheadSource>&& lookahead_sources() && noexcept { return std::move(_lookaheads); }

  bool reduce() const noexcept { return _item.reduce(); }
  bool has_next() const noexcept { return _item.has_next(); }
  Item next(const LookaheadSource& las) const {
    vector_set<LookaheadSource> lookaheads;
#if 0
    if (mark() < 2) {
      lookaheads.insert(las);
    } else {
      lookaheads = _lookaheads;
    }
#else
    lookaheads.insert(las);
#endif
    return Item(_item.next(), lookaheads, LookaheadSet(_generatedLookaheads.capacity()));
  }

  friend bool operator<(const Item& lhs, const Item& rhs) { return lhs._item < rhs._item; }

  friend bool operator==(const Item& lhs, const Item& rhs) { return lhs._item == rhs._item; }

  string to_string() const {
    using namespace std::literals;
    string result = "["s + _item.to_string() + ", {";
    for (auto&& symbol : lookaheads().symbols()) {
      result += ' ';
      result += symbol.to_string();
    }
    if (!lookahead_sources().empty()) {
      result += " }, {";
      for (auto&& source : lookahead_sources()) {
        result += ' ';
        result += source.to_string();
      }
    }
    result += " }]";
    return result;
  }

  explicit operator string() const { return to_string(); }

 private:
  LR0Item _item;
  vector_set<LookaheadSource> _lookaheads;
  LookaheadSet _generatedLookaheads;
};

struct FirstResult {
  LookaheadSet symbols;
  bool empty;
};

inline FirstResult first(const vector<Symbol>& symbols,
                         const empty_t& empty,
                         const first_t& first,
                         const TranslationGrammar& tg) {
  using Type = Symbol::Type;
  LookaheadSet result(tg.terminals());
  for (auto&& symbol : symbols) {
    switch (symbol.type()) {
      case Type::TERMINAL:
      case Type::EOI:
        result.insert(symbol);
        return {result, false};
      case Type::NONTERMINAL: {
        size_t i = symbol.id();
        result |= first[i];
        if (!empty[i]) {
          return {result, false};
        }
        break;
      }
    }
  }
  return {result, true};
}

inline vector_set<Item> closure(vector_set<Item> items,
                                const TranslationGrammar& grammar,
                                const empty_t& e,
                                const first_t& f) {
  vector_set<Item> closure{items};

  vector_set<Item> newItems;
  while (!items.empty()) {
    // expand all new items
    for (auto&& item : items) {
      const auto& input = item.rule().input();
      if (!item.reduce() && input[item.mark()].nonterminal()) {
        const auto& nonterminal = input[item.mark()];
        // all symbols after the dotted nonterminal
        vector<Symbol> followingSymbols;
        if (!item.reduce()) {
          followingSymbols = {input.begin() + item.mark() + 1, input.end()};
        }
        auto [generatedLookaheads, propagateLookahead] = first(followingSymbols, e, f, grammar);
        vector_set<LookaheadSource> propagatedLookaheads;
        if (propagateLookahead) {
          propagatedLookaheads = item.lookahead_sources();
          generatedLookaheads |= item.lookaheads();
        }
        // TODO optimization point
        for (auto&& rule : grammar.rules()) {
          if (rule.nonterminal() == nonterminal) {
            Item newItem({rule, 0}, propagatedLookaheads, generatedLookaheads);
            auto it = closure.find(newItem);
            if (it != closure.end()) {
              size_t originalSize = it->lookahead_sources().size();
              it->lookahead_sources() = set_union(it->lookahead_sources(), propagatedLookaheads);
              bool addedGenerated = it->lookaheads().set_union(generatedLookaheads);
              size_t newSize = it->lookahead_sources().size();
              if (newSize > originalSize || addedGenerated) {
                // TODO would it ultimately be faster to create in-state lookahead sources instead?
                newItems.erase(*it);
                newItems.insert(*it);
              }
            } else {
              newItems.insert(newItem);
              closure.insert(std::move(newItem));
            }
          }
        }
      }
    }
    items.swap(newItems);
    newItems.clear();
  }
  return closure;
}

inline unordered_map<Symbol, vector_set<Item>> symbol_skip_kernels(const vector_set<Item>& state,
                                                                   const size_t id) {
  unordered_map<Symbol, vector_set<Item>> result;

  for (size_t i = 0; i < state.size(); ++i) {
    auto& item = state[i];
    if (item.reduce()) {
      continue;
    }
    auto& symbol = item.rule().input()[item.mark()];
    if (symbol == Symbol::eof() || !item.has_next()) {
      continue;
    }
    Item newItem(item.next({id, i}));
    result[symbol] = set_union(result[symbol], {newItem});
  }
  return result;
}

// lookahead relation based machine
class StateMachine {
 public:
  using Item = ctf::lr1::Item;
  class State {
   public:
    State(size_t id,
          const vector_set<Item>& kernel,
          const TranslationGrammar& grammar,
          const empty_t& empty,
          const first_t& first)
      : _id(id), _items(closure(kernel, grammar, empty, first)) {
      // we can only merge states when the kernel contains a rule in the form A -> x.Y
      for (auto&& item : _items) {
        if (item.reduce()) {
          _reduce = true;
          break;
        }
      }
    }

    size_t id() const noexcept { return _id; }
    vector_set<Item>& items() noexcept { return _items; }
    const vector_set<Item>& items() const noexcept { return _items; }

    unordered_map<Symbol, size_t>& transitions() noexcept { return _transitions; }
    const unordered_map<Symbol, size_t>& transitions() const noexcept { return _transitions; }

    bool has_reduce() const noexcept { return _reduce; }

    string to_string() const {
      string result = std::to_string(id()) + ": {\n";
      for (auto&& item : items()) {
        result += '\t';
        result += item.to_string() + '\n';
      }
      result += "\t-----\n";
      for (auto&& [symbol, next] : transitions()) {
        result += '\t';
        result += symbol.to_string() + ": " + std::to_string(next) + '\n';
      }
      result += "}\n";
      return result;
    }

    explicit operator string() const { return to_string(); }

   private:
    // state index
    size_t _id;
    // closure of kernel
    vector_set<Item> _items;

    // state transitions
    unordered_map<Symbol, size_t> _transitions;

    bool _reduce = false;
  };

  StateMachine(const TranslationGrammar& grammar)
    : StateMachine(grammar, create_empty(grammar), create_first(grammar, _empty)) {
    // initial item S' -> .S$
    insert_state(
      {Item({grammar.starting_rule(), 0}, {}, LookaheadSet(grammar.terminals(), {Symbol::eof()}))});
    // recursively expand all states: dfs
    expand_state(0);
    // push all lookaheads to their items
    finalize_lookaheads();
  }

  virtual ~StateMachine() = default;

  const vector<State>& states() const noexcept { return _states; }

 protected:
  const TranslationGrammar* _grammar;
  empty_t _empty;
  first_t _first;
  vector<State> _states;

  map<vector_set<Item>, vector<size_t>> _kernelMap;

  struct InsertResult {
    size_t state;
    bool insertedNew;
  };

  struct MergeResult {
    size_t state;
    bool merge;
  };

  StateMachine(const TranslationGrammar& grammar, empty_t empty, first_t first)
    : _grammar(&grammar), _empty(std::move(empty)), _first(std::move(first)) {}

  const TranslationGrammar& grammar() const noexcept { return *_grammar; }

  InsertResult insert_state(const vector_set<Item>& kernel) {
    size_t i = _states.size();
    State newState(i, kernel, grammar(), _empty, _first);

    // try to merge with another state
    auto& kernelStates = _kernelMap[kernel];
    if (kernelStates.empty()) {
      // new mergable kernel
      kernelStates.push_back(i);

      _states.push_back(std::move(newState));
      return {i, true};
    } else {
      // check existing states with this kernel
      auto [other, merged] = merge(kernelStates, newState);
      if (merged) {
        return {other, false};
      }
      // no matching state found, insert as new
      kernelStates.push_back(i);
    }
    // insert new state
    _states.push_back(std::move(newState));
    return {i, true};
  }

  virtual MergeResult merge(const std::vector<size_t>& isocores, const State& newState) {
    auto newLookaheads = lookaheads(newState);
    for (auto other : isocores) {
      auto& existing = _states[other];
      auto lookahead = lookaheads(existing);
      // we can insert the lookaheads to the existing
      if (lookahead == newLookaheads) {
        // we can keep the same lookahead relations in the existing state
        // no modification necessary
        return {other, true};
      }
    }
    return {0, false};
  }

  vector<LookaheadSet> lookaheads(const State& state) {
    // get all back references
    unordered_map<LookaheadSource, LookaheadSet> lookaheadMap;
    vector<LookaheadSet> result;

    // get all sources
    for (auto&& item : state.items()) {
      result.push_back(TerminalSet(grammar().terminals()));
      for (auto&& source : item.lookahead_sources()) {
        auto it = lookaheadMap.find(source);
        if (it == lookaheadMap.end()) {
          // lookahead source not resolved
          lookahead_lookup(source, lookaheadMap);
          it = lookaheadMap.find(source);
        }
        result.back() |= it->second;
      }
    }
    return result;
  }

  void lookahead_lookup(const LookaheadSource& source,
                        unordered_map<LookaheadSource, LookaheadSet>& lookaheadMap) {
    const auto& state = _states[source.state];
    // stop infinite loops
    lookaheadMap.insert_or_assign(source, LookaheadSet(grammar().terminals()));
    // get all sources
    auto&& item = state.items()[source.item];
    LookaheadSet symbols(item.lookaheads());
    for (auto&& nextSource : item.lookahead_sources()) {
      auto it = lookaheadMap.find(nextSource);
      if (it == lookaheadMap.end()) {
        // recursive source not resolved yet
        lookahead_lookup(nextSource, lookaheadMap);
        it = lookaheadMap.find(nextSource);
      }
      symbols |= it->second;
    }
    lookaheadMap.insert_or_assign(source, std::move(symbols));
  }

  void expand_state(size_t i) {
    for (auto&& [symbol, kernel] : symbol_skip_kernels(_states[i].items(), i)) {
      auto [id, inserted] = insert_state(kernel);
      _states[i].transitions()[symbol] = id;
      // new inserted state
      if (inserted) {
        expand_state(id);
      }
    }
  }

  // goes through all relative lookaheads and changes them to generated lookaheads
  void finalize_lookaheads() {
    // a single map for all lookaheads
    for (auto& state : _states) {
      std::cout << "\n" << state.to_string();
      unordered_map<LookaheadSource, LookaheadSet> lookaheadMap;
      for (auto& item : state.items()) {
        for (auto&& source : item.lookahead_sources()) {
          auto it = lookaheadMap.find(source);
          if (it == lookaheadMap.end()) {
            // lookahead source not resolved
            lookahead_lookup(source, lookaheadMap);
            it = lookaheadMap.find(source);
          }
          item.lookaheads() |= it->second;
          // TODO set lookup in map
        }
        // remove all relative lookaheads from this item
        item.lookahead_sources().clear();
        item.lookahead_sources().shrink_to_fit();
      }
    }
  }
};

}  // namespace ctf::lr1
#endif