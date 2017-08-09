/**
\file lexical_analyzer.hpp
\brief Defines class LexicalAnalyzer used to supply tokens from input stream.
\author Radek Vít
*/
#ifndef CTF_LEXICAL_ANALYZER_H
#define CTF_LEXICAL_ANALYZER_H

#include "base.hpp"
#include "input_reader.hpp"

#include <cctype>
#include <functional>
#include <istream>

namespace ctf {
/**
\brief Extracts tokens from input stream. Tokens can be Symbols of any type,
type is to be ignored unless it is EOF. Abstract base class.

Lexical errors are created by creating a token with an unused name. Then,
attribute should be used as an error message.
*/
class LexicalAnalyzer {
 private:
  /**
  \brief Input manager. The managing object is responsible for setting up the
  reader object.
  */
  InputReader *reader_;

 protected:
  /**
  \brief Gets next Token from stream. Sets error flag on error.
  \returns A token from the input stream.

  Default implementation; reading a token name until a whitespace or EOF is
  read.
  */
  virtual Symbol read_token() {
    string name;
    // first character
    int c = get();
    while (std::isspace(c)) {
      c = get();
    }
    if (c == std::char_traits<char>::eof()) {
      return Symbol::eof();
    }

    while (!isspace(c) && c != std::char_traits<char>::eof()) {
      name += c;
      c = get();
    }
    reader_->unget();

    return token(name);
  }

  /**
  \brief Error flag. This flag should be set by subclasses on invalid input.
  */
  bool errorFlag_ = false;

  /**
  \brief Current token location.
  */
  Location location_ = Location::invalid();

  /**
  \brief Sets the current token location if not yet specified and reads a
  character.

  \returns The int value of the read character.
  */
  int get() {
    if (location_ == Location::invalid()) {
      return reader_->get(location_);
    }
    return reader_->get();
  }

  /**
  \brief Rolls back input.

  \param[in] num How many positions to roll back.

  \returns The integer value of the character num positions back.
  */
  int unget(size_t num = 1) { return reader_->unget(num); }

  /**
  \brief Constructs a terminal symbol and inserts the current symbol location
  automatically.

  \param[in] name The name of the created Terminal
  \param[in] attr The attribute of the created Terminal

  \returns A terminal Symbol with the current stored location_.
  */
  virtual Symbol token(const string &name, const Attribute &attr = Attribute{}) {
    return Terminal(name, attr, location_);
  }

 public:
  /**
  \brief The implicit constructor. The lexical analyzer is in inoperable state,
  an input reader must be set before it is.
  */
  LexicalAnalyzer() : reader_(nullptr) {}
  LexicalAnalyzer(InputReader &reader) : reader_(&reader) {}
  virtual ~LexicalAnalyzer() = default;

  /**
  \brief Returns true when a reader has been set and the reader has a stream
  set.
  \returns True when the lexical analyzer is ready to receive input. False
  otherwise.
  */
  bool has_input() const noexcept {
    return reader_ != nullptr && reader_->stream() != nullptr;
  }
  /**
  \brief Sets the reader.

  \param[in] reader The reader to be assigned.
  */
  void set_reader(InputReader &reader) noexcept { reader_ = &reader; }
  /**
  \brief Removes the assigned reader.
  */
  void remove_reader() noexcept { reader_ = nullptr; }
  /**
  \brief Sets the input stream to a given stream.

  \param[in] s Stream to be set.
  */
  virtual void reset() {
    clear_error();
    location_ = Location::invalid();
  }
  /**
  \brief Get the error flag.

  \returns True when an error has been encountered.
  */
  virtual bool error() noexcept { return errorFlag_; }
  /**
  \brief Clears the error flag.
  */
  virtual void clear_error() noexcept { errorFlag_ = false; }
  /**
  \brief Get the set error message.

  \returns String with an error message.
  */
  virtual string error_message() { return "Something went wrong.\n"; }
  /**
  \brief Gets next Token from stream and resets symbol location.

  \returns A token from the input stream.
  */
  Symbol get_token() {
    location_ = Location::invalid();
    return read_token();
  }
};
}  // namespace ctf

#endif
/*** End of file lexical_analyzer.hpp ***/