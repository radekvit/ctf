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
inline string default_LL_error_message(
    const Symbol& top, const Symbol& token,
    [[maybe_unused]] const Symbol& lastDerivedNonterminal,
    [[maybe_unused]] bool empty, [[maybe_unused]] const set<Symbol>& first,
    [[maybe_unused]] const set<Symbol>& follow) {
  using Type = Symbol::Type;

  string errorString{};

  errorString += token.location().to_string() + ": ";
  switch (top.type()) {
    case Type::EOI:
      errorString +=
          "Unexpected token '" + token.name() + "' at the end of input.";
      break;
    case Type::TERMINAL:
      errorString += "Unexpected token '" + token.name() + "'; expected '" +
                     top.name() + "'";
      break;
    case Type::NONTERMINAL:
      errorString += "Unexpected token '" + token.name() +
                     "' "
                     "when deriving '" +
                     top.name() + "'; expected one of:\n";
      for (auto&& expected : first) {
        errorString += "'" + expected.name() + "', ";
      }
      if (empty) {
        for (auto&& expected : follow) {
          errorString += "'" + expected.name() + "', ";
        }
      }
      if (errorString.back() == ' ') {
        errorString.pop_back();
        errorString.pop_back();
      }
      errorString += '.';
      break;
    default:
      break;
  }
  return errorString;
}

class LLTranslationControlGeneral : public TranslationControl {
 public:
  using error_message_function = std::function<string(
      const Symbol& top, const Symbol& token, const Symbol& lastDerived,
      bool empty, const set<Symbol>& first, const set<Symbol>& follow)>;

  /**
  \brief Constructs a LLTranslationControlGeneral.
  */
  explicit LLTranslationControlGeneral(
      error_message_function error_message = default_LL_error_message)
      : error_function(error_message) {}
  /**
  \brief Constructs LLTranslationControlGeneral with a LexicalAnalyzer and
  TranslationGrammar.

  \param[in] la A reference to the lexical analyzer to be used to get tokens.
  \param[in] tg The translation grammar for this translation.
  */
  LLTranslationControlGeneral(
      LexicalAnalyzer& la, TranslationGrammar& tg,
      error_message_function error_message = default_LL_error_message)
      : error_function(error_message) {
    set_grammar(tg);
    set_lexical_analyzer(la);
  }

  /**
  \brief Default destructor.
  */
  virtual ~LLTranslationControlGeneral() = default;

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
  \brief The error message adding function
  */
  error_message_function error_function;

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

  void set_error_message_function(error_message_function error_message) {
    error_function = error_message;
  }

  void set_error() { errorFlag_ = true; }

  void add_error(const Symbol& top, const Symbol& token,
                 const Symbol& lastDerivedNonterminal) {
    set_error();
    string message;
    if (top.type() == Symbol::Type::NONTERMINAL) {
      // find predict and follow
      auto& nonterminals = translationGrammar_->nonterminals();
      size_t i = std::find(nonterminals.begin(), nonterminals.end(), top) -
                 nonterminals.begin();
      message = error_function(top, token, lastDerivedNonterminal, empty_[i],
                               first_[i], follow_[i]);
    } else {
      message =
          error_function(top, token, lastDerivedNonterminal, false, {}, {});
    }
    err() << top.location().to_string() << ": " << message << "\n";
  }
};

/**
\brief Implements LL top down translation control.
*/
template <typename LLTableType>
class LLTranslationControlTemplate : public LLTranslationControlGeneral {
 public:
  using LLTranslationControlGeneral::LLTranslationControlGeneral;

  /**
  \brief Runs the translation. Output symbols are stored in output_.
  */
  void run() override {
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
            add_error(top, token, lastDerivedNonterminal);
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
            add_error(top, token, lastDerivedNonterminal);
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
            add_error(top, token, lastDerivedNonterminal);
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

  /**
  \brief Sets translation grammar.

  \param[in] tg The translation grammar for this translation.
  */
  void set_grammar(const TranslationGrammar& tg) override {
    translationGrammar_ = &tg;
    create_ll_table();
  }

 protected:
  /**
  \brief LL table used to control the translation.
  */
  LLTableType llTable_;

  /**
  Creates all predictive sets and creates a new LL table.
  */
  void create_ll_table() {
    create_predictive_sets();

    llTable_ = LLTableType(*translationGrammar_, predict_);
  }

  /**
  \brief Hartmann error recovery.

  \param[out] token The next valid token.
  \returns True if the error recovery succeeded.
  */
  virtual bool error_recovery(
      const Symbol& lastDerivedNonterminal, Symbol& token,
      tstack<vector<tstack<Symbol>::iterator>>& attributeActions) {
    using Type = Symbol::Type;
    static const string recoveryMessage = "Recovering from syntax error...\n";

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
          return false;
        case Type::TERMINAL:
          if (top == token) {
            err() << recoveryMessage;
            return true;
          }
          attributeActions.pop();
          break;
        case Type::NONTERMINAL:
          ruleIndex = llTable_.rule_index(top, token);
          if (ruleIndex < translationGrammar_->rules().size()) {
            err() << recoveryMessage;
            return true;
          }
          break;
        default:
          break;
      }
      input_.pop();
    }

    // this should never happen
    throw std::runtime_error("Syntax error recovery failed.");
  }
};

using LLTranslationControl = LLTranslationControlTemplate<LLTable>;
using PriorityLLTranslationControl =
    LLTranslationControlTemplate<PriorityLLTable>;

class GeneralLLTranslationControl : public LLTranslationControlGeneral {
 public:
  using LLTableType = GeneralLLTable;

 protected:
  vector<Symbol> tokens_;

  size_t tokenPosition_ = 0;

  struct ParseState {
    tstack<Symbol> input;
    tstack<Symbol> output;
    size_t tokenPosition;
    set<size_t> rules;
    tstack<vector<tstack<Symbol>::iterator>> attributeActions;
  };

  tstack<ParseState> parseStates_;

  Symbol next_token() override {
    while (tokenPosition_ >= tokens_.size()) {
      tokens_.push_back(TranslationControl::next_token());
    }
    return tokens_[tokenPosition_++];
  }

  bool roll_back(tstack<vector<tstack<Symbol>::iterator>>& attributeActions,
                 tstack<Symbol>::iterator& obegin) {
    if (parseStates_.empty())
      return false;
    auto&& previousState = parseStates_.pop();
    input_ = previousState.input;
    output_ = previousState.output;
    tokenPosition_ = previousState.tokenPosition;
    attributeActions = previousState.attributeActions;

    auto& rule = translationGrammar_->rules()[*(previousState.rules.begin())];
    // a rule is now applicable
    if (previousState.rules.size() > 2) {
      previousState.rules.erase(previousState.rules.begin());
      parseStates_.push(previousState);
    }
    obegin = output_.replace(input_.top(), rule.output());
    input_.replace(input_.begin(), rule.input());
    create_attibute_actions(obegin, rule.actions(), attributeActions);
    return true;
  }

  void run() override {
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
      set<size_t> applicableRules;
      switch (top.type()) {
        case Symbol::Type::EOI:
          if (token == Symbol::eof()) {
            return;
          } else {
            set_error();
            add_error(top, token, lastDerivedNonterminal);
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
            if (!roll_back(attributeActions, obegin)) {
              set_error();
              add_error(top, token, lastDerivedNonterminal);
              if (!error_recovery(lastDerivedNonterminal, token,
                                  attributeActions))
                return;
            }
          }
          break;
        case Type::NONTERMINAL:
          applicableRules = llTable_.rule_index(top, token);
          if (!applicableRules.empty()) {
            // we derive this terminal
            lastDerivedNonterminal = top;
            auto& rule =
                translationGrammar_->rules()[*(applicableRules.begin())];

            if (applicableRules.size() > 1) {
              // store state
              applicableRules.erase(applicableRules.begin());
              parseStates_.push(ParseState{input_, output_, tokenPosition_,
                                           applicableRules, attributeActions});
            }

            obegin = output_.replace(top, rule.output(), obegin);
            input_.replace(input_.begin(), rule.input());
            create_attibute_actions(obegin, rule.actions(), attributeActions);
          } else {
            if (!roll_back(attributeActions, obegin)) {
              set_error();
              add_error(top, token, lastDerivedNonterminal);
              if (!error_recovery(lastDerivedNonterminal, token,
                                  attributeActions))
                return;
            }
          }
          break;
        default:
          // unexpected symbol type on input stack
          input_.pop();
          break;
      }
    }
  }

  /**
  \brief Sets translation grammar.

  \param[in] tg The translation grammar for this translation.
  */
  void set_grammar(const TranslationGrammar& tg) override {
    translationGrammar_ = &tg;
    create_ll_table();
  }

 protected:
  /**
  \brief LL table used to control the translation.
  */
  LLTableType llTable_;

  /**
  Creates all predictive sets and creates a new LL table.
  */
  void create_ll_table() {
    create_predictive_sets();

    llTable_ = LLTableType(*translationGrammar_, predict_);
  }

  virtual bool error_recovery(const Symbol&, Symbol&,
                              tstack<vector<tstack<Symbol>::iterator>>&) {
    return false;
  }
};

}  // namespace ctf
#endif
/*** End of file ll_translation_control.hpp ***/