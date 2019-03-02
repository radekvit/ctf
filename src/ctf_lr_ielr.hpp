#ifndef CTF_LR_IELR_HPP
#define CTF_LR_IELR_HPP

#include "ctf_lr_lr1.hpp"

namespace ctf::ielr {
using Item = ctf::lr1::Item;
using LookaheadSet = ctf::lr1::LookaheadSet;
using LookaheadSource = ctf::lr1::LookaheadSource;

inline vector_set<Item> symbol_skip_kernel(const vector_set<Item>& state,
                                           Symbol s,
                                           const size_t id) {
  vector_set<Item> result;

  for (size_t i = 0; i < state.size(); ++i) {
    auto& item = state[i];
    auto& symbol = item.rule().input()[item.mark()];
    if (item.reduce() || symbol != s) {
      continue;
    }
    Item newItem(item.next({id, i}));
    result = set_union(result, {newItem});
  }
  return result;
}

class StateMachine : public ctf::lr1::StateMachine {
 public:
  // use the same constructors
  StateMachine(const TranslationGrammar& grammar)
    : ctf::lr1::StateMachine(grammar, true) {
    // initial item S' -> .S$
    insert_state({Item(
      {grammar.starting_rule(), 0}, {}, lr1::LookaheadSet(grammar.terminals(), {Symbol::eof()}))});
    // recursively expand all states: dfs
    expand_state(0);
    // identify states with conflicts
    auto conflictedStates = detect_conflicts();

    if (!conflictedStates.empty()) {
      // mark all but the first lookahead source for renewed merging
      mark_conflicts(conflictedStates);
      // split states with conflicts along the way
      split_states();
    }
    // push all lookaheads to their items
    finalize_lookaheads();
  }

 protected:
  unordered_map<size_t, vector<LookaheadSet>> _contributions;
  vector_set<size_t> _statesToSplit;

  MergeResult merge(const std::vector<size_t>& existingStates, const State& newState) override {
    assert(existingStates.size() == 1);
    auto& state = _states[existingStates[0]];
    // always succeeds, merge lookahead sources
    for (size_t i = 0; i < state.items().size(); ++i) {
      auto& item = state.items()[i];
      auto& item2 = newState.items()[i];

      item.lookahead_sources() = set_union(item.lookahead_sources(), item2.lookahead_sources());
      // there are never any generated lookaheads
    }
    return {existingStates[0], true};
  }

  struct Conflict {
    size_t state;
    unordered_map<size_t, LookaheadSet> contributions;
  };

  vector<Conflict> detect_conflicts() {
    vector<Conflict> result;
    for (auto& state : _states) {
      if (!state.has_reduce()) {
        continue;
      }
      auto conflictMap = conflicts(state, lookaheads(state));
      if (conflictMap.empty()) {
        continue;
      }
      result.push_back({state.id(), std::move(conflictMap)});
    }
    return result;
  }

  enum class Action {
    NONE,
    REDUCE,
    SHIFT,
    CONFLICT,
  };
  // list all reduce contributions to R/R and S/R conflicts for individual items
  unordered_map<size_t, LookaheadSet> conflicts(State& state,
                                                const vector<LookaheadSet>& stateLookaheads) {
    unordered_map<size_t, LookaheadSet> result;
    vector<tuple<Action, size_t>> actions(grammar().terminals(), {Action::NONE, 0});
    for (size_t i = 0; i < state.items().size(); ++i) {
      auto& item = state.items()[i];
      auto& lookahead = stateLookaheads[i];
      if (item.reduce()) {
        for (auto& symbol : lookahead.symbols()) {
          auto& [action, item] = actions[symbol.id()];
          switch (action) {
            case Action::NONE:
              action = Action::REDUCE;
              item = i;
              break;
            case Action::REDUCE: {
              add_to_lookahead(item, symbol, result);
              add_to_lookahead(i, symbol, result);
              action = Action::CONFLICT;
              break;
            }
            case Action::SHIFT:
              action = Action::CONFLICT;
              [[fallthrough]];
            case Action::CONFLICT:
              add_to_lookahead(i, symbol, result);
              break;
          }
        }
      } else if (item.rule().input()[item.mark()].terminal()) {
        auto& symbol = item.rule().input()[item.mark()];
        auto& [action, item] = actions[symbol.id()];
        switch (action) {
          case Action::NONE:
            action = Action::SHIFT;
            break;
          case Action::REDUCE:
            action = Action::CONFLICT;
            add_to_lookahead(item, symbol, result);
            break;
          case Action::SHIFT:
          case Action::CONFLICT:
            break;
        }
      }
    }
    return result;
  }

  void add_to_lookahead(size_t item, Symbol symbol, unordered_map<size_t, LookaheadSet>& map) {
    auto& contribution = map.try_emplace(item, grammar().terminals()).first->second;
    contribution[symbol] = true;
  }

  void mark_conflicts(const vector<Conflict>& conflicts) {
    for (auto& conflict : conflicts) {
      for (const auto& [item, contributions] : conflict.contributions) {
        mark_conflict(conflict.state, item, contributions);
      }
    }
  }

  void mark_conflict(size_t stateIndex, size_t itemIndex, LookaheadSet contributions) {
    auto& state = _states[stateIndex];
    auto& item = state.items()[itemIndex];
    if (item.lookahead_sources().empty() || (contributions &= ~item.lookaheads()).none()) {
      // all generated, nothing to mark
      return;
    }
    // add to conflict contributions
    auto&& defaultContributions =
      vector<LookaheadSet>(state.items().size(), LookaheadSet(grammar().terminals()));
    if (!_contributions.try_emplace(state.id(), defaultContributions)
           .first->second[itemIndex]
           .set_union(contributions)) {
      // no new contributions added, all has been marked
      return;
    }
    // conflicts[state, itemIndex] |= contributions;
    if (item.lookahead_sources().size() > 1) {
      // mark to split
      _statesToSplit.insert(state.id());
    }
    for (auto& [nextStateIndex, nextItem] : item.lookahead_sources()) {
      mark_conflict(nextStateIndex, nextItem, contributions);
    }
  }

  void split_states() {
    vector<vector_set<LookaheadSource>> splitSources;
    splitSources.reserve(_statesToSplit.size());
    // remove extra sources from all states to split
    for (auto& stateIndex : _statesToSplit) {
      auto& state = _states[stateIndex];
      // store sources from the first item
      // storing the first item is OK, we only need the transition symbol
      splitSources.push_back(state.items()[0].lookahead_sources().split(1));
      // remove all but the first source from all items
      for (auto& item : state.items()) {
        item.lookahead_sources().split(1);
      }
    }

    // regenerate transitions
    for (size_t i = 0; i < _statesToSplit.size(); ++i) {
      auto& sources = splitSources[i];
      for (auto& [sourceState, sourceItem] : sources) {
        auto& splitState = _states[sourceState];
        auto& targetItem = splitState.items()[sourceItem];
        auto& transitionSymbol = targetItem.rule().input()[targetItem.mark()];

        auto [state, inserted] = insert_state_ielr(
          symbol_skip_kernel(splitState.items(), transitionSymbol, splitState.id()));
        // modify transition
        splitState.transitions()[transitionSymbol] = state;
        if (inserted) {
          // inserted new state, generate successor states
          expand_state_ielr(state);
        }
      }
    }
  }

  InsertResult insert_state_ielr(const vector_set<Item>& kernel) {
    size_t i = _states.size();
    State newState(i, kernel, grammar(), _empty, _first);

    auto& kernelStates = _kernelMap[kernel];
    // this is never empty
    auto [other, merged] = merge_ielr(kernelStates, newState);
    if (merged) {
      return {other, false};
    }
    kernelStates.push_back(i);
    _states.push_back(std::move(newState));
    return {i, true};
  }

  void expand_state_ielr(size_t i) {
    for (auto&& [symbol, kernel] : symbol_skip_kernels(_states[i].items(), i)) {
      auto [id, inserted] = insert_state_ielr(kernel);
      _states[i].transitions()[symbol] = id;
      // new inserted state
      if (inserted) {
        expand_state_ielr(id);
      }
    }
  }

  MergeResult merge_ielr(const std::vector<size_t>& isocores, const State& newState) {
    auto it = _contributions.find(isocores[0]);
    if (it == _contributions.end()) {
      // not a conflicted state, always merge
      auto& state = _states[isocores[0]];
      for (size_t i = 0; i < state.items().size(); ++i) {
        auto& item = state.items()[i];
        auto& item2 = newState.items()[i];

        item.lookahead_sources() = set_union(item.lookahead_sources(), item2.lookahead_sources());
        // there are never any generated lookaheads
      }
      return {isocores[0], true};
    }
    // a conflicted state:
    auto newLookaheads = lookaheads_ielr(newState, it->second);

    for (auto other : isocores) {
      auto& existing = _states[other];
      auto lookahead = lookaheads_ielr(existing, it->second);
      // lookaheads match in the conflicting states
      if (lookahead == newLookaheads) {
        for (size_t i = 0; i < existing.items().size(); ++i) {
          auto& item = existing.items()[i];
          auto& item2 = newState.items()[i];

          item.lookahead_sources() = set_union(item.lookahead_sources(), item2.lookahead_sources());
          // there are never any generated lookaheads
        }
        return {other, true};
      }
    }
    return {0, false};
  }

  vector<LookaheadSet> lookaheads_ielr(const State& state, const vector<LookaheadSet>& mask) {
    auto result = lookaheads(state);
    assert(mask.size() == result.size());

    for (size_t i = 0; i < result.size(); ++i) {
      result[i] &= mask[i];
    }
    return result;
  }
};
}  // namespace ctf::ielr
#endif