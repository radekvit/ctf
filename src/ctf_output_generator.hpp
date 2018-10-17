/**
\file output_generator.hpp
\brief Defines class OutputGenerator, which transforms output tokens to output
and outputs it into a stream.
*/
#ifndef CTF_OUTPUT_GENERATOR_H
#define CTF_OUTPUT_GENERATOR_H

#include <ostream>

#include "ctf_base.hpp"

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
  explicit OutputGenerator(std::ostream& os) : os_(&os) {}
  virtual ~OutputGenerator() noexcept = default;
  /**
  \brief Returns true if an output stream has been set.

  \returns True if an output stream has been set. False otherwise.
  */
  bool has_stream() const { return os_ != nullptr; }
  /**
  \brief Sets the output stream.

  \param[in] o Output stream.
  */
  void set_output_stream(std::ostream& o) noexcept {
    os_ = &o;
    reset();
  }
  /**
  \brief Sets the error stream.

  \param[in] o Error stream.
  */
  void set_error_stream(std::ostream& o) noexcept { error_ = &o; }
  /**
  \brief Get the error flag.

  \returns True when an error has been encountered.
  */
  bool error() const noexcept { return errorFlag_; }
  /**
  \brief Clears the error flag.
  */
  void reset() noexcept {
    errorFlag_ = false;
    reset_private();
  }

  /**
  \brief Outputs a token to the given stream.

  \param[in] tokens Symbols to be output.

  The default output implementation.
  */
  virtual void output(const tstack<Symbol>& terminals) {
    auto& os = this->os();
    for (auto& t : terminals) {
      if (t == Symbol::eof())
        return;
      os << t.name();
      if (!t.attribute().empty()) {
        os << ".";
        auto& type = t.attribute().type();
        if (type == typeid(string))
          os << t.attribute().get<string>();
        else if (type == typeid(char))
          os << t.attribute().get<char>();
        else if (type == typeid(double))
          os << t.attribute().get<double>();
        else if (type == typeid(size_t))
          os << t.attribute().get<size_t>();
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
    if (!os_) {
      throw std::runtime_error(
          "ctf::OutputGenerator::os() output stream not set.");
    }
    return *os_;
  }
  /**
  \brief Set the error flag.
  */
  void set_error() noexcept { errorFlag_ = true; }

  /**
  \brief Clears the inner state and errors.
  */
  virtual void reset_private() noexcept {}

  /**
  \brief Get the error stream.
  */
  std::ostream& err() {
    if (!error_) {
      throw std::runtime_error(
          "ctf::OutputGenerator::err() error stream not set.");
    }
    return *error_;
  }

  void error_message(const string& message) { err() << message << "\n"; }
  /**
  \brief Outputs an error message with the location automatically printed before
  it.
  */
  void error_message(const tstack<Symbol>::const_iterator it,
                     const string& message) {
    err() << it->location().to_string() << ": " << message << "\n";
  }

  void fatal_error(const string& message) {
    error_message(message);
    set_error();
    throw SemanticException("Semantic error encountered.");
  }

  void fatal_error(tstack<Symbol>::const_iterator it, const string& message) {
    error_message(it, message);
    set_error();
    throw SemanticException("Semantic error encountered.");
  }

 private:
  /**
  \brief
  */
  bool errorFlag_ = false;

  /**
  \brief Pointer to the output stream tokens will be output to.
  */
  std::ostream* os_;

  /**
  \brief The error stream.
  */
  std::ostream* error_;
};
}  // namespace ctf

#endif
/*** End of file output_generator.hpp ***/