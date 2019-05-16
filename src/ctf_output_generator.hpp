/**
\file ctf_output_generator.hpp
\brief Defines class OutputGenerator, which transforms output tokens to output
and outputs it into a stream.
\author Radek Vit
*/
#ifndef CTF_OUTPUT_GENERATOR_H
#define CTF_OUTPUT_GENERATOR_H

#include <ostream>

#include "ctf_base.hpp"
#include "ctf_output_utilities.hpp"

namespace ctf {

/**
Semantic error exception class.
*/
class SemanticException : public TranslationException {
  using TranslationException::TranslationException;
};

/**
Code generation error exception class.
*/
class CodeGenerationException : public TranslationException {
  using TranslationException::TranslationException;
};

/**
Outputs tokens to output stream. Base class.
*/
class OutputGenerator {
 public:
  OutputGenerator() = default;
  explicit OutputGenerator(std::ostream& os) : _os(&os) {}
  virtual ~OutputGenerator() noexcept = default;
  /**
  \brief Returns true if an output stream has been set.

  \returns True if an output stream has been set. False otherwise.
  */
  bool has_stream() const { return _os != nullptr; }
  /**
  \brief Sets the output stream.

  \param[in] o Output stream.
  */
  void set_output_stream(std::ostream& o) noexcept {
    _os = &o;
    reset();
  }
  /**
  \brief Sets the error stream.

  \param[in] o Error stream.
  */
  void set_error_stream(std::ostream& o) noexcept { _error = &o; }
  /**
  \brief Get the error flag.

  \returns True when an error has been encountered.
  */
  bool error() const noexcept { return _errorFlag; }
  /**
  \brief Clears the error flag.
  */
  void reset() noexcept {
    _errorFlag = false;
    reset_private();
  }

  /**
  \brief Outputs a token to the given stream.

  \param[in] tokens Output Tokens.

  The default output implementation.
  */
  virtual void output(const tstack<Token>& tokens) {
    auto& os = this->os();
    for (auto& t : tokens) {
      os << t.symbol().to_string();
      if (!t.attribute().empty()) {
        os << ".";
        auto& type = t.attribute().type();
        if (type == typeid(string))
          os << t.attribute().get<string>();
        else if (type == typeid(char))
          os << t.attribute().get<char>();
        else if (type == typeid(double))
          os << t.attribute().get<double>();
        else if (type == typeid(std::size_t))
          os << t.attribute().get<std::size_t>();
      }
      os << "\n";
    }
  }

 protected:
  /**
  \brief Get the output stream.

  \returns A reference to the output stream if set.
  */
  std::ostream& os() const {
    if (!_os) {
      throw std::runtime_error("ctf::OutputGenerator::os() output stream not set.");
    }
    return *_os;
  }
  /**
  \brief Set the error flag.
  */
  void set_error() noexcept { _errorFlag = true; }

  /**
  \brief Clears the inner state and errors.
  */
  virtual void reset_private() {}

  /**
  \brief Get the error stream.
  */
  std::ostream& err() {
    if (!_error) {
      throw std::runtime_error("ctf::OutputGenerator::err() error stream not set.");
    }
    return *_error;
  }

  void warning(const string& message) {
    err() << output::color::yellow << "warning" << output::reset << ":\n" << message << "\n";
  }
  /**
  \brief Outputs an error message with the location automatically printed before
  it.
  */
  void warning(const tstack<Token>::const_iterator it, const string& message) {
    err() << it->location().to_string() << ": " << output::color::yellow << "warning"
          << output::reset << ":\n"
          << message << "\n";
  }

  void error(const string& message) {
    err() << output::color::red << "ERROR" << output::reset << ":\n" << message << "\n";
    set_error();
  }
  /**
  \brief Outputs an error message with the location automatically printed before
  it.
  */
  void error(const tstack<Token>::const_iterator it, const string& message) {
    err() << it->location().to_string() << ": " << output::color::red << "ERROR" << output::reset
          << ":\n"
          << message << "\n";
    set_error();
  }

  void fatal_error(const string& message) {
    error(message);
    throw SemanticException("Semantic error encountered.");
  }

  void fatal_error(tstack<Token>::const_iterator it, const string& message) {
    error(it, message);
    throw SemanticException("Semantic error encountered.");
  }

 private:
  /**
  \brief
  */
  bool _errorFlag = false;

  /**
  \brief Pointer to the output stream tokens will be output to.
  */
  std::ostream* _os;

  /**
  \brief The error stream.
  */
  std::ostream* _error;
};
}  // namespace ctf

#endif

/*** End of file ctf_output_generator.hpp ***/
