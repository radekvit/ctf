/**
\file lr_table.hpp
\brief Defines class LRTable and its methods.
\author Radek Vít
*/
#ifndef CRF_LR_TABLE_HPP
#define CRF_LR_TABLE_HPP

#include <istream>

#include "ctf_base.hpp"
#include "ctf_lr_lalr.hpp"
#include "ctf_lr_lr1.hpp"
#include "ctf_lr_lscelr.hpp"

namespace ctf {

enum class LRAction : unsigned char {
  ERROR = 0b00,
  SHIFT = 0b01,
  REDUCE = 0b10,
  SUCCESS = 0b11,
};

class LRActionItem {
 public:
  constexpr LRActionItem(LRAction action, size_t argument = 0) noexcept
    : _storage((argument & (std::numeric_limits<size_t>::max() >> 2)) |
               (static_cast<size_t>(action) << (8 * sizeof(size_t) - 2))) {}

  LRAction action() const noexcept {
    return static_cast<LRAction>(_storage >> (sizeof(size_t) * 8 - 2));
  }

  size_t argument() const noexcept { return (_storage << 2) >> 2; }

  friend bool operator==(const LRActionItem& lhs, const LRActionItem& rhs) {
    return lhs._storage == rhs._storage;
  }

  friend bool operator!=(const LRActionItem& lhs, const LRActionItem& rhs) { return !(lhs == rhs); }

 protected:
  size_t _storage;
};

class LRGenericTable {
 public:
  LRGenericTable() { initialize_tables(); }
  /*
  \brief Finds the record in the sorted subarray.
  */
  const LRActionItem& lr_action(size_t state, const Symbol& terminal) const {
    auto begin = _actionTable.begin() + _actionDelimiters[state];
    auto end = _actionTable.begin() + _actionDelimiters[state + 1];
    auto it = std::lower_bound(begin, end, Record<LRActionItem>{terminal.id(), {LRAction::ERROR}});
    if (it->key != terminal.id()) {
      return _errorItem;
    }
    return it->value;
  }

  size_t lr_goto(size_t state, const Symbol& nonterminal) const {
    auto begin = _gotoTable.begin() + _gotoDelimiters[state];
    auto end = _gotoTable.begin() + _gotoDelimiters[state + 1];
    auto it = std::lower_bound(begin, end, Record<size_t>{nonterminal.id(), 0});
    // this should always find the correct key
    assert(it->key == nonterminal.id());
    return it->value;
  }

  size_t states() const { return _states; }

  void save(std::ostream& os) const {
    os << _states << "\n";
    // save action table
    size_t j = 0;
    for (size_t i = 0; i < _states; ++i) {
      for (; j < _actionDelimiters[i + 1]; ++j) {
        auto& record = _actionTable[j];
        os << ' ' << record.key << ':';
        switch (record.value.action()) {
          case LRAction::ERROR:
            // this should never happen
            assert(false);
            break;
          case LRAction::SUCCESS:
            os << "S";
            break;
          case LRAction::SHIFT:
            os << 's' << record.value.argument();
            break;
          case LRAction::REDUCE:
            os << 'r' << record.value.argument();
            break;
        }
      }
      os << "\n";
    }
    // save goto table
    j = 0;
    for (size_t i = 0; i < _states; ++i) {
      for (; _gotoDelimiters.size() > i + 1 && j < _gotoDelimiters[i + 1]; ++j) {
        auto& record = _gotoTable[j];
        os << ' ' << record.key << ':' << record.value;
      }
      os << "\n";
    }
  }

 protected:
  template <typename T>
  struct Record {
    size_t key;
    T value;
    friend bool operator<(const Record& lhs, const Record& rhs) { return lhs.key < rhs.key; }
  };
  vector<Record<LRActionItem>> _actionTable;
  vector<size_t> _actionDelimiters;
  vector<Record<size_t>> _gotoTable;
  vector<size_t> _gotoDelimiters;

  size_t _states = 1;

  LRActionItem _errorItem = LRActionItem(LRAction::ERROR);

  LRActionItem& insert_action(size_t state, const Symbol& terminal) {
    // there will always be at least one action per state
    while (_actionDelimiters.size() < state + 2) {
      _actionDelimiters.push_back(_actionDelimiters.back());
    }
    assert(_actionDelimiters.size() == state + 2);
    auto begin = _actionTable.begin() + _actionDelimiters[state];
    auto end = _actionTable.begin() + _actionDelimiters[state + 1];
    auto it = std::lower_bound(begin, end, Record<LRActionItem>{terminal.id(), {LRAction::ERROR}});
    // found record
    if (it != _actionTable.end() && it->key == terminal.id()) {
      return it->value;
    }
    // insert new record
    ++_actionDelimiters[state + 1];
    return _actionTable.insert(it, {terminal.id(), LRActionItem(LRAction::ERROR)})->value;
  }

  void insert_goto(size_t state, const Symbol& nonterminal, size_t value) {
    while (_gotoDelimiters.size() < state + 2) {
      _gotoDelimiters.push_back(_gotoDelimiters.back());
    }
    assert(_gotoDelimiters.size() == state + 2);
    auto begin = _gotoTable.begin() + _gotoDelimiters[state];
    auto end = _gotoTable.begin() + _gotoDelimiters[state + 1];
    auto it = std::lower_bound(begin, end, Record<size_t>{nonterminal.id(), 0});
    // found record
    if (it != _gotoTable.end() && it->key == nonterminal.id()) {
      it->value = value;
      return;
    }
    // insert new record
    ++_gotoDelimiters[state + 1];
    _gotoTable.insert(it, {nonterminal.id(), value});
  }

  void initialize_tables() {
    _actionTable.clear();
    _actionDelimiters.clear();
    _gotoTable.clear();
    _gotoDelimiters.clear();
    _actionDelimiters.push_back(0);
    _actionDelimiters.push_back(0);
    _gotoDelimiters.push_back(0);
    _gotoDelimiters.push_back(0);
  }
};

template <typename StateMachine>
class LR1GenericTable : public LRGenericTable {
  // TODO: conflict resolution
 public:
  LR1GenericTable() {}
  LR1GenericTable(const TranslationGrammar& grammar, symbol_string_fn to_str = ctf::to_string) {
    StateMachine sm(grammar);
    _states = sm.states().size();

    for (auto& state : sm.states()) {
      for (auto& item : state.items()) {
        lr1_insert(state, item, state.transitions(), grammar, to_str);
      }
    }
  }

 protected:
  void lr1_insert(const typename StateMachine::State& state,
                  const typename StateMachine::Item& item,
                  const unordered_map<Symbol, size_t>& transitionMap,
                  const TranslationGrammar& grammar,
                  symbol_string_fn to_str = ctf::to_string) {
    using namespace std::literals;

    size_t id = state.id();
    auto& rule = item.rule();
    size_t mark = item.mark();
    // special S' -> S.EOF item
    if (rule == grammar.starting_rule() && mark == 1) {
      insert_action(id, Symbol::eof()) = {LRAction::SUCCESS};
    } else if (mark == rule.input().size()) {
      for (auto& terminal : item.lookaheads().symbols()) {
        auto& action = insert_action(id, terminal);
        if (action.action() != LRAction::ERROR) {
          action = conflict_resolution(
            terminal, {LRAction::REDUCE, rule.id}, action, rule, state, grammar, to_str);
        } else {
          // regular insert
          insert_action(id, terminal) = {LRAction::REDUCE, rule.id};
        }
      }
    } else if (rule.input()[mark].nonterminal()) {
      // marked nonterminal
      auto& nonterminal = rule.input()[mark];
      size_t nextState = transitionMap.at(nonterminal);
      insert_goto(id, nonterminal, nextState);
    } else {
      // marked terminal
      auto& terminal = rule.input()[mark];
      size_t nextState = transitionMap.at(terminal);
      auto& action = insert_action(id, terminal);
      if (action.action() == LRAction::REDUCE) {
        action = conflict_resolution(terminal,
                                     action,
                                     {LRAction::SHIFT, nextState},
                                     grammar.rules()[action.argument()],
                                     state,
                                     grammar,
                                     to_str);
      } else {
        // regular insert
        insert_action(id, terminal) = {LRAction::SHIFT, nextState};
      }
    }
  }

  LRActionItem conflict_resolution(const Symbol terminal,
                                   const LRActionItem& reduceItem,
                                   const LRActionItem& item,
                                   const TranslationGrammar::Rule& reduceRule,
                                   const typename StateMachine::State& state,
                                   const TranslationGrammar& grammar,
                                   symbol_string_fn to_str = ctf::to_string) {
    using namespace std::literals;
    // R/R conflict: select rule defined first in the grammar
    if (item.action() == LRAction::REDUCE) {
      return (reduceItem.argument() <= item.argument()) ? reduceItem : item;
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
          throw std::invalid_argument("S/R conflict on "s + to_str(terminal) +
                                      " with no associativity in state\n" +
                                      state.to_string(to_str) + ".");
      }
    } else if (precedence < precedence2) {
      // terminal higher precedence :> favor shift
      return item;
    }
    // terminal lower precedence :> favor reduce
    return reduceItem;
  }
};

template <typename StateMachine>
class LR1StrictGenericTable : public LRGenericTable {
 public:
  LR1StrictGenericTable() {}
  LR1StrictGenericTable(const TranslationGrammar& grammar,
                        symbol_string_fn to_str = ctf::to_string) {
    StateMachine sm(grammar);
    _states = sm.states().size();

    for (auto& state : sm.states()) {
      for (auto& item : state.items()) {
        lr1_insert(state, item, state.transitions(), grammar, to_str);
      }
    }
  }

 protected:
  void lr1_insert(const typename StateMachine::State& state,
                  const typename StateMachine::Item& item,
                  const unordered_map<Symbol, size_t>& transitionMap,
                  const TranslationGrammar& grammar,
                  symbol_string_fn to_str = ctf::to_string) {
    using namespace std::literals;
    auto& rule = item.rule();
    size_t mark = item.mark();
    // special S' -> S.EOF item
    if (rule == grammar.starting_rule() && mark == 1) {
      insert_action(state.id(), Symbol::eof()) = {LRAction::SUCCESS};
    } else if (mark == rule.input().size()) {
      for (auto& terminal : item.lookaheads().symbols()) {
        auto& action = insert_action(state.id(), terminal);
        if (action.action() != LRAction::ERROR) {
          throw std::invalid_argument(
            conflict_error_message(state, action.action(), LRAction::REDUCE, terminal, to_str));
        }
        action = {LRAction::REDUCE, rule.id};
      }
    } else if (rule.input()[mark].nonterminal()) {
      auto& nonterminal = rule.input()[mark];
      size_t nextState = transitionMap.at(nonterminal);
      insert_goto(state.id(), nonterminal, nextState);
    } else {
      auto& terminal = rule.input()[mark];
      size_t nextState = transitionMap.at(terminal);
      auto& action = insert_action(state.id(), terminal);
      if (action.action() != LRAction::ERROR &&
          action != LRActionItem{LRAction::SHIFT, nextState}) {
        throw std::invalid_argument(
          conflict_error_message(state, action.action(), LRAction::SHIFT, terminal, to_str));
      }
      action = {LRAction::SHIFT, nextState};
    }
  }

  string conflict_error_message(const typename StateMachine::State& state,
                                LRAction action1,
                                LRAction action2,
                                Symbol conflicted,
                                symbol_string_fn to_str) {
    string err;
    if (action1 == action2)
      err = "R/R conflict on ";
    else
      err = "S/R conflict on ";
    err += to_str(conflicted) + " in state " + state.to_string(to_str);
    return err;
  }
};

class LRSavedTable : public LRGenericTable {
 public:
  // ignore inicialization
  LRSavedTable() {}
  LRSavedTable(const TranslationGrammar&, symbol_string_fn = ctf::to_string) {}
  LRSavedTable(std::istream& is) {
    is >> _states;
    if (_states < 1) {
      throw std::invalid_argument("Invalid saved parsing table.");
    }
    // skip \n
    is.get();
    _actionDelimiters.pop_back();
    // initialize action table
    for (size_t i = 0; i < _states; ++i) {
      _actionDelimiters.push_back(_actionTable.size());
      while (true) {
        char c = is.get();
        if (c == '\n') {
          break;
        }

        size_t terminal = 0;
        is >> terminal;
        // skip :
        is.get();
        char action = is.get();
        if (action == 'S') {
          _actionTable.push_back({terminal, {LRAction::SUCCESS}});
        } else {
          size_t argument = 0;
          is >> argument;
          if (action == 'r') {
            _actionTable.push_back({terminal, {LRAction::REDUCE, argument}});
          } else if (action == 's') {
            _actionTable.push_back({terminal, {LRAction::SHIFT, argument}});
          }
        }
      }
    }
    // initialize goto table
    _gotoDelimiters.pop_back();
    for (size_t i = 0; i < _states; ++i) {
      _gotoDelimiters.push_back(_gotoTable.size());
      while (true) {
        char c = is.get();
        if (c == '\n') {
          break;
        }
        size_t nonterminal = 0;
        is >> nonterminal;
        // skip :
        is.get();
        size_t argument = 0;
        is >> argument;
        _gotoTable.push_back({nonterminal, argument});
      }
    }
  }
};

using LR1Table = LR1GenericTable<lr1::StateMachine>;
using LALRTable = LR1GenericTable<lalr::StateMachine>;
using LSCELRTable = LR1GenericTable<lscelr::StateMachine>;

using LR1StrictTable = LR1StrictGenericTable<lr1::StateMachine>;
using LALRStrictTable = LR1StrictGenericTable<lalr::StateMachine>;

}  // namespace ctf
#endif
/*** End of file lr_table.hpp ***/