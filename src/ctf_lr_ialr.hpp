#ifndef CTF_LR_IALR_HPP
#define CTF_LR_IALR_HPP

#include "ctf_lr_lr1.hpp"

namespace ctf::ialr {

using Item = lr1::Item;

class StateMachine {
 public:
  using Item = Item;
  class State {
   public:
    State(size_t id,
          const vector_set<Item>& isocore,
          const TranslationGrammar& grammar,
          const empty_t& empty,
          const first_t& first)
        : id_(id), items_(closure(isocore, grammar, empty, first)) {
      // we can only merge states when the isocore only contains rules in the form A -> x.Y
      for (auto&& item : isocore) {
        if (item.mark() != 1) {
          mergable_ = false;
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

   private:
    // isocore identifier
    size_t id_;
    // closure of isocore
    vector_set<Item> items_;

    // state transitions
    unordered_map<Symbol, size_t> transitions_;

    vector_set<size_t> reduceTargets_;

    bool mergable_ = false;
    bool reduce_ = false;
  };

 protected:
};
// TODO smarter LR(1), different merge

// merge: it existing is closed, check creation of:
// mysterious new conflicts
// mysterious invasive conflicts
// mysterious mutated conflicts

// if existing is not closed, TODO
// current solution: merge if lookahead set is the same
}  // namespace ctf::ialr
#endif