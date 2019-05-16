#ifndef CTF_LR_LALR_HPP
#define CTF_LR_LALR_HPP

#include "ctf_lr_lr1.hpp"

namespace ctf::lalr {
using Item = ctf::lr1::Item;

class StateMachine : public ctf::lr1::StateMachine {
 public:
  // use the same constructors
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

  StateMachine(const TranslationGrammar& tg, bool) : ctf::lr1::StateMachine(tg, true) {}
};
}  // namespace ctf::lalr
#endif