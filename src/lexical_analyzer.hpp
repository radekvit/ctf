/**
\file lexical_analyzer.hpp
\brief Defines class LexicalAnalyzer used to supply tokens from input stream.
\author Radek Vít
*/
#ifndef CTF_LEXICAL_ANALYZER
#define CTF_LEXICAL_ANALYZER

#include "base.hpp"
#include "input_reader.hpp"

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
  \brief Error flag.
  */
  bool errorFlag_ = false;
  /**
  \brief Input manager. The managing object is responsible for setting up the reader object.
  */
  InputReader *reader_;
  /**
  \brief Current token location.
  */
  Location location_ = Location::invalid();

  /**
  \brief Helper method. Sets the current token location if not yet specified.
  */
  int get() {
    if (location_ == Location::invalid()) {
      return reader_->get(location_);
    }
    return reader_->get();
  }

  int unget(size_t num = 1) {
    return reader_->unget(num);
  }

  virtual Token token(const string &name = "", const string &attr = "") {
    auto location = location_;
    location_ = Location::invalid();
    return Terminal(name, attr, location);
  }

 public:
  LexicalAnalyzer() : reader_(nullptr) {}
  LexicalAnalyzer(InputReader &reader) : reader_(&reader) {}
  virtual ~LexicalAnalyzer() = default;

  /**
  \brief Returns true when a reader has been set and the reader has a stream set.
  \returns True when the lexical analyzer is ready to receive input. False otherwise.
  */
  bool has_input() const noexcept {
    return reader_ != nullptr && reader_->stream() != nullptr;
  }
  /**
  \brief Sets the reader.
  */
  void set_reader(InputReader &reader) noexcept {
    reader_ = &reader;
  }
  /**
  \brief Sets the input stream to a given stream.
  \param[in] s Stream to be set.
  */
  virtual void reset() {
    clear_error();
    location_ = Location::invalid();
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

  Default implementation; reading a token name until a whitespace or EOF is
  read.
  */
  virtual Token get_token() {
    string name;
    // first character
    int c = get();
    while (std::isspace(c)) {
      c = get();
    }
    if (c == std::char_traits<char>::eof()) {
      return Token::eof();
    }

    while (!isspace(c) && c != std::char_traits<char>::eof()) {
      name += c;
      c = get();
    }
    reader_->unget();

    return token(name);
  }
};
}  // namespace ctf

#endif
/*** End of file lexical_analyzer.hpp ***/