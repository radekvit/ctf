/**
\file output_generator.hpp
\brief Defines class OutputGenerator, which transforms output tokens to output
and outputs it into a stream.
*/
#ifndef CTF_OUTPUT
#define CTF_OUTPUT

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

 public:
  OutputGenerator() = default;
  OutputGenerator(std::ostream &os) : os_(&os) {}
  virtual ~OutputGenerator() = default;
  /**
  \brief Returns true if an output stream has been set.
  \returns True if an output stream has been set. False otherwise.
  */
  virtual bool has_stream() const { return os_ != nullptr; }
  /**
  \brief Sets the output stream.
  \param[in] o Output stream.
  */
  virtual void set_stream(std::ostream &o) noexcept { os_ = &o; }
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
      if (t.attribute() != "")
        os << "." << t.attribute();
      os << "\n";
    }
  }
};
}  // namespace ctf

#endif
/*** End of file output_generator.hpp ***/