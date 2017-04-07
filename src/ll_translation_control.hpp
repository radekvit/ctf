/**
\file ll_translation_control.hpp
\brief Defines class LLTranslationControl and defines its methods.
\author Radek Vít
*/
#ifndef CTF_LL_TRANSLATION_CONTROL
#define CTF_LL_TRANSLATION_CONTROL

#include "translation_control.hpp"

namespace ctf {

/**
\brief Implements LL top down translation control.
*/
class LLTranslationControl : public TranslationControl {
 protected:
  /**
  \brief Empty set for each nonterminal.
  */
  vector<bool> empty_;
  /**
  \brief First set for each nonterminal.
  */
  vector<vector<Symbol>> first_;
  /**
  \brief Follow set for each nonterminal.
  */
  vector<vector<Symbol>> follow_;
  /**
  \brief Predict set for each nonterminal.
  */
  vector<vector<Symbol>> predict_;

  /**
  \brief LL table used to control the translation.
  */
  LLTable llTable_;

  /**
  \brief String of terminals got by LexicalAnalyzer_.
  */
  vector<Symbol> inputString_;

  /**
  Creates all sets and creates a new LL table.
  */
  void create_ll_table() {
    create_empty();
    create_first();
    create_follow();
    create_predict();

    llTable_ = LLTable(*translationGrammar_, predict_);
  }

  /**
  \brief Creates Empty set for each nonterminal.
  */
  void create_empty() {
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
          switch (s.type()) {
            case Symbol::Type::TERMINAL:
              isempty = false;
              break;
            case Symbol::Type::NONTERMINAL:
              if (empty_[tg.nonterminal_index(s)] == false) {
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
  /**
  \brief Creates First set for each nonterminal.
  */
  void create_first() {
    const TranslationGrammar &tg = *translationGrammar_;
    first_ = {tg.nonterminals().size(), vector<Symbol>{}};

    bool changed = false;
    do {
      changed = false;
      for (auto &r : tg.rules()) {
        size_t i = tg.nonterminal_index(r.nonterminal());
        bool empty = true;
        for (auto &symbol : r.input()) {
          if (!empty)
            break;
          size_t nonterm_i;
          switch (symbol.type()) {
            case Symbol::Type::NONTERMINAL:
              nonterm_i = tg.nonterminal_index(symbol);
              if (modify_set(first_[i], first_[nonterm_i]))
                changed = true;
              empty = empty_[nonterm_i];
              break;
            case Symbol::Type::TERMINAL:
              if (modify_set(first_[i], vector<Symbol>({symbol})))
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
  /**
  \brief Creates Follow set for each nonterminal.
  */
  void create_follow() {
    const TranslationGrammar &tg = *translationGrammar_;
    follow_ = {tg.nonterminals().size(), vector<Symbol>{}};
    follow_[tg.nonterminal_index(tg.starting_symbol())].push_back(
        Symbol::eof());

    bool changed = false;
    do {
      changed = false;
      for (auto &r : tg.rules()) {
        // index of origin nonterminal
        size_t i = tg.nonterminal_index(r.nonterminal());
        /* empty set of all symbols to the right of the current one */
        bool compoundEmpty = true;
        /* first set of all symbols to the right of the current symbol */
        vector<Symbol> compoundFirst;
        /* track symbols from back */
        for (auto &s : reverse(r.input())) {
          // index of nonterminal in input string, only valid with
          // nonterminal symbol
          size_t ti = 0;
          switch (s.type()) {
            case Symbol::Type::NONTERMINAL:
              ti = tg.nonterminal_index(s);
              if (modify_set(follow_[ti], compoundFirst))
                changed = true;
              if (compoundEmpty && modify_set(follow_[ti], follow_[i]))
                changed = true;
              break;
            default:
              break;
          }
          /* if empty = false */
          if (s.type() != Symbol::Type::NONTERMINAL ||
              !empty_[tg.nonterminal_index(s)]) {
            compoundEmpty = false;
            switch (s.type()) {
              case Symbol::Type::NONTERMINAL:
                compoundFirst = first_[ti];
                break;
              case Symbol::Type::TERMINAL:
                compoundFirst = {s};
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
  /**
  \brief Creates Predict set for each nonterminal.
  */
  void create_predict() {
    predict_.clear();
    const TranslationGrammar &tg = *translationGrammar_;
    for (auto &r : tg.rules()) {
      vector<Symbol> compoundFirst;
      vector<Symbol> rfollow = follow_[tg.nonterminal_index(r.nonterminal())];
      bool compoundEmpty = true;
      for (auto &s : reverse(r.input())) {
        size_t i;
        switch (s.type()) {
          case Symbol::Type::TERMINAL:
            compoundEmpty = false;
            compoundFirst = vector<Symbol>({s});
            break;
          case Symbol::Type::NONTERMINAL:
            i = tg.nonterminal_index(s);
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

  /**
  \brief Creates iterator attribute actions for incoming terminals.
  \param[in] obegin Iterator to the first Symbol of the output of the applied
  Rule.
  \param[in] targets Indices of the target actions for all input terminals.
  \param[out] attributeActions Targets to append incoming terminal's attributes.
  */
  void create_attibute_actions(
      tstack<Symbol>::iterator obegin, const vector<vector<size_t>> &targets,
      tstack<vector<tstack<Symbol>::iterator>> &attributeActions) {
    for (auto &target : reverse(targets)) {
      vector<tstack<Symbol>::iterator> iterators;
      for (auto &i : target) {
        auto oit = obegin;
        for (size_t x = 0; x < i; ++x)
          ++oit;
        if (oit->type() == Symbol::Type::TERMINAL)
          iterators.push_back(oit);
      }
      attributeActions.push(iterators);
    }
  }

 public:
  /**
  \brief Constructs a LLTranslationControl.
  */
  LLTranslationControl() = default;
  /**
  \brief Default destructor.
  */
  virtual ~LLTranslationControl() = default;
  /**
  \brief Constructs LLTranslationControl with a LexicalAnalyzer and
  TranslationGrammar.
  \param[in] la A reference to the lexical analyzer to be used to get tokens.
  \param[in] tg The translation grammar for this translation.
  */
  LLTranslationControl(LexicalAnalyzer &la, TranslationGrammar &tg) {
    set_grammar(tg);
    set_lexical_analyzer(la);
  }

  /**
  \brief Sets translation grammar.
  \param[in] tg The translation grammar for this translation.
  */
  virtual void set_grammar(const TranslationGrammar &tg) {
    translationGrammar_ = &tg;
    create_ll_table();
  }

  /**
  \brief Runs the translation. Output symbols are stored in output_.
  */
  virtual void run() {
    using Type = Symbol::Type;

    if (!lexicalAnalyzer_)
      throw TranslationException("No lexical analyzer was attached.");
    else if (!translationGrammar_)
      throw TranslationException("No translation grammar was attached.");

    input_.clear();
    output_.clear();
    inputString_.clear();
    vector<const Rule *> rules;
    tstack<vector<tstack<Symbol>::iterator>> attributeActions;

    Symbol token = next_token(inputString_);

    input_.push(Symbol::eof());
    output_.push(Symbol::eof());
    input_.push(translationGrammar_->starting_symbol());
    output_.push(translationGrammar_->starting_symbol());
    while (1) {
      Symbol &top = input_.top();
      size_t ruleIndex;
      switch (top.type()) {
        case Type::EOF:
          if (token.type() == Type::EOF)
            return;
          else
            throw SyntaxError("Unexpected token after derivation is done.");
          break;
        case Type::TERMINAL:
          if (top == token) {
            for (auto it : attributeActions.pop()) {
              it->attribute() += token.attribute();
            }
            input_.pop();
            token = next_token(inputString_);
          } else {
            throw SyntaxError("Unexpected token " + token.name() +
                              ", expected " + top.name() + ".");
          }
          break;
        case Type::NONTERMINAL:
          ruleIndex = llTable_.rule_index(top, token);
          if (ruleIndex < translationGrammar_->rules().size()) {
            auto &rule = translationGrammar_->rules()[ruleIndex];
            input_.replace(input_.begin(), rule.input());
            auto obegin = output_.replace(top, rule.output());
            create_attibute_actions(obegin, rule.actions(), attributeActions);
            rules.push_back(&(rule));
          } else {
            throw SyntaxError(syntaxErrorMessage_(top, token) + ".");
          }
          break;
        default:
          break;
      }
    }
  }
};
}  // namespace ctf
#endif
/*** End of file ll_translation_control.hpp ***/