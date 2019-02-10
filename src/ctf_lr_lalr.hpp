#ifndef CTF_LR_LALR_HPP
#define CTF_LR_LALR_HPP

#include "ctf_lr_lr1.hpp"

namespace ctf::lalr {
using Item = ctf::lr1::Item;

class StateMachine : public ctf::lr1::StateMachine {
 public:
  // use the same constructors
  StateMachine(const TranslationGrammar& grammar)
      : ctf::lr1::StateMachine(grammar, create_empty(grammar), create_first(grammar, empty_)) {
    // initial item S' -> .S$
    insert_state({Item({grammar.starting_rule(), 0}, {}, {Symbol::eof()})});
    // recursively expand all states: dfs
    expand_state(0);
    // push all lookaheads to their items
    finalize_lookaheads();
  }

 protected:
  tuple<size_t, bool> merge(const std::vector<size_t>& existingStates,
                            const State& newState) override {
    assert(existingStates.size() == 1);
    auto& state = states_[existingStates[0]];
    // always succeeds, merge lookahead sources
    for (size_t i = 0; i < state.items().size(); ++i) {
      auto& item = state.items()[i];
      auto& item2 = newState.items()[i];

      item.lookaheads() = set_union(item.lookaheads(), item2.lookaheads());
      // there are never any generated lookaheads
    }
    return {existingStates[0], true};
  }
};
}  // namespace ctf::lalr
#endif