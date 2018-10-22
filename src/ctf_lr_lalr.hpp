#ifndef CTF_LR_LALR_HPP
#define CTF_LR_LALR_HPP

#include "ctf_lr_lr0.hpp"

namespace std {
template <>
struct hash<tuple<size_t, ctf::Symbol>> {
  size_t operator()(const tuple<size_t, ctf::Symbol>& t) const noexcept {
    const size_t i = get<0>(t);
    const ctf::Symbol& s = get<1>(t);
    return hash<size_t>{}(i) ^ hash<ctf::Symbol>{}(s);
  }
};

template <>
struct hash<tuple<tuple<size_t, size_t>, tuple<size_t, ctf::Symbol>>> {
  size_t operator()(
      const tuple<tuple<size_t, size_t>, tuple<size_t, ctf::Symbol>>& t) const
      noexcept {
    const size_t i = get<0>(get<0>(t));
    const size_t j = get<0>(get<1>(t));
    const size_t k = get<1>(get<0>(t));
    const ctf::Symbol& s = get<1>(get<1>(t));
    return hash<size_t>{}(i + k) ^ hash<size_t>{}(j) + hash<ctf::Symbol>{}(s);
  }
};
}  // namespace std

namespace ctf::lalr {
template <typename X, typename T>
unordered_map<X, set<T>> digraph(const set<tuple<X, X>>& r,
                                 const unordered_map<X, set<T>>& f,
                                 size_t xsize,
                                 std::function<size_t(const X&)> index) {
  unordered_map<X, set<T>> ff;
  stack<X> stack;
  vector<size_t> n(xsize, 0);
  std::function<void(const X&, const set<T>)> traverse;
  traverse = [&](const X& x, const set<T>& fx) {
    stack.push(x);
    size_t d = stack.size();
    size_t xi = index(x);
    n[xi] = d;
    ff[x] = fx;
    for (auto&& [xx, y] : r) {
      if (xx != x) {
        continue;
      }
      size_t yi = index(y);
      if (n[yi] == 0) {
        traverse(y, f.at(y));
      }
      if (n[yi] < n[xi])
        n[xi] = n[yi];
      ff[x] = set_union(ff[x], ff[y]);
    }
    if (n[xi] == d) {
      while (true) {
        auto&& s = stack.top();
        n[index(s)] = std::numeric_limits<size_t>::max();
        ff[s] = ff[x];
        if (s == x) {
          stack.pop();
          break;
        }
        stack.pop();
      }
    }
  };
  for (auto&& [x, fx] : f) {
    if (n[index(x)] == 0) {
      traverse(x, fx);
    }
  }
  return ff;
}

inline unordered_map<tuple<size_t, Symbol>, set<Symbol>> get_direct_reads(
    const LR0StateMachine& sm) {
  unordered_map<tuple<size_t, Symbol>, set<Symbol>> directReads{{}};
  for (size_t state = 0; state < sm.states().size(); ++state) {
    for (auto&& transitionPair : sm.transitions()[state]) {
      auto& symbol = transitionPair.first;
      auto& nextState = transitionPair.second;
      if (symbol.type() != Symbol::Type::NONTERMINAL) {
        continue;
      }
      for (auto&& transitionPair : sm.transitions()[nextState]) {
        auto& nextSymbol = transitionPair.first;
        switch (auto type = nextSymbol.type(); type) {
          case Symbol::Type::EOI:
          case Symbol::Type::TERMINAL:
            directReads[{state, symbol}].insert(nextSymbol);
            break;
          default:
            break;
        }
      }
    }
  }
  return std::move(directReads);
}

inline set<tuple<tuple<size_t, Symbol>, tuple<size_t, Symbol>>> get_reads(
    const LR0StateMachine& sm,
    const TranslationGrammar& grammar,
    const empty_t& empty) {
  set<tuple<tuple<size_t, Symbol>, tuple<size_t, Symbol>>> reads;
  for (size_t state = 0; state < sm.states().size(); ++state) {
    for (auto&& transitionPair : sm.transitions()[state]) {
      auto& symbol = transitionPair.first;
      auto& nextState = transitionPair.second;
      if (symbol.type() != Symbol::Type::NONTERMINAL) {
        continue;
      }
      for (auto&& transitionPair : sm.transitions()[nextState]) {
        auto& nextSymbol = transitionPair.first;
        switch (auto type = nextSymbol.type(); type) {
          case Symbol::Type::NONTERMINAL:
            if (empty[grammar.nonterminal_index(nextSymbol)]) {
              reads.insert(
                  tuple{tuple{state, symbol}, tuple{nextState, nextSymbol}});
            }
            break;
          default:
            break;
        }
      }
    }
  }
  return std::move(reads);
}

inline set<tuple<tuple<size_t, Symbol>, tuple<size_t, Symbol>>> get_includes(
    const LR0StateMachine& sm,
    const TranslationGrammar& grammar,
    const empty_t& empty) {
  // includes relation
  // all (s1, Symbol1), (s2, Symbol2) such that
  // 1: Symbol2 -> x Symbol1 yz
  // 2: empty(yz)
  // 3: x = x1 x2 ... xn
  //    s2 -x1-x2-x3...xn-> s1
  set<tuple<tuple<size_t, Symbol>, tuple<size_t, Symbol>>> includes;
  for (auto& rule : grammar.rules()) {
    auto& nonterminal = rule.nonterminal();
    const vector<Symbol>& input = rule.input();
    if (input.empty())
      continue;
    size_t i = input.size();
    // go left until the right side isn't empty
    for (auto&& symbol : reverse(input)) {
      if (symbol.type() == Symbol::Type::NONTERMINAL) {
        // add this to the relation
        vector<Symbol> symbols = {input.begin(), input.begin() + i};
        // try starting from all states
        for (size_t state = 0; state < sm.states().size(); ++state) {
          size_t finalState = state;
          for (auto&& inputSymbol : symbols) {
            auto it = sm.transitions()[finalState].find(inputSymbol);
            if (it == sm.transitions()[finalState].end()) {
              goto try_next_state;
            }
            finalState = it->second;
          }
          includes.insert(
              tuple{tuple{finalState, symbol}, tuple{state, nonterminal}});
        try_next_state:;
        }
        // break if this nonterminal isn't empty
        if (!empty[grammar.nonterminal_index(nonterminal)]) {
          break;
        }
      } else {
        break;
      }
      --i;
    }
  }
  return std::move(includes);
}

inline set<tuple<tuple<size_t, size_t>, tuple<size_t, Symbol>>> get_lookback(
    const LR0StateMachine& sm, const TranslationGrammar& grammar) {
  // lookback:
  // all pairs of (state, A -> x), (state2, A)
  // state2 -x1-x2-...xn-> state
  set<tuple<tuple<size_t, size_t>, tuple<size_t, Symbol>>> lookback;

  for (size_t i = 0; i < grammar.rules().size(); ++i) {
    auto&& rule = grammar.rules()[i];
    for (size_t state = 0; state < sm.states().size(); ++state) {
      size_t nextState = state;
      for (auto& symbol : rule.input()) {
        auto it = sm.transitions()[nextState].find(symbol);
        if (it == sm.transitions()[nextState].end()) {
          goto try_next_state;
        }
        nextState = it->second;
      }
      lookback.insert(
          tuple{tuple{nextState, i}, tuple{state, rule.nonterminal()}});
    try_next_state:;
    }
  }
  return std::move(lookback);
}

inline vector<vector<set<Symbol>>> get_la(
    size_t stateCount,
    size_t ruleCount,
    const unordered_map<tuple<size_t, Symbol>, set<Symbol>>& follow,
    const set<tuple<tuple<size_t, size_t>, tuple<size_t, Symbol>>>& lookback) {
  vector<vector<set<Symbol>>> la(stateCount, vector(ruleCount, set<Symbol>{}));
  for (auto&& tuple : lookback) {
    auto it = follow.find(std::get<1>(tuple));
    if (it == follow.end()) {
      continue;
    }
    auto i1 = std::get<0>(std::get<0>(tuple));
    auto i2 = std::get<1>(std::get<0>(tuple));
    la[i1][i2] = set_union(la[i1][i2], it->second);
  }
  return std::move(la);
}

}  // namespace ctf::lalr

namespace ctf {
struct LALRRelations {
  LALRRelations(const LR0StateMachine& sm,
                const TranslationGrammar& grammar,
                const empty_t& empty)
      : xsize(sm.states().size() *
              (grammar.terminals().size() + grammar.nonterminals().size()))
      , xmap({})
      , xmapper([&](const tuple<size_t, Symbol>& x) {
        auto it = xmap.find(x);
        if (it != xmap.end()) {
          return it->second;
        }
        auto& state = std::get<0>(x);
        auto& symbol = std::get<1>(x);
        size_t chunk =
            grammar.terminals().size() + grammar.nonterminals().size();
        size_t offset = grammar.nonterminals().size();
        if (symbol.type() == Symbol::Type::NONTERMINAL) {
          offset = grammar.nonterminal_index(symbol);
        } else {
          offset += grammar.terminal_index(symbol);
        }
        size_t result = state * chunk + offset;
        xmap[x] = result;
        return result;
      })
      , directReads(lalr::get_direct_reads(sm))
      , reads(lalr::get_reads(sm, grammar, empty))
      , includes(lalr::get_includes(sm, grammar, empty))
      , lookback(lalr::get_lookback(sm, grammar))
      , read(lalr::digraph(reads, directReads, xsize, xmapper))
      , follow(lalr::digraph(includes, read, xsize, xmapper))
      , la(lalr::get_la(
            sm.states().size(), grammar.rules().size(), follow, lookback)) {}

 private:
  size_t xsize;
  unordered_map<tuple<size_t, Symbol>, size_t> xmap;
  std::function<size_t(const tuple<size_t, Symbol>&)> xmapper;

 public:
  unordered_map<tuple<size_t, Symbol>, set<Symbol>> directReads;
  set<tuple<tuple<size_t, Symbol>, tuple<size_t, Symbol>>> reads;
  set<tuple<tuple<size_t, Symbol>, tuple<size_t, Symbol>>> includes;
  set<tuple<tuple<size_t, size_t>, tuple<size_t, Symbol>>> lookback;

  unordered_map<tuple<size_t, Symbol>, set<Symbol>> read;
  unordered_map<tuple<size_t, Symbol>, set<Symbol>> follow;

  vector<vector<set<Symbol>>> la;
};

class LALRStateMachine {
 public:
  LALRStateMachine(const TranslationGrammar& grammar, const empty_t& empty)
      : stateMachine_(grammar), relations_(stateMachine_, grammar, empty) {}

  const LR0StateMachine& state_machine() const { return stateMachine_; }
  const LALRRelations& relations() const { return relations_; }

 private:
  LR0StateMachine stateMachine_;
  LALRRelations relations_;
};
}  // namespace ctf
#endif