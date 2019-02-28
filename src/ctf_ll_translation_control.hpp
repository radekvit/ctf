/**
\file ll_translation_control.hpp
\brief Defines class LLTranslationControl and defines its methods.
\author Radek Vít
*/
#ifndef CTF_LL_TRANSLATION_CONTROL_H
#define CTF_LL_TRANSLATION_CONTROL_H

#include "ctf_table_sets.hpp"
#include "ctf_translation_control.hpp"

namespace ctf {

/**
\brief Adds error message caused by a top symbol and incoming token
combination.

\param[in] top The current top symbol.
\param[in] token The incoming token.
*/
inline string default_LL_error_message(const Token& top,
                                       const Token& token,
                                       [[maybe_unused]] const Symbol& lastDerivedNonterminal,
                                       [[maybe_unused]] bool empty,
                                       [[maybe_unused]] const TerminalSet& first,
                                       [[maybe_unused]] const TerminalSet& follow) {
  using Type = Symbol::Type;

  string errorString{};

  switch (top.type()) {
    case Type::EOI:
      errorString += "Unexpected token '" + token.to_string() + "' at the end of input.";
      break;
    case Type::TERMINAL:
      errorString +=
        "Unexpected token '" + token.to_string() + "'; expected '" + top.to_string() + "'.";
      break;
    case Type::NONTERMINAL:
      errorString += "Unexpected token '" + token.to_string() + "' when deriving '" +
                     top.to_string() + "'; expected one of:\n";
      for (auto&& expected : first.symbols()) {
        errorString += "'" + expected.to_string() + "', ";
      }
      if (empty) {
        for (auto&& expected : follow.symbols()) {
          errorString += "'" + expected.to_string() + "', ";
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
  using error_message_function = std::function<string(const Token& top,
                                                      const Token& token,
                                                      const Symbol& lastDerived,
                                                      bool empty,
                                                      const TerminalSet& first,
                                                      const TerminalSet& follow)>;

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
  LLTranslationControlGeneral(LexicalAnalyzer& la,
                              TranslationGrammar& tg,
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
  empty_t _empty;
  /**
  \brief First set for each nonterminal.
  */
  first_t _first;
  /**
  \brief Follow set for each nonterminal.
  */
  follow_t _follow;
  /**
  \brief Predict set for each nonterminal.
  */
  predict_t _predict;

  /**
  \brief The error message adding function
  */
  error_message_function error_function;

  /**
  \brief Creates all predictive sets.
  */
  void create_predictive_sets() {
    _empty = create_empty(*translationGrammar_);
    _first = create_first(*translationGrammar_, _empty);
    _follow = create_follow(*translationGrammar_, _empty, _first);
    _predict = create_predict(*translationGrammar_, _empty, _first, _follow);
  }

  /**
  \brief Creates iterator attribute actions for incoming terminals.

  \param[in] obegin Iterator to the first Symbol of the output of the applied
  Rule.
  \param[in] targets Indices of the target actions for all input terminals.
  \param[out] attributeActions Targets to append incoming terminal's attributes.

  The added iterators point to input terminal attribute targets.
  */
  void create_attibute_actions(tstack<Token>::iterator obegin,
                               const vector<vector_set<size_t>>& targets,
                               tstack<vector<tstack<Token>::iterator>>& attributeActions) {
    for (auto& target : reverse(targets)) {
      vector<tstack<Token>::iterator> iterators;
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

  void set_error() { _errorFlag = true; }

  void add_error(const Token& top, const Token& token, const Symbol& lastDerivedNonterminal) {
    set_error();
    string message;
    if (top.nonterminal()) {
      size_t i = top.id();
      message =
        error_function(top, token, lastDerivedNonterminal, _empty[i], _first[i], _follow[i]);
    } else {
      message =
        error_function(top, token, lastDerivedNonterminal, false, TerminalSet{0}, TerminalSet{0});
    }
    err() << token.location().to_string() << ": " << message << "\n";
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
  \brief Runs the translation. Output symbols are stored in _output.
  */
  void run() override {
    using Type = Symbol::Type;

    if (!lexicalAnalyzer_)
      throw TranslationException("No lexical analyzer was attached.");
    else if (!translationGrammar_)
      throw TranslationException("No translation grammar was attached.");

    _input.clear();
    _output.clear();
    tstack<vector<tstack<Token>::iterator>> attributeActions;

    // last derived nonterminal
    Symbol lastDerivedNonterminal = translationGrammar_->starting_symbol();

    _input.push(Symbol::eof());
    _output.push(Symbol::eof());
    // add actual starting symbol
    _input.push(translationGrammar_->starting_rule().input()[0]);
    _output.push(translationGrammar_->starting_rule().output()[0]);

    Token token = next_token();

    // iterator to the first symbol of the last inserted string
    // used to speed up _output linear search
    auto obegin = _output.begin();

    while (true) {
      const Token& top = _input.top();
      size_t ruleIndex;
      switch (top.type()) {
        case Symbol::Type::EOI:
          if (token == Symbol::eof()) {
            return;
          } else {
            add_error(top, token, lastDerivedNonterminal);
            return;
          }
          break;
        case Type::TERMINAL:
          if (top == token) {
            for (auto it : attributeActions.pop()) {
              it->set_attribute(token);
            }
            _input.pop();
            token = next_token();
          } else {
            add_error(top, token, lastDerivedNonterminal);
            if (!error_recovery(lastDerivedNonterminal, token, attributeActions))
              return;
          }
          break;
        case Type::NONTERMINAL:
          ruleIndex = _llTable.rule_index(top.symbol(), token.symbol());
          if (ruleIndex < translationGrammar_->rules().size()) {
            // we derive this terminal
            lastDerivedNonterminal = top.symbol();

            auto& rule = translationGrammar_->rules()[ruleIndex];
            obegin = _output.replace(top, rule.output(), obegin);
            _input.replace(_input.begin(), rule.input());
            create_attibute_actions(obegin, rule.actions(), attributeActions);
          } else {
            add_error(top, token, lastDerivedNonterminal);
            if (!error_recovery(lastDerivedNonterminal, token, attributeActions))
              return;
          }
          break;
      }
    }
  }

 protected:
  /**
  \brief LL table used to control the translation.
  */
  LLTableType _llTable;

  /**
  Creates all predictive sets and creates a new LL table.
  */
  void create_ll_table() {
    create_predictive_sets();

    _llTable = LLTableType(*translationGrammar_, _predict);
  }

  /**
  \brief Hartmann error recovery.

  \param[out] token The next valid token.
  \returns True if the error recovery succeeded.
  */
  virtual bool error_recovery(const Symbol& lastDerivedNonterminal,
                              Token& token,
                              tstack<vector<tstack<Token>::iterator>>& attributeActions) {
    using Type = Symbol::Type;

    size_t ruleIndex = 0;
    size_t ntIndex = lastDerivedNonterminal.id();
    if (ntIndex >= translationGrammar_->nonterminals())
      return false;
    auto& ntFollow = _follow[ntIndex];
    // get a token from follow(lastNonterminal_)
    while (!ntFollow[token.symbol()] && token != Symbol::eof()) {
      token = next_token();
    }
    // pop stack until a rule is applicable or the same token is on top
    while (true) {
      Token& top = _input.top();
      switch (top.type()) {
        case Type::EOI:
          return false;
        case Type::TERMINAL:
          if (top == token) {
            return true;
          }
          attributeActions.pop();
          break;
        case Type::NONTERMINAL:
          ruleIndex = _llTable.rule_index(top.symbol(), token.symbol());
          if (ruleIndex < translationGrammar_->rules().size()) {
            return true;
          }
          break;
        default:
          break;
      }
      _input.pop();
    }

    // this should never happen
    throw std::runtime_error("Syntax error recovery failed.");
  }
};

using LLTranslationControl = LLTranslationControlTemplate<LLTable>;
using PriorityLLTranslationControl = LLTranslationControlTemplate<PriorityLLTable>;

class GeneralLLTranslationControl : public LLTranslationControlGeneral {
 public:
  using LLTableType = GeneralLLTable;

 protected:
  vector<Token> _tokens;

  size_t tokenPosition_ = 0;

  struct ParseState {
    tstack<Token> input;
    tstack<Token> output;
    size_t tokenPosition;
    vector_set<size_t> rules;
    tstack<vector<tstack<Token>::iterator>> attributeActions;
  };

  tstack<ParseState> _parseStates;

  Token next_token() override {
    while (tokenPosition_ >= _tokens.size()) {
      _tokens.push_back(TranslationControl::next_token());
    }
    return _tokens[tokenPosition_++];
  }

  bool roll_back(tstack<vector<tstack<Token>::iterator>>& attributeActions,
                 tstack<Token>::iterator& obegin) {
    if (_parseStates.empty())
      return false;
    auto&& previousState = _parseStates.pop();
    _input = previousState.input;
    _output = previousState.output;
    tokenPosition_ = previousState.tokenPosition;
    attributeActions = previousState.attributeActions;

    auto& rule = translationGrammar_->rules()[*(previousState.rules.begin())];
    // a rule is now applicable
    if (previousState.rules.size() > 1) {
      previousState.rules.erase(previousState.rules.begin());
      _parseStates.push(previousState);
    }
    obegin = _output.replace(_input.top(), rule.output());
    _input.replace(_input.begin(), rule.input());
    create_attibute_actions(obegin, rule.actions(), attributeActions);
    return true;
  }

  void run() override {
    using Type = Symbol::Type;

    if (!lexicalAnalyzer_)
      throw TranslationException("No lexical analyzer was attached.");
    else if (!translationGrammar_)
      throw TranslationException("No translation grammar was attached.");

    _input.clear();
    _output.clear();
    tstack<vector<tstack<Token>::iterator>> attributeActions;

    // last derived nonterminal
    Symbol lastDerivedNonterminal = translationGrammar_->starting_symbol();

    _input.push(Symbol::eof());
    _output.push(Symbol::eof());
    _input.push(translationGrammar_->starting_symbol());
    _output.push(translationGrammar_->starting_symbol());

    Token token = next_token();

    // iterator to the first symbol of the last inserted string
    // used to speed up _output linear search
    auto obegin = _output.begin();

    while (true) {
      Token& top = _input.top();
      vector_set<size_t> applicableRules;
      switch (top.type()) {
        case Symbol::Type::EOI:
          if (token == Symbol::eof()) {
            // set attribute from incoming EOF token
            _output.bottom().set_attribute(token);
            return;
          } else {
            add_error(top, token, lastDerivedNonterminal);
            return;
          }
          break;
        case Type::TERMINAL:
          if (top == token) {
            for (auto it : attributeActions.pop()) {
              it->set_attribute(token);
            }
            _input.pop();
            token = next_token();
          } else {
            if (!roll_back(attributeActions, obegin)) {
              add_error(top, token, lastDerivedNonterminal);
              if (!error_recovery(lastDerivedNonterminal, token, attributeActions))
                return;
            }
          }
          break;
        case Type::NONTERMINAL:
          applicableRules = _llTable.rule_index(top.symbol(), token.symbol());
          if (!applicableRules.empty()) {
            // we derive this terminal
            lastDerivedNonterminal = top.symbol();
            auto& rule = translationGrammar_->rules()[*(applicableRules.begin())];

            if (applicableRules.size() > 1) {
              // store state
              applicableRules.erase(applicableRules.begin());
              _parseStates.push(
                ParseState{_input, _output, tokenPosition_, applicableRules, attributeActions});
            }

            obegin = _output.replace(top, rule.output(), obegin);
            _input.replace(_input.begin(), rule.input());
            create_attibute_actions(obegin, rule.actions(), attributeActions);
          } else {
            if (!roll_back(attributeActions, obegin)) {
              add_error(top, token, lastDerivedNonterminal);
              if (!error_recovery(lastDerivedNonterminal, token, attributeActions))
                return;
            }
          }
          break;
        default:
          // unexpected symbol type on input stack
          _input.pop();
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
  LLTableType _llTable;

  /**
  Creates all predictive sets and creates a new LL table.
  */
  void create_ll_table() {
    create_predictive_sets();

    _llTable = LLTableType(*translationGrammar_, _predict);
  }

  virtual bool error_recovery(const Symbol&, Token&, tstack<vector<tstack<Token>::iterator>>&) {
    return false;
  }
};

}  // namespace ctf
#endif
/*** End of file ll_translation_control.hpp ***/