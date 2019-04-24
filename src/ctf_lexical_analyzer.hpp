/**
\file ctf_lexical_analyzer.hpp
\brief Defines class LexicalAnalyzer used to supply tokens from input stream.
\author Radek Vít
*/
#ifndef CTF_LEXICAL_ANALYZER_H
#define CTF_LEXICAL_ANALYZER_H

#include <cctype>
#include <functional>
#include <istream>
#include <ostream>

#include "ctf_base.hpp"
#include "ctf_input_reader.hpp"
#include "ctf_output_utilities.hpp"

namespace ctf {
/**
 * \brief Lexical analysis exception for when error recovery is impossible.
 */
class LexicalException : public TranslationException {
  using TranslationException::TranslationException;
};
/**
\brief Extracts tokens from input stream. Symbols can be Symbols of any type,
type is to be ignored unless it is EOF. Abstract base class.

Lexical errors are created by creating a token with an unused name. Then,
attribute should be used as an error message.
*/
class LexicalAnalyzer {
 public:
  /**
  \brief The implicit constructor. The lexical analyzer is in inoperable state,
  an input reader must be set before it is.
  */
  LexicalAnalyzer() {}
  explicit LexicalAnalyzer(InputReader& reader) : reader_(&reader) {}
  virtual ~LexicalAnalyzer() noexcept = default;

  /**
  \brief Returns true when a reader has been set and the reader has a stream
  set.
  \returns True when the lexical analyzer is ready to receive input. False
  otherwise.
  */
  bool has_input() const noexcept { return reader_ != nullptr && reader_->stream() != nullptr; }
  /**
  \brief Sets the reader.

  \param[in] reader The reader to be assigned.
  */
  void set_reader(InputReader& reader) noexcept { reader_ = &reader; }
  /**
  \brief Removes the assigned reader.
  */
  void remove_reader() noexcept { reader_ = nullptr; }
  /**
  \brief Resets the internal state.
  */
  void reset() {
    clear_error();
    _location = Location::invalid();
    if (reader_) {
      reader_->reset();
    }
    reset_private();
  }
  /**
  \brief Get the error flag.

  \returns True when an error has been encountered.
  */
  bool error() const noexcept { return _errorFlag; }

  /**
  \brief Resets location and gets next Token from the input stream.

  \returns A token from the input stream.
  */
  Token get_token() {
    reset_location();
    return read_token();
  }

  /**
  \brief Set the error stream.

  \param[in] os The output stream to be set.
  */
  void set_error_stream(std::ostream& os) { _error = &os; }

 protected:
  /**
  \brief Gets next Symbol from stream. Sets error flag on error.
  \returns A token from the input stream.

  Default implementation; reading a token name until a whitespace or EOF is
  read. Adds a size_t attribute to each token denoting its number.
  */
  virtual Token read_token() {
    string name;
    // first character
    int c = get();
    while (std::isspace(c)) {
      reset_location();
      c = get();
    }

    if (c == std::char_traits<char>::eof()) {
      return token_eof();
    }

    do {
      name += c;
      c = get();
    } while (!isspace(c) && c != std::char_traits<char>::eof());
    unget();

    return token(std::stoull(name));
  }

  /**
  \brief Sets the current token location if not yet specified and reads a
  character.

  \returns The int value of the read character.
  */
  int get() {
    if (_location == Location::invalid()) {
      return reader_->get(_location);
    }
    return reader_->get();
  }

  /**
  \brief Sets the current token location if not yet specified and reads a
  character that satisfies the supplied predicate.
  \param[in] accept The supplied predicate.

  \returns The int value of the read character.
  */
  int get(std::function<bool(int)> accept) {
    int result;
    do {
      result = reader_->get();
    } while (!accept(result));
    reader_->unget();
    if (_location == Location::invalid()) {
      return reader_->get(_location);
    }
    return reader_->get();
  }

  /**
  \brief Rolls back input.

  \param[in] num How many positions to roll back.

  \returns The integer value of the character num positions back.
  */
  void unget(size_t num = 1) { reader_->unget(num); }

  /**
  \brief Resets the current token's location.
  */
  void reset_location() { _location = Location::invalid(); }
  /**
  \brief Get the current stored location.

  \returns A const reference to the current location.
  */
  const Location& location() const noexcept { return _location; }

  /**
  \brief Constructs a terminal symbol and inserts the current symbol location
  automatically.

  \param[in] name The name of the created Terminal
  \param[in] attr The attribute of the created Terminal

  \returns A terminal Symbol with the current stored _location.
  */
  Token token(Symbol s, const Attribute& attr = Attribute{}) { return Token(s, attr, _location); }

  /**
  \brief Constructs a terminal symbol and inserts the current symbol location
  automatically.

  \param[in] name The name of the created Terminal
  \param[in] attr The attribute of the created Terminal

  \returns A terminal Symbol with the current stored _location.
  */
  Token token(size_t i, const Attribute& attr = Attribute{}) {
    return Token(Terminal(i), attr, _location);
  }

  /**
  \brief Constructs an EOI Symbol and inserts the current symbol location
  automatically.

  \returns An EOI Symbol with the current stored _location.
  */
  Token token_eof() { return Token(Symbol::eof(), Attribute{}, _location); }

  /**
  \brief Returns a reference to the error flag.
  */
  void set_error() noexcept { _errorFlag = true; }

  /**
  \brief Outputs a warning message with the location automatically printed before
  it.
  */
  void warning(const string& message) {
    err() << _location.to_string() << ": " << output::color::yellow << "warning" << output::reset
          << ":\n"
          << message << "\n";
  }
  /**
  \brief Outputs an error message with the location automatically printed before
  it and sets the error flag.
  */
  void error(const string& message) {
    err() << _location.to_string() << ": " << output::color::red << "ERROR" << output::reset
          << ":\n"
          << message << "\n";
    set_error();
  }
  /**
  \brief Outputs an error message with the location automatically printed before
  it, sets the error flag and throws LexicalException.
  */
  void fatal_error(const string& message) {
    error(message);
    throw LexicalException("Lexical error encountered.");
  }

  /**
  \brief Get the error stream.
  */
  std::ostream& err() {
    if (!_error) {
      throw std::runtime_error("ctf::OutputGenerator::err() error stream not set.");
    }
    return *_error;
  }

 private:
  /**
  \brief Input manager. The managing object is responsible for setting up the
  reader object.
  */
  InputReader* reader_ = nullptr;

  /**
  \brief The error stream.
  */
  std::ostream* _error = nullptr;

  /**
  \brief Error flag. This flag should be set by subclasses on invalid input.
  */
  bool _errorFlag = false;

  /**
  \brief Current token location.
  */
  Location _location = Location::invalid();

  /**
  \brief Clears the error flag and resets all error messages.
  */
  void clear_error() noexcept { _errorFlag = false; }

  virtual void reset_private() {}
};
}  // namespace ctf

#endif

/*** End of file ctf_lexical_analyzer.hpp ***/
