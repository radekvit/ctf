/**
\file lr_table.hpp
\brief Defines class LRTable and its methods.
\author Radek Vít
*/
#ifndef CRF_LR_TABLE_HPP
#define CRF_LR_TABLE_HPP

#include "ctf_base.hpp"
#include "ctf_lr_lalr.hpp"
#include "ctf_lr_lr1.hpp"
#include "ctf_lr_lscelr.hpp"

namespace ctf {

enum class LRAction {
  SHIFT,
  REDUCE,
  SUCCESS,
  ERROR,
};

struct LRActionItem {
  LRAction action = LRAction::ERROR;
  size_t argument = 0;

  friend bool operator==(const LRActionItem& lhs, const LRActionItem& rhs) {
    return lhs.action == rhs.action && lhs.argument == rhs.argument;
  }

  friend bool operator!=(const LRActionItem& lhs, const LRActionItem& rhs) { return !(lhs == rhs); }
};

class LRGenericTable {
 public:
  const LRActionItem& lr_action(size_t state, const Symbol& terminal) const {
    return _actionTable[actionIndex(state, terminal.id())];
  }

  const size_t& lr_goto(size_t state, const Symbol& nonterminal) const {
    return _gotoTable[gotoIndex(state, nonterminal.id())];
  }

  size_t total_states() const { return _states; }

 protected:
  vector<LRActionItem> _actionTable;
  vector<size_t> _gotoTable;

  size_t _states;
  size_t _terminals;
  size_t _nonterminals;

  LRActionItem& lr_action_item(size_t state, const Symbol& terminal) {
    return _actionTable[actionIndex(state, terminal.id())];
  }

  size_t& lr_goto_item(size_t state, const Symbol& nonterminal) {
    return _gotoTable[gotoIndex(state, nonterminal.id())];
  }

  /**
  \brief Maps 2D indices to 1D indices.
  */
  size_t actionIndex(size_t y, size_t x) const { return _terminals * y + x; }
  /**
  \brief Maps 2D indices to 1D indices.
  */
  size_t gotoIndex(size_t y, size_t x) const { return _nonterminals * y + x; }

  void initialize_tables(size_t size) {
    _actionTable = {size * _terminals, {LRAction::ERROR, 0}};
    _gotoTable = vector<size_t>(size * _nonterminals, 0);

    _states = size;
  }
};

template <typename StateMachine, const char* type>
class LR1GenericTable : public LRGenericTable {
  // TODO: conflict resolution
 public:
  LR1GenericTable() {}
  LR1GenericTable(const TranslationGrammar& grammar) {
    StateMachine sm(grammar);
    _terminals = grammar.terminals();
    _nonterminals = grammar.nonterminals();
    initialize_tables(sm.states().size());

    for (auto&& state : sm.states()) {
      for (auto&& item : state.items()) {
        lr1_insert(state, item, state.transitions(), grammar);
      }
    }
  }

 protected:
  void lr1_insert(const typename StateMachine::State& state,
                  const typename StateMachine::Item& item,
                  const unordered_map<Symbol, size_t>& transitionMap,
                  const TranslationGrammar& grammar) {
    using namespace std::literals;

    size_t id = state.id();
    auto&& rule = item.rule();
    auto&& mark = item.mark();
    // special S' -> S.EOF item
    if (rule == grammar.starting_rule() && mark == 1) {
      lr_action_item(id, Symbol::eof()) = {LRAction::SUCCESS, 0};
    } else if (mark == rule.input().size()) {
      for (auto&& terminal : item.lookaheads().symbols()) {
        auto& action = lr_action_item(id, terminal);
        if (action.action != LRAction::ERROR) {
          action = conflict_resolution(
            terminal, {LRAction::REDUCE, rule.id}, action, rule, state, grammar);
        } else {
          // regular insert
          lr_action_item(id, terminal) = {LRAction::REDUCE, rule.id};
        }
      }
    } else if (rule.input()[mark].nonterminal()) {
      // marked nonterminal
      auto&& nonterminal = rule.input()[mark];
      size_t nextState = transitionMap.at(nonterminal);
      lr_goto_item(id, nonterminal) = nextState;
    } else {
      // marked terminal
      auto&& terminal = rule.input()[mark];
      size_t nextState = transitionMap.at(terminal);
      auto& action = lr_action_item(id, terminal);
      if (action.action == LRAction::REDUCE) {
        action = conflict_resolution(terminal,
                                     action,
                                     {LRAction::SHIFT, nextState},
                                     grammar.rules()[action.argument],
                                     state,
                                     grammar);
      } else {
        // regular insert
        lr_action_item(id, terminal) = {LRAction::SHIFT, nextState};
      }
    }
  }

  LRActionItem conflict_resolution(const Symbol terminal,
                                   const LRActionItem& reduceItem,
                                   const LRActionItem& item,
                                   const TranslationGrammar::Rule& reduceRule,
                                   const typename StateMachine::State& state,
                                   const TranslationGrammar& grammar) {
    using namespace std::literals;
    // R/R conflict: select rule defined first in the grammar
    if (item.action == LRAction::REDUCE) {
      return (reduceItem.argument <= item.argument) ? reduceItem : item;
    }
    // S/R conflict:
    auto [associativity, precedence] = grammar.precedence(terminal);
    auto precedence2 = std::get<1>(grammar.precedence(reduceRule.precedence_symbol()));
    if (precedence == precedence2) {
      switch (associativity) {
        case Associativity::LEFT:
          // left associative, same precedence :> favor reduce
          return reduceItem;
        case Associativity::RIGHT:
          // right associative, same precedence :> favor shift
          return item;
        case Associativity::NONE:
          // not associative, same precedence :> error
          throw std::invalid_argument("Unresolvable S/R conflict on "s + terminal.to_string() +
                                      " in state " + state.to_string() + ".");
      }
    } else if (precedence < precedence2) {
      // terminal higher precedence :> favor shift
      return item;
    }
    // terminal lower precedence :> favor reduce
    return reduceItem;
  }
};

template <typename StateMachine, const char* type>
class LR1StrictGenericTable : public LRGenericTable {
 public:
  LR1StrictGenericTable() {}
  LR1StrictGenericTable(const TranslationGrammar& grammar) {
    StateMachine sm(grammar);
    _terminals = grammar.terminals();
    _nonterminals = grammar.nonterminals();
    initialize_tables(sm.states().size());

    for (auto&& state : sm.states()) {
      for (auto&& item : state.items()) {
        lr1_insert(state.id(), item, state.transitions(), grammar);
      }
    }
  }

 protected:
  void lr1_insert(size_t state,
                  const typename StateMachine::Item& item,
                  const unordered_map<Symbol, size_t>& transitionMap,
                  const TranslationGrammar& grammar) {
    using namespace std::literals;
    auto&& rule = item.rule();
    auto&& mark = item.mark();
    // special S' -> S.EOF item
    if (rule == grammar.starting_rule() && mark == 1) {
      lr_action_item(state, Symbol::eof()) = {LRAction::SUCCESS, 0};
    } else if (mark == rule.input().size()) {
      for (auto&& terminal : item.lookaheads().symbols()) {
        if (lr_action(state, terminal).action != LRAction::ERROR) {
          throw std::invalid_argument("Translation grammar is not "s + type);
        }
        lr_action_item(state, terminal) = {LRAction::REDUCE, rule.id};
      }
    } else if (rule.input()[mark].nonterminal()) {
      auto&& nonterminal = rule.input()[mark];
      size_t nextState = transitionMap.at(nonterminal);
      lr_goto_item(state, nonterminal) = nextState;
    } else {
      auto&& terminal = rule.input()[mark];
      size_t nextState = transitionMap.at(terminal);
      auto&& action = lr_action(state, terminal);
      if (action.action != LRAction::ERROR && action != LRActionItem{LRAction::SHIFT, nextState}) {
        throw std::invalid_argument("Translation grammar is not "s + type);
      }
      lr_action_item(state, terminal) = {LRAction::SHIFT, nextState};
    }
  }
};

inline char CanonicalLR1String[] = "Canonical LR(1)";
inline char LALRString[] = "LALR";
inline char LSCELRString[] = "LSCELR";
inline char StrictCanonicalLR1String[] = "Strict Canonical LR(1)";
inline char StrictLALRString[] = "Strict LALR";

using LR1Table = LR1GenericTable<lr1::StateMachine, CanonicalLR1String>;
using LALRTable = LR1GenericTable<lalr::StateMachine, LALRString>;
using LSCELRTable = LR1GenericTable<lscelr::StateMachine, LSCELRString>;

using LR1StrictTable = LR1StrictGenericTable<lr1::StateMachine, StrictCanonicalLR1String>;
using LALRStrictTable = LR1StrictGenericTable<lalr::StateMachine, StrictLALRString>;

}  // namespace ctf
#endif
/*** End of file lr_table.hpp ***/