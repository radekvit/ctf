/**
\file lr_translation_control.hpp
\brief Defines class LRTranslationControl and defines its methods.
\author Radek Vít
*/
#ifndef CTF_LR_TRANSLATION_CONTROL_H
#define CTF_LR_TRANSLATION_CONTROL_H

#include "ctf_lr_base.hpp"
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
  \brief Empty set for each nonterminal.
  */
  empty_t empty_;
  /**
  \brief First set for each nonterminal.
  */
  first_t first_;
  /**
  \brief Follow set for each nonterminal.
  */
  follow_t follow_;

  /**
  \brief Creates all predictive sets.
  */
  void create_predictive_sets() {
    empty_ = create_empty(*translationGrammar_);
    first_ = create_first(*translationGrammar_, empty_);
    follow_ = create_follow(*translationGrammar_, empty_, first_);
  }

  /**
  \brief Creates iterator attribute actions for incoming terminals.

  \param[in] obegin Iterator to the first Symbol of the output of the applied
  Rule.
  \param[in] targets Indices of the target actions for all input terminals.
  \param[out] attributeActions Targets to append incoming terminal's attributes.

  The added iterators point to input terminal attribute targets.
  */
  void create_attibute_actions() {
    // TODO
  }

  void set_error() { errorFlag_ = true; }

  void add_error(const Symbol& top,
                 const Symbol& token,
                 const Symbol& lastDerivedNonterminal) {
    set_error();
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
  using LRTranslationControlGeneral::LRTranslationControlGeneral;

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
    vector<size_t> appliedRules;
    // last derived nonterminal
    Symbol lastDerivedNonterminal = translationGrammar_->starting_symbol();

    pushdown.push_back(state);

    Symbol token = next_token();

    while (1) {
      switch (auto&& item = lrTable_.lr_action(state, token); item.type) {
        case LRActionType::SHIFT:
          state = item.argument;
          pushdown.push_back(state);
          token = next_token();
          break;
        case LRActionType::REDUCE: {
          auto&& rule = translationGrammar_->rules()[item.argument];
          // TODO: check popped symbols? necessary?
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
          // TODO output from productions
          produce_output(appliedRules);
          break;
        case LRActionType::ERROR:
          // TODO error
          break;
      }
    }
  }

  void produce_output(const vector<size_t> appliedRules) {
    using Type = Symbol::Type;

    tstack<vector<tstack<Symbol>::iterator>> attributeActions;

    input_.push(Symbol::eof());
    input_.push(translationGrammar_->starting_symbol());
    output_.push(Symbol::eof());
    output_.push(translationGrammar_->starting_symbol());
    size_t tokenIndex = 0;

    auto obegin = output_.begin();

    for (auto&& ruleIndex : reverse(appliedRules)) {
      // roll over all terminals on top and apply attribute actions
      while (1) {
        const Symbol& top = input_.top();
        if (top.type() != Type::TERMINAL) {
          break;
        }
        const Symbol& token = tokens_[tokenIndex];
        for (auto it : attributeActions.pop()) {
          it->set_attribute(token);
        }
        input_.pop();
        ++tokenIndex;
      }
      const Symbol& top = input_.top();

      // top nonterminal production
      auto& rule = translationGrammar_->rules()[ruleIndex];

      obegin = output_.replace(top, rule.output(), obegin);
      input_.replace(input_.begin(), rule.input());
      create_attibute_actions(obegin, rule.actions(), attributeActions);
    }
    // go over remaining terminals
    while (1) {
      const Symbol& top = input_.top();
      if (top.type() != Type::TERMINAL) {
        break;
      }
      const Symbol& token = tokens_[tokenIndex];
      for (auto it : attributeActions.pop()) {
        it->set_attribute(token);
      }
      input_.pop();
      ++tokenIndex;
    }
  }

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

  vector<Symbol> tokens_;

  /**
  Creates all predictive sets and creates a new LR table.
  */
  void create_lr_table() {
    create_predictive_sets();

    lrTable_ = LRTableType(*translationGrammar_, follow_);
  }

  Symbol next_token() override {
    tokens_.push_back(TranslationControl::next_token());
    return tokens_.back();
  }
};

using SLRTranslationControl = LRTranslationControlTemplate<SLRTable>;

}  // namespace ctf
#endif
/*** End of file ll_translation_control.hpp ***/