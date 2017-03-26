/**
\file translation_control.h
\brief Defines class TranslationControl and its methods.
\author Radek Vít
*/
#ifndef CTF_TRANSLATION_CONTROL
#define CTF_TRANSLATION_CONTROL

#include <base.h>
#include <lexical_analyzer.h>
#include <ll_table.h>
#include <translation_grammar.h>

namespace ctf {

/**
\brief Exception class for syntax errors.
*/
class SyntaxError : public TranslationException {
  using TranslationException::TranslationException;
};

/**
\brief Abstract class for syntax driven translation control.
*/
class TranslationControl {
 public:
  /**
  \brief Error message getter function.
  */
  using error_function =
      std::function<string(const Symbol &nonterminal, const Symbol &terminal)>;

 protected:
  /**
  \brief Alias for TranslationGrammar::Rule
  */
  using Rule = TranslationGrammar::Rule;

  /**
  \brief Lexical analyzer for getting tokens from input.
  */
  LexicalAnalyzer *lexicalAnalyzer_ = nullptr;
  /**
  \brief Translation grammar defining the input and output languages.
  */
  const TranslationGrammar *translationGrammar_ = nullptr;
  /**
  \brief Tstack of input symbols.
  */
  tstack<Symbol> input_;
      /**
      \brief Tstack of output symbols.
      */
      tstack<Symbol>
          output_;
  /**
  \brief Syntax error message function.

  Defaults to writing nonterminal's and token's names.
  */
  error_function syntaxErrorMessage_ = [](auto nt, auto t) {
    return "Nonterminal " + nt.name() + ", token " + t.name() +
           (t.attribute() == "" ? "" : "." + t.attribute());
  };

 public:
  virtual ~TranslationControl() = default;

  /**
  \brief Sets lexical analyzer.
  \param[in] la LexicalAnalyzer to be set.
  */
  void set_lexical_analyzer(LexicalAnalyzer &la) { lexicalAnalyzer_ = &la; }
  /**
  \brief Sets translation grammar.
  \param[in] tg Translation grammar to be set.
  */
  virtual void set_grammar(const TranslationGrammar &tg) {
    translationGrammar_ = &tg;
  }
  /**
  \brief Sets syntax error message function.
  \param[in] f Callable to return syntax error strings.
  */
  void set_syntax_error_message(error_function f) { syntaxErrorMessage_ = f; }
  /**
  \brief Runs translation.
  */
  virtual void run() = 0;
  /**
  \brief Gets token from lexicalAnalyzer_ and stores it in a given vector.
  Returns this terminal.
  \param[out] string Vector of all read symbols.
  \returns New symbol obtained from lexical analyzer.
  */
  Token next_token(vector<Symbol> &string) {
    string.push_back(lexicalAnalyzer_->get_token());
    return string.back();
  }
  /**
  \brief Returns a constant reference to output symbols.
  \returns All output symbols.
  */
  const tstack<Symbol> &output() const { return output_; }
};
}  // namespace ctf
#endif
/*** End of file translation_control.h ***/