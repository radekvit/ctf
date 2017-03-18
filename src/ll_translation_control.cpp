/**
\file ll_translation_control.cpp
\brief Implements methods for LLTranslationControl
\author Radek Vít
 */
#include <ll_translation_control.h>

namespace ctf {

LLTranslationControl::LLTranslationControl(LexicalAnalyzer &la,
                                           TranslationGrammar &tg) {
  set_grammar(tg);
  set_lexical_analyzer(la);
}

void LLTranslationControl::set_grammar(const TranslationGrammar &tg) {
  translationGrammar_ = &tg;
  create_ll_table();
}

void LLTranslationControl::run() {
  // TODO: set output attributes from inputString_
  using Type = Symbol::Type;

  if (!lexicalAnalyzer_)
    throw SyntacticError("No lexical analyzer was attached.");
  else if (!translationGrammar_)
    throw SyntacticError("No translation grammar was attached.");

  input_.clear();
  output_.clear();
  inputString_.clear();
  vector<const Rule *> rules;
  tstack<vector<tstack<Symbol>::iterator>> attributeTargets;

  Terminal token = next_token(inputString_);

  input_.push(Symbol::EOI());
  output_.push(Symbol::EOI());
  input_.push(translationGrammar_->starting_symbol());
  output_.push(translationGrammar_->starting_symbol());
  while (1) {
    Symbol &top = input_.top();
    size_t ruleIndex;
    switch (top.type) {
      case Type::EOI:
        if (token.name() == "")
          return;
        else
          throw SyntacticError("Unexpected token after derivation is done.");
        break;
      case Type::TERMINAL:
        if (top.terminal == token) {
          for (auto it : attributeTargets.pop()) {
            it->terminal.attribute() += token.attribute();
          }
          input_.pop();
          token = next_token(inputString_);
        } else {
          throw SyntacticError("Unexpected token.");
        }
        break;
      case Type::NONTERMINAL:
        ruleIndex = llTable_.rule_index(top.nonterminal, token);
        if (ruleIndex < translationGrammar_->rules().size()) {
          auto &rule = translationGrammar_->rules()[ruleIndex];
          input_.replace(input_.begin(), rule.input());
          auto obegin = output_.replace(top, rule.output());
          create_attibute_targets(obegin, rule.targets(), attributeTargets);
          rules.push_back(&(rule));
        } else {
          throw SyntacticError("No rule can be applied.");
        }
        break;
      default:
        break;
    }
  }
}

void LLTranslationControl::create_attibute_targets(
    tstack<Symbol>::iterator obegin, const vector<vector<size_t>> &targets,
    tstack<vector<tstack<Symbol>::iterator>> &attributeTargets) {
  for (auto &target : targets) {
    vector<tstack<Symbol>::iterator> iterators;
    for (auto &i : target) {
      auto oit = obegin;
      for (size_t x = 0; x < i; ++x) ++oit;
      iterators.push_back(oit);
    }
    attributeTargets.push(iterators);
  }
}

void LLTranslationControl::create_ll_table() {
  create_empty();
  create_first();
  create_follow();
  create_predict();

  llTable_ = LLTable(*translationGrammar_, predict_);
}

void LLTranslationControl::create_empty() {
  const TranslationGrammar &tg = *translationGrammar_;
  empty_ = vector<bool>(tg.nonterminals().size(), false);

  for (auto &r : tg.rules()) {
    if (r.input().size() == 0) {
      empty_[tg.nonterminal_index(r.nonterminal())] = true;
    }
  }

  bool changed = false;
  do {
    changed = false;
    for (auto &r : tg.rules()) {
      bool isempty = true;
      for (auto &s : r.input()) {
        switch (s.type) {
          case Symbol::Type::TERMINAL:
            isempty = false;
            break;
          case Symbol::Type::NONTERMINAL:
            if (empty_[tg.nonterminal_index(s.nonterminal)] == false) {
              isempty = false;
            }
            break;
          default:
            break;
        }
      }
      if (isempty) {
        if (!empty_[tg.nonterminal_index(r.nonterminal())]) {
          changed = true;
          empty_[tg.nonterminal_index(r.nonterminal())] = true;
        }
      }
    }
  } while (changed);
}

void LLTranslationControl::create_first() {
  const TranslationGrammar &tg = *translationGrammar_;
  first_ = {tg.nonterminals().size(), vector<Terminal>{}};

  bool changed = false;
  do {
    changed = false;
    for (auto &r : tg.rules()) {
      size_t i = tg.nonterminal_index(r.nonterminal());
      bool empty = true;
      for (auto &symbol : r.input()) {
        if (!empty) break;
        size_t nonterm_i;
        switch (symbol.type) {
          case Symbol::Type::NONTERMINAL:
            nonterm_i = tg.nonterminal_index(symbol.nonterminal);
            if (modify_set(first_[i], first_[nonterm_i])) changed = true;
            empty = empty_[nonterm_i];
            break;
          case Symbol::Type::TERMINAL:
            if (modify_set(first_[i], vector<Terminal>({symbol.terminal})))
              changed = true;
            empty = false;
            break;
          default:
            break;
        }
      }
    }
  } while (changed);
}

void LLTranslationControl::create_follow() {
  const TranslationGrammar &tg = *translationGrammar_;
  follow_ = {tg.nonterminals().size(), vector<Terminal>{}};
  follow_[tg.nonterminal_index(tg.starting_symbol().nonterminal)].push_back(
      Terminal::EOI());

  bool changed = false;
  do {
    changed = false;
    for (auto &r : tg.rules()) {
      // index of origin nonterminal
      size_t i = tg.nonterminal_index(r.nonterminal());
      /* empty set of all symbols to the right of the current one */
      bool compoundEmpty = true;
      /* first set of all symbols to the right of the current symbol */
      vector<Terminal> compoundFirst;
      /* track symbols from back */
      for (auto &s : reverse(r.input())) {
        // index of nonterminal in input string, only valid with
        // nonterminal symbol
        size_t ti = 0;
        switch (s.type) {
          case Symbol::Type::NONTERMINAL:
            ti = tg.nonterminal_index(s.nonterminal);
            if (modify_set(follow_[ti], compoundFirst)) changed = true;
            if (compoundEmpty && modify_set(follow_[ti], follow_[i]))
              changed = true;
            break;
          default:
            break;
        }
        /* if empty = false */
        if (s.type != Symbol::Type::NONTERMINAL ||
            !empty_[tg.nonterminal_index(s.nonterminal)]) {
          compoundEmpty = false;
          switch (s.type) {
            case Symbol::Type::NONTERMINAL:
              compoundFirst = first_[ti];
              break;
            case Symbol::Type::TERMINAL:
              compoundFirst = {s.terminal};
              break;
            default:
              break;
          }
        }
        /* empty = true, nonterminal*/
        else {
          modify_set(compoundFirst, first_[ti]);
        }
      }  // for all reverse input
    }    // for all rules
  } while (changed);
}

void LLTranslationControl::create_predict() {
  predict_.clear();
  const TranslationGrammar &tg = *translationGrammar_;
  for (auto &r : tg.rules()) {
    vector<Terminal> compoundFirst;
    vector<Terminal> rfollow = follow_[tg.nonterminal_index(r.nonterminal())];
    bool compoundEmpty = true;
    for (auto &s : reverse(r.input())) {
      size_t i;
      switch (s.type) {
        case Symbol::Type::TERMINAL:
          compoundEmpty = false;
          compoundFirst = vector<Terminal>({s.terminal});
          break;
        case Symbol::Type::NONTERMINAL:
          i = tg.nonterminal_index(s.nonterminal);
          if (!empty_[i]) {
            compoundEmpty = false;
            compoundFirst = first_[i];
          } else {
            modify_set(compoundFirst, first_[i]);
          }
        default:
          break;
      }
    }
    predict_.push_back(compoundFirst);

    if (compoundEmpty) {
      modify_set(predict_.back(), rfollow);
    }
  }  // for all rules
}

}  // namespace ctf

/*** End of file ll_translation_control.cpp ***/