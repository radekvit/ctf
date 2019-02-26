#ifndef CTF_LR_IALR_HPP
#define CTF_LR_IALR_HPP

#include "ctf_lr_lr1.hpp"

namespace ctf::ialr {

using LookaheadSource = lr1::LookaheadSource;
using LookaheadSet = lr1::LookaheadSet;
using Item = lr1::Item;

class StateMachine {
 public:
  using Item = ctf::ialr::Item;
  class State {
   public:
    State(size_t id,
          const vector_set<Item>& kernel,
          const TranslationGrammar& grammar,
          const empty_t& empty,
          const first_t& first)
        : id_(id), items_(closure(kernel, grammar, empty, first)), reduceTargets_() {
      // we can only merge states when the kernel only contains rules in the form A -> x.Y
      for (auto&& item : kernel) {
        if (item.mark() == 1) {
          mergable_ = true;
          break;
        }
      }
      for (auto&& item : items_) {
        if (item.reduce()) {
          reduce_ = true;
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
    bool has_reduce() const noexcept { return reduce_; }

    vector_set<size_t>& reduce_targets() noexcept { return reduceTargets_; }
    const vector_set<size_t>& reduce_targets() const noexcept { return reduceTargets_; }

    unordered_map<Symbol, vector_set<size_t>>& conflicts() noexcept { return conflicts_; }
    const unordered_map<Symbol, vector_set<size_t>>& conflicts() const noexcept {
      return conflicts_;
    }

    void set_expanded() noexcept { expanded_ = true; }
    bool expanded() const noexcept { return expanded_; }

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
    // kernel identifier
    size_t id_;
    // closure of kernel
    vector_set<Item> items_;

    // state transitions
    unordered_map<Symbol, size_t> transitions_;

    vector_set<size_t> reduceTargets_;
    // reduce state conflicts
    unordered_map<Symbol, vector_set<size_t>> conflicts_;

    bool mergable_ = false;
    bool reduce_ = false;
    bool expanded_ = false;
  };

  StateMachine(const TranslationGrammar& grammar)
      : StateMachine(grammar, create_empty(grammar), create_first(grammar, empty_)) {
    // initial item S' -> .S$
    insert_state({Item({grammar.starting_rule(), 0}, {}, LookaheadSet(grammar.terminals(), {Symbol::eof()}))});
    // recursively expand all states: dfs
    expand_state(0);
    // merge or generate postoponed states
    handle_postponed();
    // push all lookaheads to their items
    finalize_lookaheads();
  }

  ~StateMachine() = default;

  const vector<State>& states() const noexcept { return states_; }

 protected:
  const TranslationGrammar* grammar_;
  empty_t empty_;
  first_t first_;
  vector<State> states_;

  map<vector_set<Item>, vector<size_t>> kernelMap_;

  struct PostponedTransition {
    size_t state;
    vector<Symbol> transitionSymbols;
  };

  deque<PostponedTransition> postponed_;

  struct InsertResult {
    size_t state;
    bool insertedNew;
    bool insertionPostponed;
  };

  struct MergeResult {
    size_t state;
    bool merge;
  };

  StateMachine(const TranslationGrammar& grammar, empty_t empty, first_t first)
      : grammar_(&grammar), empty_(std::move(empty)), first_(std::move(first)) {}

  const TranslationGrammar& grammar() const noexcept { return *grammar_; }

  InsertResult insert_state(const vector_set<Item>& kernel) {
    // identifier for new state
    size_t i = states_.size();
    State newState(i, kernel, grammar(), empty_, first_);
    if (newState.mergable()) {
      // try to merge with another state
      auto& kernelStates = kernelMap_[kernel];
      if (kernelStates.empty()) {
        // new mergable kernel
        kernelStates.push_back(i);

        states_.push_back(std::move(newState));
        return {i, true, false};
      } else {
        // check existing states with this kernel
        auto [other, merged] = merge(kernelStates, newState);
        if (merged) {
          return {other, false, false};
        }
        // speculative
        else if (other == 1) {
          return {0, false, true};
        }
        // no matching state found, insert as new
        kernelStates.push_back(i);
        states_.push_back(std::move(newState));
        return {i, true, false};
      }
    } else {
      // not a mergable kernel, simply insert
      states_.push_back(std::move(newState));
      return {i, true, false};
    }
  }

  void expand_state(size_t i) {
    // set as reduce target for all lookahead sources
    mark_as_reduce(i, states_[i]);
    vector<Symbol> postponedSymbols;
    // expand all transitions
    for (auto [symbol, kernel] : lr1::symbol_skip_closures(states_[i].items(), i)) {
      auto [id, inserted, postponed] = insert_state(kernel);
      auto& state = states_[i];
      if (postponed) {
        postponedSymbols.push_back(symbol);
        continue;
      }
      state.transitions()[symbol] = id;
      // new inserted state
      if (inserted) {
        expand_state(id);
      }
    }
    if (!postponedSymbols.empty()) {
      postponed_.push_back({i, std::move(postponedSymbols)});
    }
    // mark; this state knows its lookahead reduce targets
    states_[i].set_expanded();
  }

  MergeResult merge(const std::vector<size_t>& isocores, const State& newState) {
    auto newLookaheads = lookaheads(newState);
    vector<vector<LookaheadSet>> existingLookaheads;
    // first try to merge as canonical LR(1)
    for (auto other : isocores) {
      auto& existing = states_[other];
      existingLookaheads.push_back(lookaheads(existing));
      // always merge if lookahead set is the same
      if (existingLookaheads.back() == newLookaheads) {
        // add lookahead sources
        merge_lookaheads(existing, newState);
        merge_reduce_targets(existing, newState);
        return {other, true};
      }
    }
    // try to merge
    size_t speculative = 0;
    for (size_t i = 0; i < isocores.size(); ++i) {
      auto other = isocores[i];
      auto& existing = states_[other];
      // we can resolve this later
      if (!existing.expanded()) {
        speculative = 1;
        continue;
      }
      // try to merge state
      auto [state, success] = ialr_merge(other, newState, existingLookaheads[i], newLookaheads);
      if (success) {
        merge_lookaheads(states_[state], newState);

        return {state, true};
      }
    }
    return {speculative, false};
  }

  vector<LookaheadSet> lookaheads(const State& state) {
    unordered_map<LookaheadSource, LookaheadSet> lookaheadMap;
    // get all back references
    vector<LookaheadSet> result;

    // get all sources
    for (auto&& item : state.items()) {
      result.push_back(item.generated_lookaheads());
      for (auto&& source : item.lookaheads()) {
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

  vector<LookaheadSet> lookaheads(
      const State& state, unordered_map<LookaheadSource, LookaheadSet> lookaheadMap) {
    // get all back references
    vector<LookaheadSet> result;

    // get all sources
    for (size_t i = 0; i < state.items().size(); ++i) {
      auto& item = state.items()[i];
      // first check if the item isn't in the map already
      auto it = lookaheadMap.find({state.id(), i});
      if (it != lookaheadMap.end()) {
        result.push_back(it->second);
        continue;
      }
      result.push_back(item.generated_lookaheads());
      for (auto&& source : item.lookaheads()) {
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
    const auto& state = states_[source.state];
    // stop infinite loops
    lookaheadMap.insert_or_assign(source, LookaheadSet(grammar().terminals()));
    // get all sources
    auto&& item = state.items()[source.item];
    LookaheadSet symbols(item.generated_lookaheads());
    for (auto&& nextSource : item.lookaheads()) {
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

  // try to merge postponed transitions
  void handle_postponed() {
    while (!postponed_.empty()) {
      auto [i, symbols] = postponed_.front();
      postponed_.pop_front();

      auto& state = states_[i];
      auto transitions = lr1::symbol_skip_closures(state.items(), i);
      vector<Symbol> postponedSymbols;
      for (auto symbol : symbols) {
        auto& kernel = transitions[symbol];
        auto [id, inserted, postponed] = insert_state(kernel);
        if (postponed) {
          postponedSymbols.push_back(symbol);
          continue;
        }
        state.transitions()[symbol] = id;
        // new inserted state
        if (inserted) {
          expand_state(id);
        }
      }
      if (!postponedSymbols.empty()) {
        postponed_.push_back({i, std::move(postponedSymbols)});
      }
    }
  }
  // go through all postponed transitions:
  // try to merge with isocores again, else insert and expand
  // in case of new conflict new postponed may be added to the end of queue

  // merge if the result state has the same ammount of conflicts with the same
  // contributions
  // precondition: state is expanded
  MergeResult ialr_merge(size_t state,
                         const State& newState,
                         const vector<LookaheadSet>& existingLookaheads,
                         const vector<LookaheadSet>& newLookaheads) {
    assert(existingLookaheads.size() == newLookaheads.size());

    auto& existing = states_[state];
    unordered_map<LookaheadSource, LookaheadSet> lookaheadMap;
    // for all states that are la targets
    for (auto rstatei : existing.reduce_targets()) {
      auto& rstate = states_[rstatei];
      // 1: construct actions with additional lookaheads
      for (size_t i = 0; i < existingLookaheads.size(); ++i) {
        // set lookaheads
        lookaheadMap.insert_or_assign({state, i}, existingLookaheads[i] | newLookaheads[i]);
      }
      vector<LookaheadSet> mergedLookaheads;
      auto mergedLookups = lookaheads(rstate, lookaheadMap);
      // 2: no conflicts in 1 = OK, skip rest
      auto mergedConflicts = conflicts(rstate, mergedLookups);
      if (mergedConflicts.empty()) {
        continue;
      }
      // 3: get original conflicts
      const auto& oldConflicts = rstate.conflicts();

      if (mergedConflicts != oldConflicts) {
        // additional conflicts if merged, reject
        return {0, false};
      }

      // 4: construct actions with new lookaheads lookaheads
      // construct original lookaheads
      for (size_t i = 0; i < newLookaheads.size(); ++i) {
        // set lookaheads
        lookaheadMap.insert_or_assign({state, i}, newLookaheads[i]);
      }
      auto newLookups = lookaheads(rstate, lookaheadMap);
      auto newConflicts = conflicts(rstate, newLookups);
      if (newConflicts != oldConflicts) {
        // 6: else don't merge
        return {0, false};
      }
      // 5: both conflict contributions are the same: OK
    }
    // if all ok, add lookahead source and reduce targets to source
    merge_lookaheads(existing, newState);
    merge_reduce_targets(existing, newState);
    return {state, true};
  }

  enum class TempAction {
    NONE,
    REDUCE,
    SHIFT,
    CONFLICT,
  };

  // list all reduce contributions to R/R and S/R conflicts
  unordered_map<Symbol, vector_set<size_t>> conflicts(
      State& state, const vector<LookaheadSet>& stateLookaheads) {
    unordered_map<Symbol, vector_set<size_t>> result;
    vector<tuple<TempAction, size_t>> actions(grammar().terminals(), {TempAction::NONE, 0});
    for (size_t i = 0; i < state.items().size(); ++i) {
      auto& item = state.items()[i];
      auto& lookahead = stateLookaheads[i];
      if (item.reduce()) {
        for (auto& symbol : lookahead.symbols()) {
          auto& [action, item] = actions[symbol.id()];
          switch (action) {
            case TempAction::NONE:
              action = TempAction::REDUCE;
              item = i;
              break;
            case TempAction::REDUCE:
              result[symbol] = {item, i};
              action = TempAction::CONFLICT;
              break;
            case TempAction::SHIFT:
              action = TempAction::CONFLICT;
              [[fallthrough]];
            case TempAction::CONFLICT:
              result[symbol].insert(i);
              break;
          }
        }
      } else if (item.rule().input()[item.mark()].terminal()) {
        auto& symbol = item.rule().input()[item.mark()];
        auto& [action, item] = actions[symbol.id()];
        switch (action) {
          case TempAction::NONE:
            action = TempAction::SHIFT;
            break;
          case TempAction::REDUCE:
            action = TempAction::CONFLICT;
            result[symbol].insert(item);
            break;
          case TempAction::SHIFT:
          case TempAction::CONFLICT:
            break;
        }
      }
    }
    return result;
  }

  void merge_lookaheads(State& existing, const State& newState) {
    assert(existing.items().size() == newState.items().size());

    for (size_t i = 0; i < existing.items().size(); ++i) {
      auto& item = existing.items()[i];
      auto& item2 = newState.items()[i];

      item.lookaheads() = set_union(item.lookaheads(), item2.lookaheads());
    }
  }

  void merge_reduce_targets(const State& mergeState, const State& newState) {
    for (auto& item : newState.items()) {
      for (auto reduceState : mergeState.reduce_targets()) {
        mark_source(reduceState, item.lookaheads());
      }
    }
  }

  void mark_as_reduce(size_t i, State& state) {
    if (!state.has_reduce()) {
      return;
    }
    // marks predecessors
    for (auto& item : state.items()) {
      if (!item.reduce()) {
        continue;
      }
      mark_source(i, item.lookaheads());
      // can mark itself if this is a mergable state
      if (state.mergable() && !item.lookaheads().empty()) {
        state.reduce_targets().insert(i);
      }
    }
    // calculate conflicts fir this state
    auto lookaheadSet = lookaheads(state);
    state.conflicts() = conflicts(state, lookaheadSet);
  }

  /**
  \brief Marks all mergable states whose lookahead sets influence reduceState.
  */
  void mark_source(size_t reduceState, const vector_set<LookaheadSource>& lookaheads) {
    for (auto source : lookaheads) {
      auto& state = states_[source.state];
      // only mark transitional nodes
      const auto& la = state.items()[source.item].lookaheads();
      if (!la.empty() && state.reduce_targets().insert(reduceState).inserted) {
        // recursively mark its sources
        mark_source(reduceState, la);
      }
    }
  }

  /**
  \brief Removes all lookahead sources and replaces them with generated lookaheads.
  */
  void finalize_lookaheads() {
    unordered_map<LookaheadSource, LookaheadSet> lookaheadMap;
    for (auto& state : states_) {
      // reset for each state in case of lookahead loops
      lookaheadMap.clear();
      for (auto& item : state.items()) {
        for (auto&& source : item.lookaheads()) {
          auto it = lookaheadMap.find(source);
          if (it == lookaheadMap.end()) {
            // lookahead source not resolved
            lookahead_lookup(source, lookaheadMap);
            it = lookaheadMap.find(source);
          }
          item.generated_lookaheads() |= it->second;
        }
        // remove all relative lookaheads from this item
        item.lookaheads().clear();
        item.lookaheads().shrink_to_fit();
      }
    }
  }

};  // namespace ctf::ialr
}  // namespace ctf::ialr
#endif