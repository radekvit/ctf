/**
\file translation_control.hpp
\brief Defines class TranslationControl and its methods.
\author Radek Vít
*/
#ifndef CTF_TRANSLATION_CONTROL_H
#define CTF_TRANSLATION_CONTROL_H

#include "ctf_lexical_analyzer.hpp"
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
    _input.clear();
    _output.clear();
  }

  /**
  \brief Get the error flag.

  \returns The value of the error flag.
  */
  bool error() const { return _errorFlag; }

  /**
  \brief Clears the error flag.
  */
  virtual void clear_error() noexcept { _errorFlag = false; }

  /**
  \brief Sets lexical analyzer.

  \param[in] la LexicalAnalyzer to be set.
  */
  void set_lexical_analyzer(LexicalAnalyzer& la) { _lexicalAnalyzer = &la; }

  /**
  \brief Sets translation grammar.

  \param[in] tg Translation grammar to be set.
  */
  virtual void set_grammar(const TranslationGrammar& tg, symbol_string_fn = ctf::to_string) {
    _translationGrammar = &tg;
  }

  /**
  \brief Set the error stream.

  \param[in] os The output stream to be set.
  */
  void set_error_stream(std::ostream& os) { _error = &os; }

  /**
  \brief Runs translation. Translation output is stored in _output.
  */
  virtual void run(const InputReader& reader, symbol_string_fn to_str = ctf::to_string) = 0;

  /**
  \brief Returns a constant reference to output symbols.

  \returns All output symbols.
  */
  const tstack<Token>& output() const noexcept { return _output; }

  virtual void save(std::ostream&) const {}

 protected:
  /**
  \brief Alias for TranslationGrammar::Rule
  */
  using Rule = TranslationGrammar::Rule;

  /**
  \brief Lexical analyzer for getting tokens from input.
  */
  LexicalAnalyzer* _lexicalAnalyzer = nullptr;

  /**
  \brief Translation grammar defining the input and output languages.
  */
  const TranslationGrammar* _translationGrammar = nullptr;

  /**
  \brief Tstack of input symbols.
  */
  tstack<Token> _input;

  /**
  \brief Tstack of output symbols.
  */
  tstack<Token> _output;

  /**
  \brief Error flag.
  */
  bool _errorFlag = false;

  /**
  \brief Returns the next token obtained from _lexicalAnalyzer.
  */
  virtual Token next_token() { return _lexicalAnalyzer->get_token(); }

  /**
  \brief Get the error stream.
  */
  std::ostream& err() {
    if (!_error) {
      throw std::runtime_error("ctf::TranslationControl::err() error stream not set.");
    }
    return *_error;
  }

 private:
  /**
  \brief The error stream.
  */
  std::ostream* _error = nullptr;
};
}  // namespace ctf
#endif
/*** End of file translation_control.hpp ***/