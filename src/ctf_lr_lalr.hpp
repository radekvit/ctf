#ifndef CTF_LR_LALR_HPP
#define CTF_LR_LALR_HPP

#include "ctf_lr_lr1.hpp"

namespace ctf::lalr {
using Item = ctf::lr1::Item;

class StateMachine : public ctf::lr1::StateMachine {
 public:
  // the same constructor
  using ctf::lr1::StateMachine::StateMachine;

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