/**
\file lexical_analyzer.h
\brief Defines class LexicalAnalyzer used to supply tokens from input stream.
\author Radek Vít
*/
#ifndef CTF_LEXICAL_ANALYZER
#define CTF_LEXICAL_ANALYZER

#include <cctype>
#include <functional>
#include <istream>

#include "base.h"

namespace ctf {

/**
\brief Exception class to be thrown by user defined lexical analyzers on lexical
errors.
*/
class LexicalError : public TranslationException {
 public:
  using TranslationException::TranslationException;
};

/**
\brief Alias for Symbol. Token and Symbol are interchangable.
*/
using Token = Symbol;

/**
\brief Extracts tokens from input stream. Tokens can be Symbols of any type,
type is to be ignored.

Lexical errors are created by creating a token with an unused name. Then,
attribute should be used as an error message.
*/
class LexicalAnalyzer {
 public:
  /**
  \brief Alias to std::function.
  */
  using token_function = std::function<Token(std::istream &)>;

 private:
  /**
  \brief Pointer to the input stream that tokenFunction takes input from. May
  be changed between tokenFunction calls.
  */
  std::istream *is;
  /**
  \brief A function, lambda or callable object used to extract tokens from an
  input stream. At EOF it is expected to return Token::EOI().
  */
  token_function tokenFunction;

 public:
  /**
  \brief Constructs LexicalAnalyzer without a given input stream. If
  specified, f determines the tokenFunction.
  \param[in] f A callable for extracting Tokens from a stream.
  */
  LexicalAnalyzer(token_function f = LexicalAnalyzer::default_input)
      : is(nullptr), tokenFunction(f) {}
  /**
  \brief Constructs LexicalAnalyzer with a given istream. If specified, f
  determines the tokenFunction.
  \param[in] _i A stream from which tokens are extracted.
  \param[in] f A callable for extracting Tokens from a stream.
  */
  LexicalAnalyzer(std::istream &_i,
                  token_function f = LexicalAnalyzer::default_input)
      : is(&_i), tokenFunction(f) {}

  /**
  \brief Returns true when a stream has been set.
  \returns True when a stream has been set. False otherwise.
  */
  bool stream_set() const { return is; }
  /**
  \brief Sets the input stream to a given stream.
  \param[in] s Stream to be set.
  */
  void set_input(std::istream &s) { is = &s; }
  /**
  \brief Uses tokenFunction to get a Token from input stream. If
  LexicalAnalyzer::stream_set() is false, this results in undefined behavior.
  \returns A token recieved from tokenFunction.
  */
  Token get_token() { return tokenFunction(*is); };

  /**
  \brief Default token extractor. All characters that are not spaces become
  tokens.
  */
  static Token default_input(std::istream &is) {
    char c;
  read:
    if (is.get(c)) {
      if (std::isspace(static_cast<unsigned char>(c)))
        goto read;
      return Token{{c}};
    } else
      return Token::EOI();
  }
};
}  // namespace ctf

#endif
/*** End of file lexical_analyzer.h ***/