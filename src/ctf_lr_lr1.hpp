#ifndef CTF_LR_LR1_HPP
#define CTF_LR_LR1_HPP

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
  // pair of (state, item)

  Item(const LR0Item& item) : item_(item) {}
  Item(LR0Item&& item) : item_(item) {}

  Item(const LR0Item& item,
       const vector_set<LookaheadSource>& lookaheads,
       const vector_set<Symbol>& generatedLookaheads = {})
      : item_(item), lookaheads_(lookaheads), generatedLookaheads_(generatedLookaheads) {}

  Item(const LR0Item& item,
       const vector_set<LookaheadSource>& lookaheads,
       vector_set<Symbol>&& generatedLookaheads)
      : item_(item), lookaheads_(lookaheads), generatedLookaheads_(generatedLookaheads) {}

  Item(const Item& item) = default;
  Item(Item&& item) = default;

  Item& operator=(const Item& other) = default;
  Item& operator=(Item&& other) = default;

  const Rule& rule() const noexcept { return item_.rule(); }

  size_t mark() const noexcept { return item_.mark(); }

  const LR0Item& lr0_item() const& noexcept { return item_; }

  LR0Item&& lr0_item() && noexcept { return std::move(item_); }

  vector_set<Symbol>& generated_lookaheads() & noexcept { return generatedLookaheads_; }
  const vector_set<Symbol>& generated_lookaheads() const& noexcept { return generatedLookaheads_; }
  vector_set<Symbol>&& generated_lookaheads() && noexcept {
    return std::move(generatedLookaheads_);
  }

  vector_set<LookaheadSource>& lookaheads() & noexcept { return lookaheads_; }
  const vector_set<LookaheadSource>& lookaheads() const& noexcept { return lookaheads_; }
  vector_set<LookaheadSource>&& lookaheads() && noexcept { return std::move(lookaheads_); }

  bool reduce() const noexcept { return item_.reduce(); }
  bool has_next() const noexcept { return item_.has_next(); }
  Item next(const LookaheadSource& las) const {
    vector_set<LookaheadSource> lookaheads;
    if ((mark() == 0 && !generated_lookaheads().empty()) || mark() == 1) {
      lookaheads.insert(las);
    } else {
      lookaheads = lookaheads_;
    }
    return Item(item_.next(), lookaheads);
  }

  friend bool operator<(const Item& lhs, const Item& rhs) { return lhs.item_ < rhs.item_; }

  friend bool operator==(const Item& lhs, const Item& rhs) { return lhs.item_ == rhs.item_; }

  string to_string() const {
    using namespace std::literals;
    string result = "["s + item_.to_string() + ", {";
    for (auto&& symbol : generated_lookaheads()) {
      result += ' ';
      result += symbol.to_string();
    }
    if (!lookaheads().empty()) {
      result += " }, {";
      for (auto&& source : lookaheads()) {
        result += ' ';
        result += source.to_string();
      }
    }
    result += " }]";
    return result;
  }

  explicit operator string() const { return to_string(); }

 private:
  LR0Item item_;
  vector_set<LookaheadSource> lookaheads_;
  vector_set<Symbol> generatedLookaheads_;
};

inline tuple<vector_set<Symbol>, bool> first(const TranslationGrammar& grammar,
                                             const empty_t& empty,
                                             const first_t& first,
                                             const vector<Symbol>& symbols) {
  using Type = Symbol::Type;
  vector_set<Symbol> result;
  for (auto&& symbol : symbols) {
    switch (symbol.type()) {
      case Type::TERMINAL:
      case Type::EOI:
        result.insert(symbol);
        return {result, false};
      case Type::NONTERMINAL: {
        size_t i = grammar.nonterminal_index(symbol);
        result = set_union(result, first[i]);
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
        auto [generatedLookaheads, propagateLookahead] = first(grammar, e, f, followingSymbols);
        vector_set<LookaheadSource> propagatedLookaheads;
        if (propagateLookahead) {
          propagatedLookaheads = item.lookaheads();
          generatedLookaheads = set_union(generatedLookaheads, item.generated_lookaheads());
        }
        // TODO optimization point
        for (auto&& rule : grammar.rules()) {
          if (rule.nonterminal() == nonterminal) {
            Item newItem{{rule, 0}, propagatedLookaheads, generatedLookaheads};
            auto it = closure.find(newItem);
            if (it != closure.end()) {
              size_t originalSize = it->lookaheads().size() + it->generated_lookaheads().size();
              it->lookaheads() = set_union(it->lookaheads(), propagatedLookaheads);
              it->generated_lookaheads() =
                  set_union(it->generated_lookaheads(), generatedLookaheads);
              size_t newSize = it->lookaheads().size() + it->generated_lookaheads().size();
              if (newSize > originalSize) {
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

inline unordered_map<Symbol, vector_set<Item>> symbol_skip_closures(const vector_set<Item>& state,
                                                                    const size_t id) {
  unordered_map<Symbol, vector_set<Item>> result;

  for (size_t i = 0; i < state.size(); ++i) {
    auto&& item = state[i];
    if (!item.has_next()) {
      continue;
    }
    auto&& symbol = item.rule().input()[item.mark()];
    Item newItem{item.next({id, i})};
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
        : id_(id), items_(closure(kernel, grammar, empty, first)) {
      // we can only merge states when the kernel only contains rules in the form A -> x.Y
      for (auto&& item : kernel) {
        if (item.mark() == 1) {
          mergable_ = true;
          break;
        }
      }
    }

    size_t id() const noexcept { return id_; }
    vector_set<Item>& items() noexcept { return items_; }
    const vector_set<Item>& items() const noexcept { return items_; }

    unordered_map<Symbol, size_t>& transitions() noexcept { return transitions_; }
    const unordered_map<Symbol, size_t>& transitions() const noexcept { return transitions_; }

    bool mergable() const noexcept { return mergable_; }

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
    size_t id_;
    // closure of kernel
    vector_set<Item> items_;

    // state transitions
    unordered_map<Symbol, size_t> transitions_;

    bool mergable_ = false;
  };

  StateMachine(const TranslationGrammar& grammar)
      : StateMachine(grammar, create_empty(grammar), create_first(grammar, empty_)) {
    // initial item S' -> .S$
    insert_state({Item({grammar.starting_rule(), 0}, {}, {Symbol::eof()})});
    // recursively expand all states: dfs
    expand_state(0);
    // push all lookaheads to their items
    finalize_lookaheads();
  }

  virtual ~StateMachine() = default;

  const vector<State>& states() const noexcept { return states_; }

 protected:
  const TranslationGrammar* grammar_;
  empty_t empty_;
  first_t first_;
  vector<State> states_;

  map<vector_set<Item>, vector<size_t>> kernelMap_;

  StateMachine(const TranslationGrammar& grammar, empty_t empty, first_t first)
      : grammar_(&grammar), empty_(std::move(empty)), first_(std::move(first)) {}

  const TranslationGrammar& grammar() const noexcept { return *grammar_; }

  tuple<size_t, bool> insert_state(const vector_set<Item>& kernel) {
    size_t i = states_.size();
    State newState(i, kernel, grammar(), empty_, first_);

    if (newState.mergable()) {
      // try to merge with another state
      auto& kernelStates = kernelMap_[kernel];
      if (kernelStates.empty()) {
        // new mergable kernel
        kernelStates.push_back(i);

        states_.push_back(std::move(newState));
        return {i, true};
      } else {
        // check existing states with this kernel
        auto [other, merged] = merge(kernelStates, newState);
        if (merged) {
          return {other, false};
        }
        // no matching state found, insert as new
        kernelStates.push_back(i);
        states_.push_back(std::move(newState));
        return {i, true};
      }
    } else {
      // not a mergable kernel, just insert
      states_.push_back(std::move(newState));
      return {i, true};
    }
  }

  virtual tuple<size_t, bool> merge(const std::vector<size_t>& isocores, const State& newState) {
    auto&& newLookaheads = lookaheads(newState);
    for (auto other : isocores) {
      auto& existing = states_[other];
      if (lookaheads(existing) == newLookaheads) {
        // we can keep the same lookahead relations in the existing state
        // no modification necessary
        return {other, true};
      }
    }
    return {0, false};
  }

  vector<vector_set<Symbol>> lookaheads(const State& state) {
    // get all back references
    unordered_map<LookaheadSource, vector_set<Symbol>> lookaheadMap;
    vector<vector_set<Symbol>> result;

    // get all sources
    for (auto&& item : state.items()) {
      for (auto&& source : item.lookaheads()) {
        auto it = lookaheadMap.find(source);
        if (it == lookaheadMap.end()) {
          // lookahead source not resolved
          lookahead_lookup(source, lookaheadMap);
          it = lookaheadMap.find(source);
        }
        result.push_back(it->second);
      }
    }
    return result;
  }

  void lookahead_lookup(const LookaheadSource& source,
                        unordered_map<LookaheadSource, vector_set<Symbol>>& lookaheadMap) {
    const auto& state = states_[source.state];
    // stop infinite loops
    lookaheadMap[source] = {};
    // get all sources
    auto&& item = state.items()[source.item];
    vector_set<Symbol> symbols(item.generated_lookaheads());
    for (auto&& nextSource : item.lookaheads()) {
      auto it = lookaheadMap.find(nextSource);
      if (it == lookaheadMap.end()) {
        // recursive source not resolved yet
        lookahead_lookup(nextSource, lookaheadMap);
        it = lookaheadMap.find(nextSource);
      }
      symbols = set_union(symbols, it->second);
    }
    lookaheadMap[source] = std::move(symbols);
  }

  void expand_state(size_t i) {
    for (auto&& [symbol, kernel] : symbol_skip_closures(states_[i].items(), i)) {
      auto [id, inserted] = insert_state(kernel);
      states_[i].transitions()[symbol] = id;
      // new inserted state
      if (inserted) {
        expand_state(id);
      }
    }
  }

  // goes through all relative lookaheads and changes them to generated lookaheads
  void finalize_lookaheads() {
    // a single map for all lookaheads
    unordered_map<LookaheadSource, vector_set<Symbol>> lookaheadMap;
    for (auto& state : states_) {
      for (auto& item : state.items()) {
        for (auto&& source : item.lookaheads()) {
          auto it = lookaheadMap.find(source);
          if (it == lookaheadMap.end()) {
            // lookahead source not resolved
            lookahead_lookup(source, lookaheadMap);
            it = lookaheadMap.find(source);
          }
          item.generated_lookaheads() = set_union(item.generated_lookaheads(), it->second);
          // TODO set lookup in map
        }
        // remove all relative lookaheads from this item
        item.lookaheads().clear();
        item.lookaheads().shrink_to_fit();
      }
    }
  }
};

}  // namespace ctf::lr1
#endif