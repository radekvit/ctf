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
 protected:
  /**
  \brief Pointer to the output stream tokens will be output to.
  */
  std::ostream *os_;
  /**
  \brief
  */
  bool errorFlag_ = false;

  /**
  \brief Get a reference to the error flag.

  \returns A reference to the error flag.
  */
  void set_error() noexcept { errorFlag_ = true; }

 public:
  OutputGenerator() = default;
  OutputGenerator(std::ostream &os) : os_(&os) {}
  virtual ~OutputGenerator() = default;
  /**
  \brief Returns true if an output stream has been set.

  \returns True if an output stream has been set. False otherwise.
  */
  bool has_stream() const { return os_ != nullptr; }
  /**
  \brief Sets the output stream.

  \param[in] o Output stream.
  */
  void set_stream(std::ostream &o) noexcept { os_ = &o; }
  /**
  \brief Get the error flag.

  \returns True when an error has been encountered.
  */
  bool error() const noexcept { return errorFlag_; }
  /**
  \brief Clears the error flag.
  */
  virtual void clear_error() noexcept { errorFlag_ = false; }
  /**
  \brief Get the error message.

  \returns The error message string.
  */
  virtual string error_message() { return "Something went wrong.\n"; }
  /**
  \brief Outputs a token to the given stream.

  \param[in] tokens Symbols to be output.

  The default output implementation.
  */
  virtual void output(const tstack<Symbol> &terminals) {
    auto &os = *os_;
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