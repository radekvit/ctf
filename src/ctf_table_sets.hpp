#ifndef CTF_PREDICTIVE_SETS
#define CTF_PREDICTIVE_SETS

#include "ctf_translation_grammar.hpp"

namespace ctf {

using empty_t = vector<bool>;
using first_t = vector<TerminalSet>;
using follow_t = vector<TerminalSet>;
using predict_t = vector<TerminalSet>;

/**
\brief Creates Empty set for each nonterminal.

Empty is true if a series of productions from the nonterminal can result in an
empty string.
*/
inline empty_t create_empty(const TranslationGrammar& tg) {
  empty_t empty = empty_t(tg.nonterminals(), false);

  for (auto& r : tg.rules()) {
    if (r.input().size() == 0) {
      empty[r.nonterminal().id()] = true;
    }
  }

  bool changed = false;
  do {
    changed = false;
    for (auto& r : tg.rules()) {
      if (empty[r.nonterminal().id()]) {
        continue;
      }
      bool isempty = true;
      for (auto& s : r.input()) {
        switch (s.type()) {
          case Symbol::Type::EOI:
          case Symbol::Type::TERMINAL:
            isempty = false;
            break;
          case Symbol::Type::NONTERMINAL:
            if (!empty[s.id()]) {
              isempty = false;
            }
            break;
          default:
            break;
        }
      }
      if (isempty) {
        changed = true;
        empty[r.nonterminal().id()] = true;
      }
    }
  } while (changed);
  return empty;
}

/**
\brief Creates First set for each nonterminal.

First contains all characters that can be at the first position of any string
derived from this nonterminal.
*/
inline first_t create_first(const TranslationGrammar& tg, const empty_t& empty) {
  first_t first = {tg.nonterminals(), TerminalSet(tg.terminals())};

  bool changed = false;
  do {
    changed = false;
    for (auto& r : tg.rules()) {
      size_t i = r.nonterminal().id();
      bool isEmpty = true;
      for (auto& symbol : r.input()) {
        if (!isEmpty)
          break;
        size_t nonterm_i;
        switch (symbol.type()) {
          case Symbol::Type::NONTERMINAL:
            nonterm_i = symbol.id();
            changed |= first[i].set_union(first[nonterm_i]);
            isEmpty = empty[nonterm_i];
            break;
          case Symbol::Type::EOI:
          case Symbol::Type::TERMINAL:
            changed |= first[i].insert(symbol).inserted;
            isEmpty = false;
            break;
          default:
            break;
        }
      }
    }
  } while (changed);

  return first;
}

/**
\brief Creates Follow set for each nonterminal.

Follow contains all characters that may follow that nonterminal in a
sentential form from the starting nonterminal.
*/
inline follow_t create_follow(const TranslationGrammar& tg,
                              const empty_t& empty,
                              const first_t& first) {
  follow_t follow = {tg.nonterminals(), TerminalSet(tg.terminals())};
  follow[tg.starting_rule().input()[0].id()].insert(Symbol::eof());

  bool changed = false;
  do {
    changed = false;
    for (auto& r : tg.rules()) {
      // index of origin nonterminal
      size_t i = r.nonterminal().id();
      /* empty set of all symbols to the right of the current one */
      bool compoundEmpty = true;
      /* first set of all symbols to the right of the current symbol */
      TerminalSet compoundFirst(tg.terminals());
      /* track symbols from back */
      for (auto& s : reverse(r.input())) {
        // index of nonterminal in input string, only valid with
        // nonterminal symbol
        size_t ti = 0;
        switch (s.type()) {
          case Symbol::Type::NONTERMINAL:
            ti = s.id();
            changed |= follow[ti].set_union(compoundFirst);
            changed |= (compoundEmpty && follow[ti].set_union(follow[i]));
            break;
          default:
            break;
        }
        /* if !empty */
        if (s.type() != Symbol::Type::NONTERMINAL || !empty[s.id()]) {
          compoundEmpty = false;
          switch (s.type()) {
            case Symbol::Type::NONTERMINAL:
              compoundFirst = first[ti];
              break;
            case Symbol::Type::EOI:
            case Symbol::Type::TERMINAL:
              compoundFirst = TerminalSet(tg.terminals(), {s});
              break;
            default:
              break;
          }
        }
        /* empty == true, nonterminal*/
        else {
          compoundFirst |= first[ti];
        }
      }  // for all reverse input
    }    // for all rules
  } while (changed);

  return follow;
}

/**
\brief Creates Predict set for each nonterminal.

Predict contains all Terminals that may be the first terminal read in a
sentential form from that nonterminal.
*/
inline predict_t create_predict(const TranslationGrammar& tg,
                                const empty_t& empty,
                                const first_t& first,
                                const follow_t& follow) {
  predict_t predict;
  for (auto& r : tg.rules()) {
    TerminalSet compoundFirst(tg.terminals());
    TerminalSet rfollow = follow[r.nonterminal().id()];
    bool compoundEmpty = true;
    for (auto& s : reverse(r.input())) {
      size_t i;
      switch (s.type()) {
        case Symbol::Type::EOI:
        case Symbol::Type::TERMINAL:
          compoundEmpty = false;
          compoundFirst = TerminalSet(tg.terminals(), {s});
          break;
        case Symbol::Type::NONTERMINAL:
          i = s.id();
          if (!empty[i]) {
            compoundEmpty = false;
            compoundFirst = first[i];
          } else {
            compoundFirst |= first[i];
          }
      }
    }
    predict.push_back(compoundFirst);

    if (compoundEmpty) {
      predict.back() |= rfollow;
    }
  }  // for all rules
  return predict;
}

inline TerminalSet string_first(const std::vector<Symbol>& symbols,
                                const empty_t& empty,
                                const first_t& first,
                                const TranslationGrammar& tg) {
  using Type = Symbol::Type;
  TerminalSet result(tg.terminals());
  for (auto&& symbol : symbols) {
    switch (symbol.type()) {
      case Type::TERMINAL:
      case Type::EOI:
        result.insert(symbol);
        return result;
      case Type::NONTERMINAL: {
        size_t i = symbol.id();
        result |= first[i];
        if (!empty[i]) {
          return result;
        }
        break;
      }
      default:
        break;
    }
  }
  return result;
}

}  // namespace ctf

#endif