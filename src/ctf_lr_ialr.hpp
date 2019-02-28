#ifndef CTF_LR_IALR_HPP
#define CTF_LR_IALR_HPP

#include "ctf_lr_lr1.hpp"
// TODO: new approach
// 1) LALR
// 2) identify conflicts
// 3) mark all mergable kernels states with the potential contributions
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
      : _id(id), _items(closure(kernel, grammar, empty, first)), _reduceTargets() {
      // we can only merge states when the kernel only contains rules in the form A -> x.Y
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

    vector_set<size_t>& _reducetargets() noexcept { return _reduceTargets; }
    const vector_set<size_t>& _reducetargets() const noexcept { return _reduceTargets; }

    unordered_map<Symbol, vector_set<size_t>>& conflicts() noexcept { return _conflicts; }
    const unordered_map<Symbol, vector_set<size_t>>& conflicts() const noexcept {
      return _conflicts;
    }

    void set_expanded() noexcept { _expanded = true; }
    bool expanded() const noexcept { return _expanded; }

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
    size_t _id;
    // closure of kernel
    vector_set<Item> _items;

    // state transitions
    unordered_map<Symbol, size_t> _transitions;

    vector_set<size_t> _reduceTargets;
    // reduce state conflicts
    unordered_map<Symbol, vector_set<size_t>> _conflicts;

    bool _reduce = false;
    bool _expanded = false;
  };

  StateMachine(const TranslationGrammar& grammar)
    : StateMachine(grammar, create_empty(grammar), create_first(grammar, _empty)) {
    // initial item S' -> .S$
    insert_state(
      {Item({grammar.starting_rule(), 0}, {}, LookaheadSet(grammar.terminals(), {Symbol::eof()}))});
    // recursively expand all states: dfs
    expand_state(0);
    // merge or generate postoponed states
    handle_postponed();
    // push all lookaheads to their items
    finalize_lookaheads();
  }

  ~StateMachine() = default;

  const vector<State>& states() const noexcept { return _states; }

 protected:
  const TranslationGrammar* _grammar;
  empty_t _empty;
  first_t _first;
  vector<State> _states;

  map<vector_set<Item>, vector<size_t>> _kernelMap;

  struct PostponedTransition {
    size_t state;
    vector<Symbol> transitionSymbols;
  };

  deque<PostponedTransition> _postponed;

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
    : _grammar(&grammar), _empty(std::move(empty)), _first(std::move(first)) {}

  const TranslationGrammar& grammar() const noexcept { return *_grammar; }

  InsertResult insert_state(const vector_set<Item>& kernel) {
    // identifier for new state
    size_t i = _states.size();
    State newState(i, kernel, grammar(), _empty, _first);
    // try to merge with another state
    auto& kernelStates = _kernelMap[kernel];
    if (kernelStates.empty()) {
      // new mergable kernel
      kernelStates.push_back(i);

      _states.push_back(std::move(newState));
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
      _states.push_back(std::move(newState));
      return {i, true, false};
    }
  }

  void expand_state(size_t i) {
    // set as reduce target for all lookahead sources
    mark_as_reduce(i, _states[i]);
    vector<Symbol> postponedSymbols;
    // expand all transitions
    for (auto [symbol, kernel] : lr1::symbol_skip_kernels(_states[i].items(), i)) {
      auto [id, inserted, postponed] = insert_state(kernel);
      auto& state = _states[i];
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
      _postponed.push_back({i, std::move(postponedSymbols)});
    }
    // mark; this state knows its lookahead reduce targets
    _states[i].set_expanded();
  }

  MergeResult merge(const std::vector<size_t>& isocores, const State& newState) {
    auto newLookaheads = lookaheads(newState);
    vector<vector<LookaheadSet>> existingLookaheads;
    // first try to merge as canonical LR(1)
    for (auto other : isocores) {
      auto& existing = _states[other];
      existingLookaheads.push_back(lookaheads(existing));
      // always merge if lookahead set is the same
      if (existingLookaheads.back() == newLookaheads) {
        // add lookahead sources
        merge_lookaheads(existing, newState);
        merge__reducetargets(existing, newState);
        return {other, true};
      }
    }
    // try to merge
    size_t speculative = 0;
    for (size_t i = 0; i < isocores.size(); ++i) {
      auto other = isocores[i];
      auto& existing = _states[other];
      // we can resolve this later
      if (!existing.expanded()) {
        speculative = 1;
        continue;
      }
      // try to merge state
      auto [state, success] = ialr_merge(other, newState, existingLookaheads[i], newLookaheads);
      if (success) {
        merge_lookaheads(_states[state], newState);

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
      result.push_back(item.lookaheads());
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

  vector<LookaheadSet> lookaheads(const State& state,
                                  unordered_map<LookaheadSource, LookaheadSet> lookaheadMap) {
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
      result.push_back(item.lookaheads());
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

  // try to merge postponed transitions
  void handle_postponed() {
    while (!_postponed.empty()) {
      auto [i, symbols] = _postponed.front();
      _postponed.pop_front();

      auto& state = _states[i];
      auto transitions = lr1::symbol_skip_kernels(state.items(), i);
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
        _postponed.push_back({i, std::move(postponedSymbols)});
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

    auto& existing = _states[state];
    unordered_map<LookaheadSource, LookaheadSet> lookaheadMap;
    // for all states that are la targets
    for (auto rstatei : existing._reducetargets()) {
      auto& rstate = _states[rstatei];
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
    merge__reducetargets(existing, newState);
    return {state, true};
  }

  enum class TempAction {
    NONE,
    REDUCE,
    SHIFT,
    CONFLICT,
  };

  // list all reduce contributions to R/R and S/R conflicts
  unordered_map<Symbol, vector_set<size_t>> conflicts(State& state,
                                                      const vector<LookaheadSet>& stateLookaheads) {
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

      item.lookahead_sources() = set_union(item.lookahead_sources(), item2.lookahead_sources());
    }
  }

  void merge__reducetargets(const State& mergeState, const State& newState) {
    for (auto& item : newState.items()) {
      for (auto reduceState : mergeState._reducetargets()) {
        mark_source(reduceState, item.lookahead_sources());
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
      mark_source(i, item.lookahead_sources());
      // can mark itself if this is a mergable state
      if (!item.lookahead_sources().empty()) {
        state._reducetargets().insert(i);
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
      auto& state = _states[source.state];
      // only mark transitional nodes
      const auto& la = state.items()[source.item].lookahead_sources();
      if (!la.empty() && state._reducetargets().insert(reduceState).inserted) {
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
    for (auto& state : _states) {
      // reset for each state in case of lookahead loops
      lookaheadMap.clear();
      for (auto& item : state.items()) {
        for (auto&& source : item.lookahead_sources()) {
          auto it = lookaheadMap.find(source);
          if (it == lookaheadMap.end()) {
            // lookahead source not resolved
            lookahead_lookup(source, lookaheadMap);
            it = lookaheadMap.find(source);
          }
          item.lookaheads() |= it->second;
        }
        // remove all relative lookaheads from this item
        item.lookahead_sources().clear();
        item.lookahead_sources().shrink_to_fit();
      }
    }
  }

};  // namespace ctf::ialr
}  // namespace ctf::ialr
#endif