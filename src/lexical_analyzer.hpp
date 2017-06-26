/**
\file lexical_analyzer.hpp
\brief Defines class LexicalAnalyzer used to supply tokens from input stream.
\author Radek Vít
*/
#ifndef CTF_LEXICAL_ANALYZER
#define CTF_LEXICAL_ANALYZER

#include "base.hpp"

#include <cctype>
#include <functional>
#include <istream>

namespace ctf {
/**
\brief Alias for Symbol. Token and Symbol are interchangable.
*/
using Token = Symbol;

/**
\brief Extracts tokens from input stream. Tokens can be Symbols of any type,
type is to be ignored. Abstract base class.

Lexical errors are created by creating a token with an unused name. Then,
attribute should be used as an error message.
*/
class LexicalAnalyzer {
 protected:
  /**
  \brief Pointer to the input stream that tokenFunction takes input from.
  LexicalAnalyzer does not own the
  istream.
  */
  std::istream *is_;

  string streamName_;

  bool errorFlag_ = false;

 public:
  LexicalAnalyzer() = default;
  LexicalAnalyzer(std::istream &is) : is_(&is) {}
  virtual ~LexicalAnalyzer() = default;

  /**
  \brief Returns true when a stream has been set.
  \returns True when a stream has been set. False otherwise.
  */
  virtual bool has_stream() const noexcept { return is_ != nullptr; }
  /**
  \brief Sets the input stream to a given stream.
  \param[in] s Stream to be set.
  */
  virtual void set_stream(std::istream &s, const string &streamName = "") noexcept {
    is_ = &s;
    clear_error();
    streamName_ = streamName;
  }
  /**
  \returns True when an error has been encountered.
  */
  virtual bool error() noexcept { return errorFlag_; }
  /**
  \brief Clears the error flag.
  */
  virtual void clear_error() noexcept { errorFlag_ = false; }
  /**
  \returns String with appropriate error message. Is only to be called when
  error() is true.
  */
  virtual string error_message() { return "Something went wrong.\n"; }
  /**
  \brief Gets next Token from stream. Sets error flag on error.
  \returns A token from the input stream.

  Default implementation; characters before a dot set the token name, characters
  after the first dot are the attribute.
  */
  virtual Token get_token() {
    static bool eofFlag = false;
    string name;
    string attribute;
    if (eofFlag)
      return Symbol::eof();
    int c = is_->get();
    if (c == std::char_traits<char>::eof())
      return Symbol::eof();
    // ignoring prefix dot characters, setting error if encountered
    while (c == '.') {
      errorFlag_ = true;
      c = is_->get();
    }
    while (c != '.' && c != '\n' && c != std::char_traits<char>::eof()) {
      name += string{static_cast<char>(c)};

      c = is_->get();
    }
    if (c == '.')
      c = is_->get();
    while (c != '\n' && c != std::char_traits<char>::eof()) {
      attribute += string{static_cast<char>(c)};
      c = is_->get();
    }
    if (c == std::char_traits<char>::eof())
      eofFlag = true;
    return Terminal(name, attribute);
  }
};
}  // namespace ctf

#endif
/*** End of file lexical_analyzer.hpp ***/