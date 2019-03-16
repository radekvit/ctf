/**
\file lr_translation_control.hpp
\brief Defines class LRTranslationControl and defines its methods.
\author Radek Vít
*/
#ifndef CTF_LR_TRANSLATION_CONTROL_H
#define CTF_LR_TRANSLATION_CONTROL_H

#include <functional>
#include <iostream>

#include "ctf_lr_lalr.hpp"
#include "ctf_lr_lr0.hpp"
#include "ctf_lr_table.hpp"
#include "ctf_output_utilities.hpp"
#include "ctf_table_sets.hpp"
#include "ctf_translation_control.hpp"

namespace ctf {

inline string default_lr_error_message(size_t state,
                                       const Token& token,
                                       const TranslationGrammar& tg,
                                       const LRGenericTable& lrTable,
                                       const InputReader&,
                                       symbol_string_fn to_str) {
  string message = "Unexpected symbol ";
  message += to_str(token.symbol());
  message += "\nExpected:";
  for (auto terminal = Symbol::eof(); terminal.id() < tg.terminals();
       terminal = Terminal(terminal.id())) {
    if (lrTable.lr_action(state, terminal).action() != LRAction::ERROR) {
      message += " ";
      message += to_str(terminal);
    }
  }
  return message;
}

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
  LRTranslationControlGeneral(LexicalAnalyzer& la,
                              TranslationGrammar& tg,
                              symbol_string_fn to_str = ctf::to_string) {
    set_grammar(tg, to_str);
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

  void set_error() { _errorFlag = true; }

  void add_error(const Token& token, const string& message) {
    set_error();
    err() << token.location().to_string() << ": " << output::color::red << "ERROR" << output::reset
          << ":\n"
          << message << "\n";
  }

  /**
  \brief Placeholder error recovery.
  */
  virtual bool error_recovery(vector<size_t>&, Token&) { return false; }
};  // namespace ctf

/**
\brief Implements LR bottom up translation control.
*/
template <typename LRTableType>
class LRTranslationControlTemplate : public LRTranslationControlGeneral {
 public:
  using error_function = std::function<string(size_t state,
                                              const Token& token,
                                              const TranslationGrammar& tg,
                                              const LRGenericTable& lrTable,
                                              const InputReader&,
                                              symbol_string_fn to_str)>;
  /**
  \brief Constructs a LRTranslationControlGeneral.
  */
  explicit LRTranslationControlTemplate(error_function errorMessage = default_lr_error_message)
    : _errorMessage(errorMessage) {}
  /**
  \brief Constructs LRTranslationControlGeneral with a LexicalAnalyzer and
  TranslationGrammar.

  \param[in] la A reference to the lexical analyzer to be used to get tokens.
  \param[in] tg The translation grammar for this translation.
  */
  LRTranslationControlTemplate(LexicalAnalyzer& la,
                               TranslationGrammar& tg,
                               symbol_string_fn to_str = ctf::to_string) {
    set_grammar(tg, to_str);
    set_lexical_analyzer(la);
  }

  /**
  \brief Runs the translation. Output symbols are stored in _output.
  */
  void run(const InputReader& reader, symbol_string_fn to_str = ctf::to_string) final {
    if (!_lexicalAnalyzer)
      throw TranslationException("No lexical analyzer was attached.");
    else if (!_translationGrammar)
      throw TranslationException("No translation grammar was attached.");

    _input.clear();
    _output.clear();

    size_t state = 0;
    vector<size_t> pushdown;
    vector<size_t> appliedRules{};

    pushdown.push_back(state);

    Token token = next_token();

    while (true) {
      switch (auto& item = _lrTable.lr_action(state, token.symbol()); item.action()) {
        case LRAction::SHIFT:
          state = item.argument();
          pushdown.push_back(state);
          token = next_token();
          break;
        case LRAction::REDUCE: {
          auto& rule = _translationGrammar->rules()[item.argument()];
          for (size_t i = 0; i < rule.input().size(); ++i) {
            pushdown.pop_back();
          }
          const auto& stackState = pushdown.back();
          state = _lrTable.lr_goto(stackState, rule.nonterminal());
          pushdown.push_back(state);
          appliedRules.push_back(item.argument());
          break;
        }
        case LRAction::SUCCESS:
          appliedRules.push_back(_translationGrammar->rules().size() - 1);
          produce_output(appliedRules);
          return;
        case LRAction::ERROR:
          add_error(token,
                    _errorMessage(state, token, *_translationGrammar, _lrTable, reader, to_str));
          if (!error_recovery(pushdown, token))
            return;
          state = pushdown.back();
      }
    }
  }

  /**
   * Iterates over reversed rules and applies them in a top-down manner.
   */
  void produce_output(const vector<size_t>& appliedRules) {
    tstack<vector<tstack<Token>::iterator>> attributeActions;

    _input.push(_translationGrammar->starting_symbol());
    _output.push(_translationGrammar->starting_symbol());

    auto obegin = _output.begin();
    auto tokenIt = _tokens.crbegin();
    for (auto& ruleIndex : reverse(appliedRules)) {
      auto& rule = _translationGrammar->rules()[ruleIndex];
      _input.replace_last(rule.nonterminal(), rule.input());
      obegin = _output.replace_last(rule.nonterminal(), rule.output(), obegin);
      create_attibute_actions(obegin, rule.actions(), rule.output().size(), attributeActions);

      // apply attribute actions for all current rightmost terminals
      for (auto workingTerminalIt = _input.crbegin();
           workingTerminalIt != _input.crend() &&
           workingTerminalIt->type() != Symbol::Type::NONTERMINAL;
           ++tokenIt) {
        for (auto symbolIt : attributeActions.pop()) {
          symbolIt->set_attribute(*tokenIt);
        }
        _input.pop_bottom();
        workingTerminalIt = _input.crbegin();
      }
    }
    assert(attributeActions.empty());
  }

  /**
  \brief Sets translation grammar.

  \param[in] tg The translation grammar for this translation.
  */
  void set_grammar(const TranslationGrammar& tg,
                   symbol_string_fn to_str = ctf::to_string) override {
    _translationGrammar = &tg;
    create_lr_table(to_str);
  }

  bool error_recovery(vector<size_t>&, Token&) override { return false; }

  void save(std::ostream& os) const override { _lrTable.save(os); }

 protected:
  /**
  \brief LR table used to control the translation.
  */
  LRTableType _lrTable;
  /**
  \brief All read tokens
  */
  vector<Token> _tokens;

  error_function _errorMessage;

  /**
  Creates all predictive sets and creates a new LR table.
  */
  void create_lr_table(symbol_string_fn to_str = ctf::to_string) {
    _lrTable = LRTableType(*_translationGrammar, to_str);
  }

  Token next_token() override {
    _tokens.push_back(TranslationControl::next_token());
    return _tokens.back();
  }
};

class SavedLRTranslationControl : public LRTranslationControlTemplate<LRSavedTable> {
 public:
  SavedLRTranslationControl(std::istream& is, error_function errFn = default_lr_error_message)
    : LRTranslationControlTemplate<LRSavedTable>(errFn) {
    _lrTable = LRSavedTable(is);
  }

 protected:
  void set_grammar(const TranslationGrammar& tg, symbol_string_fn = ctf::to_string) override {
    _translationGrammar = &tg;
  }
};

using LALRTranslationControl = LRTranslationControlTemplate<LALRTable>;
using LR1TranslationControl = LRTranslationControlTemplate<LR1Table>;
using LSCELRTranslationControl = LRTranslationControlTemplate<LSCELRTable>;

using LALRStrictTranslationControl = LRTranslationControlTemplate<LALRStrictTable>;
using LR1StrictTranslationControl = LRTranslationControlTemplate<LR1StrictTable>;

}  // namespace ctf
#endif
/*** End of file ll_translation_control.hpp ***/