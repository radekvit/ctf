/**
\file ctf_lr_lscelr.hpp
\brief Contains the LSCELR automaton implementation.
\author Radek Vít
*/
#ifndef CTF_LR_LSCELR_HPP
#define CTF_LR_LSCELR_HPP

#include <optional>

#include "ctf_lr_lr1.hpp"

namespace ctf::lscelr {
using Item = ctf::lr1::Item;
using LookaheadSet = ctf::lr1::LookaheadSet;
using LookaheadSource = ctf::lr1::LookaheadSource;
/**
\brief Get a specific successor item kernel.

\param[in] state The LS state we want this for.
\param[in] s The symbols we skip over in the state.
\param[in] id The identifier of the state.
*/
inline vector_set<Item> symbol_skip_kernel(const vector_set<Item>& state,
                                           Symbol s,
                                           const std::size_t id) {
  vector_set<Item> result;

  for (std::size_t i = 0; i < state.size(); ++i) {
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
/**
\brief The LSCELR automaton.
*/
class StateMachine : public ctf::lalr::StateMachine {
 public:
  /**
  \brief Constructs the LSCELR automaton. Merges all isocores with the same potential conflict
  contributions.

  \param[in] grammar The translation grammar of this automaton.
  */
  StateMachine(const TranslationGrammar& grammar) : ctf::lalr::StateMachine(grammar, true) {
    // initial item S' -> .S$
    insert_state({Item(
      {grammar.starting_rule(), 0}, {}, lr1::LookaheadSet(grammar.terminals(), {Symbol::eof()}))});
    // recursively expand all states: dfs
    expand_state(0);
    // identify states with conflicts
    auto conflictedStates = detect_conflicts();

    if (!conflictedStates.empty()) {
      _contributions = vector<std::optional<vector<LookaheadSet>>>(
        _states.size(), std::optional<vector<LookaheadSet>>());
      // mark all but the first lookahead source for renewed merging
      mark_conflicts(conflictedStates);
      // split states with conflicts along the way
      split_states();
    }
    // push all lookaheads to their items
    finalize_lookaheads();
  }

 protected:
  /**
  \brief The set of potential conflict contributions for all states.
  */
  vector<std::optional<vector<LookaheadSet>>> _contributions;
  /**
  \brief The set of states that need to be split.
  */
  vector_set<std::size_t> _statesToSplit;
  /**
  \brief The set of lookaheads that contribute to conflicts for states that have them.

  This serves as cache so that we don't recompute the set for every attempt to merge.
  */
  vector<std::optional<vector<vector<LookaheadSet>>>> _contributionLookaheads;
  /**
  \brief A single conflict. Contains the state where it manifests and the conflicted symbols for
  each item.
  */
  struct Conflict {
    std::size_t state;
    unordered_map<std::size_t, LookaheadSet> contributions;
  };
  /**
  \brief Detect all conflicts and return their representation.
  */
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
  /**
  \brief All possible LR actions or conflict.
  */
  enum class Action {
    NONE,
    REDUCE,
    SHIFT,
    CONFLICT,
  };
  /**
  \brief Obtain all conflicts for a single state.

  \param[in] state The examined state.
  \param[in] stateLookaheads The full lookahead set of the state.

  \returns A map where the keys are item indices and the values are the conflicted terminals.
  */
  unordered_map<std::size_t, LookaheadSet> conflicts(State& state,
                                                     const vector<LookaheadSet>& stateLookaheads) {
    unordered_map<std::size_t, LookaheadSet> result;
    vector<tuple<Action, std::size_t>> actions(grammar().terminals(), {Action::NONE, 0});
    for (std::size_t i = 0; i < state.items().size(); ++i) {
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
  /**
  \brief Add a conflicted symbol to the contribution set of a single item.

  \param[in] item The item's id.
  \param[in] symbol The conflicted symbol.
  \param[in] map The conflict map.
  */
  void add_to_lookahead(std::size_t item,
                        Symbol symbol,
                        unordered_map<std::size_t, LookaheadSet>& map) {
    auto& contribution = map.try_emplace(item, grammar().terminals()).first->second;
    contribution[symbol] = true;
  }
  /**
  \brief Mark conflict contributions for all states.

  \param[in] conflicts The detected conflicts in all states.
  */
  void mark_conflicts(const vector<Conflict>& conflicts) {
    for (auto& conflict : conflicts) {
      for (const auto& [item, contributions] : conflict.contributions) {
        mark_conflict(conflict.state, item, contributions);
      }
    }
  }
  /**
  \brief Recursively mark the conflict contributions caused by a single conflict.

  \param[in] stateIndex The state we mark the contributions for.
  \param[in] itemIndex The index of the conflicted item.
  \param[in] contributions The set of conflicted symbols to mark.
  */
  void mark_conflict(std::size_t stateIndex, std::size_t itemIndex, LookaheadSet contributions) {
    auto& state = _states[stateIndex];
    auto& item = state.items()[itemIndex];
    if (item.lookahead_sources().empty() || (contributions -= item.lookaheads()).none()) {
      // all generated, nothing to mark
      return;
    }
    // add to conflict contributions
    auto& contribution = _contributions[state.id()];
    if (!contribution) {
      contribution.emplace(state.items().size(), LookaheadSet(grammar().terminals()));
      contribution.value()[itemIndex] |= contributions;
    } else if (!contribution.value()[itemIndex].set_union(contributions)) {
      // no new contributions added, all has been marked
      return;
    }
    if (item.lookahead_sources().size() > 1) {
      // mark to split
      _statesToSplit.insert(state.id());
    }
    for (auto& [nextStateIndex, nextItem] : item.lookahead_sources()) {
      mark_conflict(nextStateIndex, nextItem, contributions);
    }
  }
  /**
  \brief Locate the index for splitting lookahead sources so that the first state source is kept.

  \param[in] item The examined item.

  \returns The index of the first lookahead source that is different from the first source's state.
  */
  std::size_t split_location(const Item& item) {
    auto& sources = item.lookahead_sources();
    std::size_t split = 1;
    const std::size_t keptState = sources[0].state;
    for (std::size_t i = 1; i < sources.size(); ++i) {
      if (sources[i].state != keptState) {
        break;
      }
      ++split;
    }
    return split;
  }
  /**
  \brief Split all conflicted states to eliminate LALR-caused conflicts.

  First, we split states with multiple sources and then we regenerate the corresponding successor
  states.
  */
  void split_states() {
    vector<vector_set<LookaheadSource>> splitSources;
    splitSources.reserve(_statesToSplit.size());
    // remove extra sources from all states to split
    for (auto& stateIndex : _statesToSplit) {
      auto& state = _states[stateIndex];
      // store sources from the first item
      // the first item will always store the source states
      // we only need the transition symbol
      // remove all but the first source state from all items
      auto& firstItem = state.items()[0];
      splitSources.push_back(firstItem.lookahead_sources().split(split_location(firstItem)));
      for (auto& item : state.items()) {
        if (item.lookahead_sources().empty()) {
          continue;
        }
        item.lookahead_sources().split(split_location(item));
      }
    }
    // cache lookahead contributions to states
    _contributionLookaheads.assign(_states.size(), {});
    unordered_map<LookaheadSource, LookaheadSet> lookaheadMap;
    for (std::size_t i = 0; i < _states.size(); ++i) {
      auto& contribution = _contributions[i];
      if (!contribution)
        continue;

      _contributionLookaheads[i] = {
        lookaheads_lscelr(_states[i], contribution.value(), lookaheadMap)};
    }

    // regenerate transitions
    for (auto& sources : splitSources) {
      for (auto& [sourceStateIndex, sourceItemIndex] : sources) {
        auto& sourceState = _states[sourceStateIndex];
        auto& sourceItem = sourceState.items()[sourceItemIndex];
        auto& transitionSymbol = sourceItem.rule().input()[sourceItem.mark()];

        auto [state, inserted] = insert_state_lscelr(
          symbol_skip_kernel(sourceState.items(), transitionSymbol, sourceStateIndex));
        if (inserted) {
          // modify transition
          // state reference may have been invalidated
          _states[sourceStateIndex].transitions()[transitionSymbol] = state;
          // generate successor states
          expand_state_lscelr(state);
        }
      }
    }
  }
  /**
  \brief Attempt to insert a new state after splitting states.

  \param[in] kernel The kernel of the inserted item.

  \returns A structure containing the state's index and whether it was merged.
  */
  InsertResult insert_state_lscelr(const vector_set<Item>& kernel) {
    std::size_t i = _states.size();
    State newState(i, kernel, grammar(), _empty, _first);

    auto& kernelStates = _kernelMap[kernel];
    // this is never empty
    auto [other, merged] = merge_lscelr(kernelStates, newState);
    if (merged) {
      return {other, false};
    }
    kernelStates.push_back(i);
    _states.push_back(std::move(newState));
    return {i, true};
  }
  /**
  \brief Recursively expand new states after the initial state splitting.

  \param[in] i The index of the expanded state.

  We use the LSCELR compatibility test for all merging.
  */
  void expand_state_lscelr(std::size_t i) {
    for (auto& [symbol, kernel] : symbol_skip_kernels(_states[i].items(), i)) {
      auto [id, inserted] = insert_state_lscelr(kernel);
      _states[i].transitions()[symbol] = id;
      // new inserted state
      if (inserted) {
        expand_state_lscelr(id);
      }
    }
  }
  /**
  \brief Try to merge a new state to some existing state that first the LSCELR merge compatibility
  test.

  \param[in] isocores The set of states that are isocores of newState.
  \param[in] newState The new state we are attempting to merge with the existing states.

  \returns The structure containing the index of the state and whether it was merged.

  Two states are compatible if they are isocores and have the same potential conflict contributions
  (the mask is stored for the first isocore).
  */
  MergeResult merge_lscelr(const std::vector<std::size_t>& isocores, const State& newState) {
    // there is always a state from LALR
    auto& contribution = _contributions[isocores[0]];
    if (!contribution) {
      // not a conflicted state, always merge
      auto& state = _states[isocores[0]];
      for (std::size_t i = 0; i < state.items().size(); ++i) {
        auto& item = state.items()[i];
        auto& item2 = newState.items()[i];

        item.lookahead_sources() = set_union(item.lookahead_sources(), item2.lookahead_sources());
        // there are never any generated lookaheads
      }
      return {isocores[0], true};
    }
    // a conflicted state:
    auto& contributionLookaheads = _contributionLookaheads[isocores[0]].value();
    auto newLookaheads = lookaheads_lscelr(newState, contribution.value());

    for (std::size_t i = 0; i < isocores.size(); ++i) {
      std::size_t other = isocores[i];
      auto& existing = _states[other];
      auto& lookahead = contributionLookaheads[i];
      // lookaheads match in the conflicting states
      if (lookahead == newLookaheads) {
        for (std::size_t i = 0; i < existing.items().size(); ++i) {
          auto& item = existing.items()[i];
          auto& item2 = newState.items()[i];

          item.lookahead_sources() = set_union(item.lookahead_sources(), item2.lookahead_sources());
        }
        return {other, true};
      }
    }
    contributionLookaheads.push_back(newLookaheads);
    return {0, false};
  }
  /**
  \brief Get the set of lookaheads masked with the potential contributions for this state.

  \param[in] state The examined state.
  \param[in] masks Potential contributions for each item.

  \returns The lookahead set masked with the contributions.
  */
  vector<LookaheadSet> lookaheads_lscelr(const State& state, const vector<LookaheadSet>& masks) {
    unordered_map<LookaheadSource, LookaheadSet> lookaheadMap;
    return lookaheads_lscelr(state, masks, lookaheadMap);
  }
  /**
  \brief Get the set of lookaheads masked with the potential contributions for this state.

  \param[in] state The examined state.
  \param[in] masks Potential contributions for each item.
  \param[in,out] lookaheadMap A map containing the full lookahead sets for some sources.

  \returns The lookahead set masked with the contributions.
  */
  vector<LookaheadSet> lookaheads_lscelr(
    const State& state,
    const vector<LookaheadSet>& masks,
    unordered_map<LookaheadSource, LookaheadSet>& lookaheadMap) {
    vector<LookaheadSet> result;
    LookaheadSet lookaheadMask(0);

    for (std::size_t i = 0; i < state.items().size(); ++i) {
      auto& item = state.items()[i];
      auto& mask = masks[i];
      if (mask.empty()) {
        continue;
      }
      lookaheadMask = mask;
      result.push_back(item.lookaheads());
      for (auto& source : item.lookahead_sources()) {
        unordered_map<LookaheadSource, LookaheadSet> tempMap(lookaheadMap);
        auto it = lookaheadMap.find(source);
        if (it == lookaheadMap.end()) {
          // lookahead source not resolved
          lookahead_lookup_lscelr(source, lookaheadMask, tempMap);
          it = tempMap.find(source);
          lookaheadMap.insert_or_assign(it->first, it->second);
        }
        result.back() |= it->second;
        if (lookaheadMask.empty()) {
          break;
        }
      }
      result.back() &= mask;
    }

    return result;
  }
  /**
  \brief Obtain the lookahead set for a single source masked with potential contributions and store
  it in a map.

  \param[in] source The examined source.
  \param[in] lookaheadMask The set of potential contributions.
  \param[in,out] lookaheadMap A map containing the full lookahead sets for some sources.
  */
  void lookahead_lookup_lscelr(const LookaheadSource& source,
                               LookaheadSet& lookaheadMask,
                               unordered_map<LookaheadSource, LookaheadSet>& lookaheadMap) {
    const auto& state = _states[source.state];
    // get all sources
    auto& item = state.items()[source.item];

    // stop infinite loops
    lookaheadMap.insert_or_assign(source, item.lookaheads());

    lookaheadMask -= item.lookaheads();
    if (lookaheadMask.empty()) {
      return;
    }

    LookaheadSet symbols(item.lookaheads());
    for (auto& nextSource : item.lookahead_sources()) {
      auto it = lookaheadMap.find(nextSource);
      if (it == lookaheadMap.end()) {
        // recursive source not resolved yet
        lookahead_lookup(nextSource, lookaheadMap);
        it = lookaheadMap.find(nextSource);
      }
      symbols |= it->second;
      if (lookaheadMask.empty()) {
        lookaheadMap.insert_or_assign(source, std::move(symbols));
        return;
      }
    }
    lookaheadMap.insert_or_assign(source, std::move(symbols));
  }
};
}  // namespace ctf::lscelr
#endif
/*** End of file ctf_lr_lscelr.hpp ***/
