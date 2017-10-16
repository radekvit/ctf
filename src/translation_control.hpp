/**
\file translation_control.hpp
\brief Defines class TranslationControl and its methods.
\author Radek Vít
*/
#ifndef CTF_TRANSLATION_CONTROL_H
#define CTF_TRANSLATION_CONTROL_H

#include "lexical_analyzer.hpp"
#include "ll_table.hpp"
#include "translation_grammar.hpp"

namespace ctf {
/**
 * \brief Syntax exception when error recovery is impossible.
 */
class SyntaxException : public TranslationException {
  using TranslationException::TranslationException;
};
/**
\brief Abstract class for syntax driven translation control.
*/
class TranslationControl {
 protected:
  /**
  \brief Alias for TranslationGrammar::Rule
  */
  using Rule = TranslationGrammar::Rule;

  /**
  \brief Lexical analyzer for getting tokens from input.
  */
  LexicalAnalyzer* lexicalAnalyzer_ = nullptr;
  /**
  \brief Translation grammar defining the input and output languages.
  */
  const TranslationGrammar* translationGrammar_ = nullptr;
  /**
  \brief Tstack of input symbols.
  */
  tstack<Symbol> input_;
  /**
  \brief Tstack of output symbols.
  */
  tstack<Symbol> output_;
  /**
  \brief Error flag.
  */
  bool errorFlag_ = false;
  /**
  \brief Returns the next token obtained from lexicalAnalyzer_.
  */
  virtual Symbol next_token() { return lexicalAnalyzer_->get_token(); }

 public:
  virtual ~TranslationControl() = default;

  /**
  \brief Resets translation state.
  */
  virtual void reset() noexcept {
    clear_error();
    input_.clear();
    output_.clear();
  }
  /**
  \brief Get the error flag.

  \returns The value of the error flag.
  */
  virtual bool error() const { return errorFlag_; }
  /**
  \brief Clears the error flag.
  */
  virtual void clear_error() noexcept { errorFlag_ = false; }
  /**
  \brief Get the error message.

  \returns String with appropriate error message. Is only to be called when
  error() is true.
  */
  virtual string error_message() { return "Something went wrong.\n"; }
  /**
  \brief Sets lexical analyzer.

  \param[in] la LexicalAnalyzer to be set.
  */
  virtual void set_lexical_analyzer(LexicalAnalyzer& la) {
    lexicalAnalyzer_ = &la;
  }
  /**
  \brief Sets translation grammar.

  \param[in] tg Translation grammar to be set.
  */
  virtual void set_grammar(const TranslationGrammar& tg) {
    translationGrammar_ = &tg;
  }
  /**
  \brief Runs translation. Translation output is stored in output_.
  */
  virtual void run() = 0;
  /**
  \brief Returns a constant reference to output symbols.

  \returns All output symbols.
  */
  virtual const tstack<Symbol>& output() const noexcept { return output_; }
};
}  // namespace ctf
#endif
/*** End of file translation_control.hpp ***/