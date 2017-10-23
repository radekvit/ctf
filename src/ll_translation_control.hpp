/**
\file ll_translation_control.hpp
\brief Defines class LLTranslationControl and defines its methods.
\author Radek Vít
*/
#ifndef CTF_LL_TRANSLATION_CONTROL_H
#define CTF_LL_TRANSLATION_CONTROL_H

#include "table_sets.hpp"
#include "translation_control.hpp"

namespace ctf {

/**
\brief Adds error message caused by a top symbol and incoming token
combination.

\param[in] top The current top symbol.
\param[in] token The incoming token.
*/
inline void default_LL_error_message(
    string& errorString, const Symbol& top, const Symbol& token,
    [[maybe_unused]] const Symbol& lastDerivedNonterminal) {
  using Type = Symbol::Type;

  errorString += token.location().to_string() + ": ";
  switch (top.type()) {
    case Type::EOI:
      errorString += "Unexpected token '" + token.name() +
                     "' after translation has finished.";
      break;
    case Type::TERMINAL:
      errorString += "Unexpected token '" + token.name() + "'; expected '" +
                     top.name() + "'";
      break;
    case Type::NONTERMINAL:
      errorString += "Unexpected token '" + token.name() + "', nonterminal '" +
                     top.name() + "'";
      break;
    default:
      break;
  }
  errorString += "\n";
}

/**
\brief Implements LL top down translation control.
*/
template <typename LLTableType>
class LLTranslationControlTemplate : public TranslationControl {
 public:
  using error_message_function =
      std::function<void(string&, const Symbol&, const Symbol&, const Symbol&)>;

 protected:
  /**
  \brief Empty set for each nonterminal.
  */
  empty_type empty_;
  /**
  \brief First set for each nonterminal.
  */
  first_type first_;
  /**
  \brief Follow set for each nonterminal.
  */
  follow_type follow_;
  /**
  \brief Predict set for each nonterminal.
  */
  predict_type predict_;

  /**
  \brief LL table used to control the translation.
  */
  LLTableType llTable_;

  /**
  \brief Error message string.
  */
  string errorString_;

  /**
  \brief The error message adding function
  */
  error_message_function add_error;
  /**
  Creates all predictive sets and creates a new LL table.
  */
  void create_ll_table() {
    create_predictive_sets();

    llTable_ = LLTableType(*translationGrammar_, predict_);
  }

  /**
  \brief Creates all predictive sets.
  */
  void create_predictive_sets() {
    empty_ = create_empty(*translationGrammar_);
    first_ = create_first(*translationGrammar_, empty_);
    follow_ = create_follow(*translationGrammar_, empty_, first_);
    predict_ = create_predict(*translationGrammar_, empty_, first_, follow_);
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
      tstack<Symbol>::iterator obegin, const vector<set<size_t>>& targets,
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

 public:
  /**
  \brief Constructs a LLTranslationControlTemplate.
  */
  explicit LLTranslationControlTemplate(
      error_message_function error_message = default_LL_error_message)
      : add_error(error_message) {}
  /**
  \brief Default destructor.
  */
  virtual ~LLTranslationControlTemplate() = default;
  /**
  \brief Constructs LLTranslationControlTemplate with a LexicalAnalyzer and
  TranslationGrammar.

  \param[in] la A reference to the lexical analyzer to be used to get tokens.
  \param[in] tg The translation grammar for this translation.
  */
  LLTranslationControlTemplate(
      LexicalAnalyzer& la, TranslationGrammar& tg,
      error_message_function error_message = default_LL_error_message)
      : add_error(error_message) {
    set_grammar(tg);
    set_lexical_analyzer(la);
  }

  /**
  \brief Sets translation grammar.

  \param[in] tg The translation grammar for this translation.
  */
  virtual void set_grammar(const TranslationGrammar& tg) {
    translationGrammar_ = &tg;
    create_ll_table();
  }

  void set_error_message(error_message_function error_message) {
    add_error = error_message;
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
    tstack<vector<tstack<Symbol>::iterator>> attributeActions;

    Symbol token = next_token();
    // last derived nonterminal
    Symbol lastDerivedNonterminal = translationGrammar_->starting_symbol();

    input_.push(Symbol::eof());
    output_.push(Symbol::eof());
    input_.push(translationGrammar_->starting_symbol());
    output_.push(translationGrammar_->starting_symbol());

    // iterator to the first symbol of the last inserted string
    // used to speed up output_ linear search
    auto obegin = output_.begin();

    while (1) {
      Symbol& top = input_.top();
      size_t ruleIndex;
      switch (top.type()) {
        case Symbol::Type::EOI:
          if (token == Symbol::eof()) {
            return;
          } else {
            set_error();
            add_error(errorString_, top, token, lastDerivedNonterminal);
            return;
          }
          break;
        case Type::TERMINAL:
          if (top == token) {
            for (auto it : attributeActions.pop()) {
              it->set_attribute(token);
            }
            input_.pop();
            token = next_token();
          } else {
            set_error();
            add_error(errorString_, top, token, lastDerivedNonterminal);
            if (!error_recovery(lastDerivedNonterminal, token,
                                attributeActions))
              return;
          }
          break;
        case Type::NONTERMINAL:
          ruleIndex = llTable_.rule_index(top, token);
          if (ruleIndex < translationGrammar_->rules().size()) {
            // we derive this terminal
            lastDerivedNonterminal = top;

            auto& rule = translationGrammar_->rules()[ruleIndex];

            obegin = output_.replace(top, rule.output(), obegin);
            input_.replace(input_.begin(), rule.input());
            create_attibute_actions(obegin, rule.actions(), attributeActions);
          } else {
            set_error();
            add_error(errorString_, top, token, lastDerivedNonterminal);
            if (!error_recovery(lastDerivedNonterminal, token,
                                attributeActions))
              return;
          }
          break;
        default:
          // unexpected symbol type on input stack
          input_.pop();
          break;
      }
    }
  }

  void set_error() { errorFlag_ = true; }

  /**
  \brief Hartmann error recovery.

  \param[out] token The next valid token.
  \returns True if the error recovery succeeded.
  */
  virtual bool error_recovery(
      const Symbol& lastDerivedNonterminal, Symbol& token,
      tstack<vector<tstack<Symbol>::iterator>>& attributeActions) {
    using Type = Symbol::Type;

    size_t ruleIndex = 0;
    size_t ntIndex =
        translationGrammar_->nonterminal_index(lastDerivedNonterminal);
    if (ntIndex >= translationGrammar_->nonterminals().size())
      return false;
    auto& ntFollow = follow_[ntIndex];
    // get a token from follow(lastNonterminal_)
    while (!is_in(ntFollow, token) && token != Symbol::eof()) {
      token = next_token();
    }
    // pop stack until a rule is applicable or the same token is on top
    while (true) {
      Symbol& top = input_.top();
      switch (top.type()) {
        case Type::EOI:
          return true;
        case Type::TERMINAL:
          if (top == token)
            return true;
          attributeActions.pop();
          break;
        case Type::NONTERMINAL:
          ruleIndex = llTable_.rule_index(top, token);
          if (ruleIndex < translationGrammar_->rules().size()) {
            return true;
          }
          break;
        default:
          break;
      }
      input_.pop();
    }

    // this should never happen
    return true;
  }
  /**
  \brief Get error message.

  \returns The error message string.
  */
  string error_message() { return errorString_; }
};

using LLTranslationControl = LLTranslationControlTemplate<LLTable>;
using PriorityLLTranslationControl =
    LLTranslationControlTemplate<PriorityLLTable>;
}  // namespace ctf
#endif
/*** End of file ll_translation_control.hpp ***/