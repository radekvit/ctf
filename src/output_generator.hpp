/**
\file output_generator.hpp
\brief Defines class OutputGenerator, which transforms output tokens to output
and outputs it into a stream.
*/
#ifndef CTF_OUTPUT_GENERATOR_H
#define CTF_OUTPUT_GENERATOR_H

#include <ostream>

#include "base.hpp"

namespace ctf {
/**
Outputs tokens to output stream. Base class.
*/
class OutputGenerator {
  /**
  \brief
  */
  bool errorFlag_ = false;

  /**
  \brief Pointer to the output stream tokens will be output to.
  */
  std::ostream *os_;

  /**
  \brief Clears the inner state and errors.
  */
  virtual void reset_private() noexcept {}

 protected:
  /**
  \brief Get the output stream.

  \returns A reference to the output stream if set.
  */
  std::ostream &os() const {
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

 public:
  OutputGenerator() = default;
  OutputGenerator(std::ostream &os) : os_(&os) {}
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
  void set_stream(std::ostream &o) noexcept {
    os_ = &o;
    reset();
  }
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
  \brief Get the error message.

  \returns The error message string.
  */
  virtual string error_message() { return ""; }
  /**
  \brief Outputs a token to the given stream.

  \param[in] tokens Symbols to be output.

  The default output implementation.
  */
  virtual void output(const tstack<Symbol> &terminals) {
    auto &os = this->os();
    for (auto &t : terminals) {
      if (t == Symbol::eof())
        return;
      os << t.name();
      if (!t.attribute().empty()) {
        os << ".";
        auto &type = t.attribute().type();
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
};
}  // namespace ctf

#endif
/*** End of file output_generator.hpp ***/