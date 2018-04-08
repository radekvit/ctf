/**
\file translation_control.hpp
\brief Defines class TranslationControl and its methods.
\author Radek Vít
*/
#ifndef CTF_TRANSLATION_CONTROL_H
#define CTF_TRANSLATION_CONTROL_H

#include "ctf_lexical_analyzer.hpp"
#include "ctf_ll_table.hpp"
#include "ctf_translation_grammar.hpp"

namespace ctf {
/**
\brief Syntax exception when error recovery is impossible.
 */
class SyntaxException : public TranslationException {
  using TranslationException::TranslationException;
};
/**
\brief Abstract class for syntax driven translation control.
*/
class TranslationControl {
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
  \brief Set the error stream.

  \param[in] os The output stream to be set.
  */
  void set_error_stream(std::ostream& os) { error_ = &os; }

  /**
  \brief Runs translation. Translation output is stored in output_.
  */
  virtual void run() = 0;

  /**
  \brief Returns a constant reference to output symbols.

  \returns All output symbols.
  */
  virtual const tstack<Symbol>& output() const noexcept { return output_; }

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

  /**
  \brief Get the error stream.
  */
  std::ostream& err() {
    if (!error_) {
      throw std::runtime_error(
          "ctf::TranslationControl::err() error stream not set.");
    }
    return *error_;
  }

 private:
  /**
  \brief The error stream.
  */
  std::ostream* error_ = nullptr;
};
}  // namespace ctf
#endif
/*** End of file translation_control.hpp ***/