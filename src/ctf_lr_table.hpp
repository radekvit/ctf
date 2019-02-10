/**
\file lr_table.hpp
\brief Defines class LRTable and its methods.
\author Radek Vít
*/
#ifndef CRF_LR_TABLE_HPP
#define CRF_LR_TABLE_HPP

#include "ctf_base.hpp"
#include "ctf_lr_lalr.hpp"
#include "ctf_lr_lr0.hpp"

namespace ctf {

enum class LRActionType {
  SHIFT,
  REDUCE,
  SUCCESS,
  ERROR,
};

struct LRActionItem {
  LRActionType type = LRActionType::ERROR;
  size_t argument = 0;

  friend bool operator==(const LRActionItem& lhs, const LRActionItem& rhs) {
    return lhs.type == rhs.type && lhs.argument == rhs.argument;
  }

  friend bool operator!=(const LRActionItem& lhs, const LRActionItem& rhs) { return !(lhs == rhs); }
};

class LRGenericTable {
 public:
  const LRActionItem& lr_action(size_t state, const Symbol& terminal) const {
    return actionTable_[actionIndex(state, terminalMap_.at(terminal))];
  }

  const size_t& lr_goto(size_t state, const Symbol& nonterminal) const {
    return gotoTable_[gotoIndex(state, nonterminalMap_.at(nonterminal))];
  }

  size_t total_states() const { return states_; }

 protected:
  vector<LRActionItem> actionTable_;
  vector<size_t> gotoTable_;

  /**
  \brief Mapping nonterminals to table_ indices.
  */
  unordered_map<Symbol, size_t> nonterminalMap_;
  /**
  \brief Mapping terminals to table_ row indices.
  */
  unordered_map<Symbol, size_t> terminalMap_;

  size_t states_;

  LRActionItem& lr_action_item(size_t state, const Symbol& terminal) {
    return actionTable_[actionIndex(state, terminalMap_.at(terminal))];
  }

  size_t& lr_goto_item(size_t state, const Symbol& nonterminal) {
    return gotoTable_[gotoIndex(state, nonterminalMap_.at(nonterminal))];
  }

  /**
  \brief Maps 2D indices to 1D indices.
  */
  size_t actionIndex(size_t y, size_t x) const { return terminalMap_.size() * y + x; }
  /**
  \brief Maps 2D indices to 1D indices.
  */
  size_t gotoIndex(size_t y, size_t x) const { return nonterminalMap_.size() * y + x; }

  void initialize_maps(const TranslationGrammar& tg) {
    // create index maps for terminals and nonterminals
    for (size_t i = 0; i < tg.nonterminals().size(); ++i) {
      nonterminalMap_.insert(std::make_pair(tg.nonterminals()[i], i));
    }
    // eof is a terminal in augmented grammars
    for (size_t i = 0; i < tg.terminals().size(); ++i) {
      terminalMap_.insert(std::make_pair(tg.terminals()[i], i));
    }
  }

  void initialize_tables(size_t size) {
    actionTable_ = {size * terminalMap_.size(), {LRActionType::ERROR, 0}};
    gotoTable_ = vector<size_t>(size * nonterminalMap_.size(), 0);

    states_ = size;
  }
};

class SLRTable : public LRGenericTable {
 public:
  SLRTable() {}
  SLRTable(const TranslationGrammar& grammar) {
    const empty_t empty = create_empty(grammar);
    const first_t first = create_first(grammar, empty);
    const follow_t follow = create_follow(grammar, empty, first);
    initialize_maps(grammar);
    LR0StateMachine sm(grammar);
    initialize_tables(sm.states().size());

    for (size_t i = 0; i < sm.states().size(); ++i) {
      for (auto&& item : sm.states()[i]) {
        slr_insert(i, item, sm.transitions()[i], grammar, follow);
      }
    }
  }

 protected:
  void slr_insert(size_t state,
                  const lr0::Item& item,
                  const unordered_map<Symbol, size_t>& transitionMap,
                  const TranslationGrammar& grammar,
                  const follow_t& follow) {
    auto&& rule = item.rule();
    auto&& mark = item.mark();
    // special S' -> S EOF. item
    if (rule == grammar.starting_rule() && mark == 2) {
      lr_action_item(state, Symbol::eof()) = {LRActionType::SUCCESS, 0};
    } else if (mark == rule.input().size()) {
      size_t ni = grammar.nonterminal_index(rule.nonterminal());
      for (auto&& terminal : follow[ni]) {
        if (lr_action(state, terminal).type != LRActionType::ERROR) {
          throw std::invalid_argument("Constructing SLRTable from a non-SLR TranslationGrammar.");
        }
        lr_action_item(state, terminal) = {LRActionType::REDUCE, rule.id};
      }
    } else if (rule.input()[mark].type() == Symbol::Type::NONTERMINAL) {
      auto&& nonterminal = rule.input()[mark];
      size_t nextState = transitionMap.at(nonterminal);
      lr_goto_item(state, nonterminal) = nextState;
    } else {
      auto&& terminal = rule.input()[mark];
      size_t nextState = transitionMap.at(terminal);
      if (lr_action(state, terminal).type != LRActionType::ERROR) {
        throw std::invalid_argument("Constructing SLRTable from a non-SLR TranslationGrammar.");
      }
      lr_action_item(state, terminal) = {LRActionType::SHIFT, nextState};
    }
  }
};
// TODO: conflict resolution
template <typename StateMachine, const char* type>
class LR1GenericTable : public LRGenericTable {
 public:
  LR1GenericTable() {}
  LR1GenericTable(const TranslationGrammar& grammar) {
    StateMachine sm(grammar);
    initialize_maps(grammar);
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
    if (rule == grammar.starting_rule() && mark == 2) {
      lr_action_item(state, Symbol::eof()) = {LRActionType::SUCCESS, 0};
    } else if (mark == rule.input().size()) {
      for (auto&& terminal : item.generated_lookaheads()) {
        if (lr_action(state, terminal).type != LRActionType::ERROR) {
          throw std::invalid_argument("Translation grammar is not "s + type);
        }
        lr_action_item(state, terminal) = {LRActionType::REDUCE, rule.id};
      }
    } else if (rule.input()[mark].type() == Symbol::Type::NONTERMINAL) {
      auto&& nonterminal = rule.input()[mark];
      size_t nextState = transitionMap.at(nonterminal);
      lr_goto_item(state, nonterminal) = nextState;
    } else {
      auto&& terminal = rule.input()[mark];
      size_t nextState = transitionMap.at(terminal);
      auto&& action = lr_action(state, terminal);
      if (action.type != LRActionType::ERROR &&
          action != LRActionItem{LRActionType::SHIFT, nextState}) {
        throw std::invalid_argument("Translation grammar is not "s + type);
      }
      lr_action_item(state, terminal) = {LRActionType::SHIFT, nextState};
    }
  }
};

inline char CanonicalLR1String[] = "Canonical LR(1)";
inline char LALRString[] = "LALR";

using LR1Table = LR1GenericTable<lr1::StateMachine, CanonicalLR1String>;
using LALRTable = LR1GenericTable<lalr::StateMachine, LALRString>;

}  // namespace ctf
#endif
/*** End of file lr_table.hpp ***/