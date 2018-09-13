/**
\file lr_table.hpp
\brief Defines class LRTable and its methods.
\author Radek Vít
*/
#ifndef CRF_LR_TABLE_HPP
#define CRF_LR_TABLE_HPP

#include "ctf_base.hpp"
#include "ctf_lr_lr0.hpp"
#include "ctf_lr_lalr.hpp"

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
};

class LRGenericTable {
 public:
  const LRActionItem& lr_action(size_t state, const Symbol& terminal) const {
    return actionTable_[actionIndex(state, terminalMap_.at(terminal))];
  }

  const size_t& lr_goto(size_t state, const Symbol& nonterminal) const {
    return gotoTable_[gotoIndex(state, nonterminalMap_.at(nonterminal))];
  }

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

  LRActionItem& lr_action_item(size_t state, const Symbol& terminal) {
    return actionTable_[actionIndex(state, terminalMap_.at(terminal))];
  }

  size_t& lr_goto_item(size_t state, const Symbol& nonterminal) {
    return gotoTable_[gotoIndex(state, nonterminalMap_.at(nonterminal))];
  }

  /**
  \brief Maps 2D indices to 1D indices.
  */
  size_t actionIndex(size_t y, size_t x) const {
    return terminalMap_.size() * y + x;
  }
  /**
  \brief Maps 2D indices to 1D indices.
  */
  size_t gotoIndex(size_t y, size_t x) const {
    return nonterminalMap_.size() * y + x;
  }

  void initialize_maps(const TranslationGrammar& tg) {
    // create index maps for terminals and nonterminals
    for (size_t i = 0; i < tg.nonterminals().size(); ++i) {
      nonterminalMap_.insert(std::make_pair(tg.nonterminals()[i], i));
    }
    for (size_t i = 0; i < tg.terminals().size(); ++i) {
      terminalMap_.insert(std::make_pair(tg.terminals()[i], i));
    }
    terminalMap_.insert(std::make_pair(Symbol::eof(), tg.terminals().size()));
  }

  void initialize_tables(const vector<set<lr0::Item>>& states) {
    actionTable_ = {states.size() * terminalMap_.size(),
                    {LRActionType::ERROR, 0}};
    gotoTable_ = vector<size_t>(states.size() * nonterminalMap_.size(), 0);
  }
};

// TODO propably delete
class SLRTable : public LRGenericTable {
 public:
  SLRTable() {}
  SLRTable(const TranslationGrammar& grammar) {
    const empty_t empty = create_empty(grammar);
    const first_t first = create_first(grammar, empty);
    const follow_t follow = create_follow(grammar, empty, first);
    initialize_maps(grammar);
    LR0StateMachine sm(grammar);
    initialize_tables(sm.states);

    for (size_t i = 0; i < sm.states.size(); ++i) {
      for (auto&& item : sm.states[i]) {
        slr_insert(i, item, sm.transitions[i], grammar, follow);
      }
    }
  }

 protected:
  size_t insert_state(vector<set<lr0::Item>>& states,
                      vector<unordered_map<Symbol, size_t>>& transitions,
                      const set<lr0::Item>& state) {
    auto it = std::find(states.begin(), states.end(), state);
    if (it == states.end()) {
      // insert
      states.push_back(state);
      transitions.push_back({});
      return states.size() - 1;
    } else {
      return it - states.begin();
    }
  }

  void slr_insert(size_t state,
                  const lr0::Item& item,
                  const unordered_map<Symbol, size_t>& transitionMap,
                  const TranslationGrammar& grammar,
                  const follow_t& follow) {
    auto&& rule = item.rule();
    auto&& mark = item.mark();
    // special S' -> S. item
    if (rule.nonterminal() == grammar.starting_symbol() && mark == 1) {
      lr_action_item(state, Symbol::eof()) = {LRActionType::SUCCESS, 0};
    } else if (mark == rule.input().size()) {
      size_t ni = grammar.nonterminal_index(rule.nonterminal());
      for (auto&& terminal : follow[ni]) {
        if (lr_action(state, terminal).type != LRActionType::ERROR) {
          throw std::invalid_argument(
              "Constructing SLRTable from a non-SLR TranslationGrammar.");
        }
        lr_action_item(state, terminal) = {LRActionType::REDUCE, rule.id};
      }
    } else if (rule.input()[mark].type() == Symbol::Type::NONTERMINAL) {
      auto&& nonterminal = rule.input()[mark];
      size_t nextState = transitionMap.at(nonterminal);
      lr_goto_item(state, nonterminal) = nextState;
    } else if (rule.input()[mark].type() == Symbol::Type::TERMINAL) {
      auto&& terminal = rule.input()[mark];
      size_t nextState = transitionMap.at(terminal);
      if (lr_action(state, terminal).type != LRActionType::ERROR) {
        throw std::invalid_argument(
            "Constructing SLRTable from a non-SLR TranslationGrammar.");
      }
      lr_action_item(state, terminal) = {LRActionType::SHIFT, nextState};
    }
  }
};

class LALRTable : public LRGenericTable {
  LALRTable() {}
  LALRTable(const TranslationGrammar& grammar) {
    const empty_t empty = create_empty(grammar);
    LALRStateMachine sm(grammar, empty);
    // TODO
  }
};

}  // namespace ctf
#endif
/*** End of file lr_table.hpp ***/