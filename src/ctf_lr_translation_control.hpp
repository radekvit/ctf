/**
\file lr_translation_control.hpp
\brief Defines class LRTranslationControl and defines its methods.
\author Radek Vít
*/
#ifndef CTF_LR_TRANSLATION_CONTROL_H
#define CTF_LR_TRANSLATION_CONTROL_H
#include <iostream>
#include "ctf_lr_lalr.hpp"
#include "ctf_lr_lr0.hpp"
#include "ctf_lr_table.hpp"
#include "ctf_table_sets.hpp"
#include "ctf_translation_control.hpp"

namespace ctf {
class LRTranslationControlGeneral : public TranslationControl {
 public:
  /**
  \brief Constructs a LRTranslationControlGeneral.
  */
  explicit LRTranslationControlGeneral() {}
  /**
  \brief Constructs LRTranslationControlGeneral with a LexicalAnalyzer and
  TranslationGrammar.

  \param[in] la A reference to the lexical analyzer to be used to get tokens.
  \param[in] tg The translation grammar for this translation.
  */
  LRTranslationControlGeneral(LexicalAnalyzer& la, TranslationGrammar& tg) {
    set_grammar(tg);
    set_lexical_analyzer(la);
  }

  /**
  \brief Default destructor.
  */
  virtual ~LRTranslationControlGeneral() = default;

 protected:
  /**
  \brief Creates iterator attribute actions for incoming terminals.

  \param[in] obegin Iterator to the first Symbol of the output of the applied
  Rule.
  \param[in] targets Indices of the target actions for all input terminals.
  \param[in] outputSize The size of the output for target generation.
  \param[out] attributeActions Targets to append incoming terminal's attributes.

  The added iterators point to input terminal attribute targets.
  */
  void create_attibute_actions(tstack<Token>::iterator obegin,
                               const vector<vector_set<size_t>>& targets,
                               size_t outputSize,
                               tstack<vector<tstack<Token>::iterator>>& attributeActions) {
    for (auto& target : targets) {
      vector<tstack<Token>::iterator> iterators;
      for (auto& i : target) {
        auto oit = obegin;
        for (size_t x = 0; x < outputSize - i; ++x) {
          --oit;
        }
        if (oit->type() == Symbol::Type::TERMINAL || oit->type() == Symbol::Type::EOI)
          iterators.push_back(oit);
      }
      attributeActions.push(iterators);
    }
  }

  void set_error() { errorFlag_ = true; }

  void add_error(const Token& token, const string& message) {
    set_error();
    err() << token.location().to_string() << ": " << message << "\n";
  }

  /**
  \brief Placeholder error recovery.
  */
  virtual bool error_recovery() { return false; }
};  // namespace ctf

/**
\brief Implements LR bottom up translation control.
*/
template <typename LRTableType>
class LRTranslationControlTemplate : public LRTranslationControlGeneral {
 public:
  /**
  \brief Constructs a LRTranslationControlGeneral.
  */
  explicit LRTranslationControlTemplate() {}
  /**
  \brief Constructs LRTranslationControlGeneral with a LexicalAnalyzer and
  TranslationGrammar.

  \param[in] la A reference to the lexical analyzer to be used to get tokens.
  \param[in] tg The translation grammar for this translation.
  */
  LRTranslationControlTemplate(LexicalAnalyzer& la, TranslationGrammar& tg) {
    set_grammar(tg);
    set_lexical_analyzer(la);
  }

  /**
  \brief Runs the translation. Output symbols are stored in output_.
  */
  void run() override {
    if (!lexicalAnalyzer_)
      throw TranslationException("No lexical analyzer was attached.");
    else if (!translationGrammar_)
      throw TranslationException("No translation grammar was attached.");

    input_.clear();
    output_.clear();

    size_t state = 0;
    vector<size_t> pushdown;
    vector<size_t> appliedRules{};

    pushdown.push_back(state);

    Token token = next_token();

    while (true) {
      switch (auto&& item = lrTable_.lr_action(state, token.symbol()); item.action) {
        case LRAction::SHIFT:
          state = item.argument;
          pushdown.push_back(state);
          token = next_token();
          break;
        case LRAction::REDUCE: {
          auto&& rule = translationGrammar_->rules()[item.argument];
          for (size_t i = 0; i < rule.input().size(); ++i) {
            pushdown.pop_back();
          }
          const auto& stackState = pushdown.back();
          state = lrTable_.lr_goto(stackState, rule.nonterminal());
          pushdown.push_back(state);
          appliedRules.push_back(item.argument);
          break;
        }
        case LRAction::SUCCESS:
          appliedRules.push_back(translationGrammar_->rules().size() - 1);
          produce_output(appliedRules);
          return;
        case LRAction::ERROR:
          add_error(token, error_message(state, token));
          if (!error_recovery(state, token))
            return;
      }
    }
  }

  /**
   * Iterates over reversed rules and applies them in a top-down manner.
   */
  void produce_output(const vector<size_t>& appliedRules) {
    tstack<vector<tstack<Token>::iterator>> attributeActions;

    input_.push(translationGrammar_->starting_symbol());
    output_.push(translationGrammar_->starting_symbol());

    // remove extra eof
    tokens_.pop_back();

    auto obegin = output_.begin();
    auto tokenIt = tokens_.crbegin();
    for (auto&& ruleIndex : reverse(appliedRules)) {
      auto& rule = translationGrammar_->rules()[ruleIndex];
      input_.replace_last(rule.nonterminal(), rule.input());
      obegin = output_.replace_last(rule.nonterminal(), rule.output(), obegin);
      create_attibute_actions(obegin, rule.actions(), rule.output().size(), attributeActions);

      // apply attribute actions for all current rightmost terminals
      for (auto workingTerminalIt = input_.crbegin();
           workingTerminalIt != input_.crend() &&
           workingTerminalIt->type() != Symbol::Type::NONTERMINAL;
           ++tokenIt) {
        for (auto symbolIt : attributeActions.pop()) {
          symbolIt->set_attribute(*tokenIt);
        }
        input_.pop_bottom();
        workingTerminalIt = input_.crbegin();
      }
    }
    assert(attributeActions.empty());
  }

  /**
  \brief Sets translation grammar.

  \param[in] tg The translation grammar for this translation.
  */
  void set_grammar(const TranslationGrammar& tg) override {
    translationGrammar_ = &tg;
    create_lr_table();
  }

  // TODO allow example-based error messages
  string error_message(size_t state, const Token& token) {
    string message = "Unexpected symbol '";
    message += token.to_string();
    message += "'\nexpected one of:";
    if (lrTable_.lr_action(state, Symbol::eof()).action != LRAction::ERROR) {
      message += " EOF";
    }
    for (auto terminal = Symbol::eof(); terminal.id() < translationGrammar_->terminals();
         terminal = Terminal(terminal.id())) {
      if (lrTable_.lr_action(state, terminal).action != LRAction::ERROR) {
        message += " '";
        message += terminal.to_string() + "'";
      }
    }
    message += "\n";
    return std::move(message);
  }

  bool error_recovery(size_t state, const Token& token) {
    (void)state;
    (void)token;
    return false;
  }

 protected:
  /**
  \brief LR table used to control the translation.
  */
  LRTableType lrTable_;
  /**
  \brief All read tokens
  */
  vector<Token> tokens_;

  /**
  Creates all predictive sets and creates a new LR table.
  */
  void create_lr_table() { lrTable_ = LRTableType(*translationGrammar_); }

  Token next_token() override {
    tokens_.push_back(TranslationControl::next_token());
    return tokens_.back();
  }
};

using SLRTranslationControl = LRTranslationControlTemplate<SLRTable>;
using LALRTranslationControl = LRTranslationControlTemplate<LALRTable>;
using LR1TranslationControl = LRTranslationControlTemplate<LR1Table>;

using LALRStrictTranslationControl = LRTranslationControlTemplate<LALRStrictTable>;
using LR1StrictTranslationControl = LRTranslationControlTemplate<LR1StrictTable>;

}  // namespace ctf
#endif
/*** End of file ll_translation_control.hpp ***/