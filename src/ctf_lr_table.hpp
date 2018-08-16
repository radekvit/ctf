/**
\file lr_table.hpp
\brief Defines class LRTable and its methods.
\author Radek Vít
*/
#ifndef CRF_LR_TABLE_HPP
#define CRF_LR_TABLE_HPP

#include "ctf_base.hpp"
#include "ctf_lr_base.hpp"

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

  void initialize_tables(const vector<set<Item>>& states) {
    actionTable_ = {states.size() * terminalMap_.size(),
                    {LRActionType::ERROR, 0}};
    gotoTable_ = vector<size_t>(states.size() * nonterminalMap_.size(), 0);
  }
};

class SLRTable : public LRGenericTable {
 public:
  SLRTable(const TranslationGrammar& grammar, const follow_t& follow) {
    initialize_maps(grammar);
    // dummy rule for the grammar
    TranslationGrammar::Rule DUMMY_RULE{Symbol::eof(),
                                        {grammar.starting_symbol()}};
    // states
    vector<set<Item>> states{{Item{DUMMY_RULE, 0}.closure(grammar)}};
    // state transitions
    vector<unordered_map<Symbol, size_t>> transitions{{{}}};
    // create grammar machine
    for (size_t i = 0; i < states.size(); ++i) {
      auto&& state = states[i];
      // get all nonempty closures
      auto&& stateTransitions = symbol_skip_closures(grammar, state);
      // add transitions
      for (auto&& transitionPair : stateTransitions) {
        const Symbol& symbol = transitionPair.first;
        auto&& state = transitionPair.second;
        size_t j = insert_state(states, transitions, state);
        transitions[i][symbol] = j;
      }
    }
    initialize_tables(states);

    for (size_t i = 0; i < states.size(); ++i) {
      for (auto&& item : states[i]) {
        slr_insert(i, item, transitions[i], grammar, follow);
      }
    }
  }

 protected:
  size_t insert_state(vector<set<Item>>& states,
                      vector<unordered_map<Symbol, size_t>>& transitions,
                      const set<Item>& state) {
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
                  const Item& item,
                  const unordered_map<Symbol, size_t>& transitionMap,
                  const TranslationGrammar& grammar,
                  const follow_t& follow) {
    auto&& rule = item.rule();
    auto&& mark = item.mark();
    // special S' -> S. item
    if (rule.nonterminal() == Symbol::eof() && mark == 1) {
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
}  // namespace ctf
#endif
/*** End of file lr_table.hpp ***/