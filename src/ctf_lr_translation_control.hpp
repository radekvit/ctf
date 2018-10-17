/**
\file lr_translation_control.hpp
\brief Defines class LRTranslationControl and defines its methods.
\author Radek Vít
*/
#ifndef CTF_LR_TRANSLATION_CONTROL_H
#define CTF_LR_TRANSLATION_CONTROL_H

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
  \param[out] attributeActions Targets to append incoming terminal's attributes.

  The added iterators point to input terminal attribute targets.
  */
  void create_attibute_actions(
      tstack<Symbol>::iterator obegin,
      const vector<set<size_t>>& targets,
      tstack<vector<tstack<Symbol>::iterator>>& attributeActions) {
    for (auto& target : reverse(targets)) {
      vector<tstack<Symbol>::iterator> iterators;
      for (auto& i : target) {
        auto oit = obegin;
        for (size_t x = 0; x < i; ++x)
          ++oit;
        if (oit->type() == Symbol::Type::TERMINAL)
          iterators.push_back(oit);
      }
      attributeActions.push(iterators);
    }
  }

  void set_error() { errorFlag_ = true; }

  void add_error(const Symbol& top,
                 const Symbol& token,
                 const Symbol& lastDerivedNonterminal) {
    set_error();
    (void)top;
    (void)token;
    (void)lastDerivedNonterminal;
    string message{"parsing error"};
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

    Symbol token = next_token();

    while (true) {
      switch (auto&& item = lrTable_.lr_action(state, token); item.type) {
        case LRActionType::SHIFT:
          state = item.argument;
          pushdown.push_back(state);
          token = next_token();
          break;
        case LRActionType::REDUCE: {
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
        case LRActionType::SUCCESS:
          appliedRules.push_back(translationGrammar_->rules().size() - 1);
          produce_output(appliedRules);
          return;
        case LRActionType::ERROR:
          set_error();
          // TODO error message
          return;
      }
    }
  }

  /**
   * Iterates over reversed rules and applies them in a top-down manner.
   */
  void produce_output(const vector<size_t> appliedRules) {
    tstack<vector<tstack<Symbol>::iterator>> attributeActions;

    input_.push(Symbol::eof());
    output_.push(Symbol::eof());
    output_.push(translationGrammar_->starting_symbol());
    size_t tokenIndex = 0;

    auto obegin = output_.begin();

    for (auto&& ruleIndex : reverse(appliedRules)) {
      auto& rule = translationGrammar_->rules()[ruleIndex];

      obegin = output_.replace_last(rule.nonterminal(), rule.output(), obegin);
      create_attibute_actions(obegin, rule.actions(), attributeActions);
    }
    // apply attribute actions
    for (const Symbol& token : tokens_) {
      if (token == Symbol::eof()) {
        break;
      }
      for (auto it : attributeActions.pop()) {
        it->set_attribute(token);
      }
      ++tokenIndex;
    }
  }

  /**
  \brief Sets translation grammar.

  \param[in] tg The translation grammar for this translation.
  */
  void set_grammar(const TranslationGrammar& tg) override {
    translationGrammar_ = &tg;
    create_lr_table();
  }

 protected:
  /**
  \brief LR table used to control the translation.
  */
  LRTableType lrTable_;
  /**
  \brief All read tokens
  */
  vector<Symbol> tokens_;

  /**
  Creates all predictive sets and creates a new LR table.
  */
  void create_lr_table() { lrTable_ = LRTableType(*translationGrammar_); }

  Symbol next_token() override {
    tokens_.push_back(TranslationControl::next_token());
    return tokens_.back();
  }
};

using SLRTranslationControl = LRTranslationControlTemplate<SLRTable>;

}  // namespace ctf
#endif
/*** End of file ll_translation_control.hpp ***/