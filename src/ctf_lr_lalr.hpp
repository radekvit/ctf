/**
\file ctf_lr_lalr.hpp
\brief Contains the LSLALR automaton implementation.
\author Radek Vít
*/
#ifndef CTF_LR_LALR_HPP
#define CTF_LR_LALR_HPP

#include "ctf_lr_lr1.hpp"

namespace ctf::lalr {
using Item = ctf::lr1::Item;

/**
\brief The LSLALR automaton. Merges all LR(1) isocores.
*/
class StateMachine : public ctf::lr1::StateMachine {
 public:
  /**
  \brief Construct the LSLALR automaton from a translation grammar.

  \param[in] grammar An augmented translation grammar.
  */
  StateMachine(const TranslationGrammar& grammar) : ctf::lr1::StateMachine(grammar, true) {
    // initial item S' -> .S$
    insert_state({Item(
      {grammar.starting_rule(), 0}, {}, lr1::LookaheadSet(grammar.terminals(), {Symbol::eof()}))});
    // recursively expand all states: dfs
    expand_state(0);
    // push all lookaheads to their items
    finalize_lookaheads();
  }

 protected:
  /**
  \brief Merges a new state to an existing state if they are isocores.

  \param[in] existingStates A vector of existing states that are isocores. Always contains 0 or 1
  elements. \param[in] newState The new state we are trying to merge.

  \returns The structure containing the index of the state and whether it was merged.
  */
  MergeResult merge(const std::vector<std::size_t>& existingStates, State& newState) override {
    if (existingStates.empty()) {
      return {0, false};
    }
    assert(existingStates.size() == 1);
    auto& state = _states[existingStates[0]];
    // always succeeds, merge lookahead sources
    for (std::size_t i = 0; i < state.items().size(); ++i) {
      auto& item = state.items()[i];
      auto& item2 = newState.items()[i];

      item.lookahead_sources() = set_union(item.lookahead_sources(), item2.lookahead_sources());
    }
    return {existingStates[0], true};
  }
  /**
  \brief Initializes the basic fields and nothing else.
  */
  StateMachine(const TranslationGrammar& tg, bool) : ctf::lr1::StateMachine(tg, true) {}
};
}  // namespace ctf::lalr

#endif
/*** End of file ctf_lr_lalr.hpp ***/
