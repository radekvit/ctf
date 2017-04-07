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
\brief An exception class for semantic errors.
*/
class SemanticError : public TranslationException {
 public:
  using TranslationException::TranslationException;
};
/**
Outputs tokens to output stream.
*/
class OutputGenerator {
 public:
  /**
  \brief Alias for std::function.
  */
  using output_function = std::function<void(std::ostream &, const Symbol &)>;

 private:
  /**
  \brief Pointer to the output stream tokens will be output to.
  */
  std::ostream *os;
  /**
  \brief A function, lambda or callable class that outputs incoming tokens to
  a given output stream.
  */
  output_function outputFunction;

 public:
  /**
  \brief Constructs OutputGenerator without an output stream.
  \param[in] f Callable to print incoming symbols to a stream. At the end of
  output, it will receive Symbol::EOI() and should reset itself.
  */
  OutputGenerator(output_function f = OutputGenerator::default_output)
      : os(nullptr), outputFunction(f) {}
  /**
  \brief Constructs OutputGenerator with an output stream.
  \param[in] _o Output stream.
  \param[in] f Callable to print incoming symbols to a stream. At the end of
  output, it will receive Symbol::EOI() and should reset itself.
  */
  OutputGenerator(std::ostream &_o,
                  output_function f = OutputGenerator::default_output)
      : os(&_o), outputFunction(f) {}

  /**
  \brief Returns true if an output stream has been set.
  \returns True if an output stream has been set. False otherwise.
  */
  bool stream_set() const { return os; }
  /**
  \brief Sets the output stream.
  \param[in] o Output stream.
  */
  void set_output(std::ostream &o) { os = &o; }
  /**
  \brief Outputs a token to the given stream.
  \param[in] t Symbol to be output. If t is equal to Symbol::EOI(),
  outputFunction should reset itself.

  If OutputGenerator::stream_set()
  is false, this results in undefined behavior.
  */
  void get_token(const Symbol &t) { outputFunction(*os, t); }

  /**
  \brief Default output function.

  Prints Symbol name and attribute.
  */
  static void default_output(std::ostream &os, const Symbol &t) {
    if (t == Symbol::EOI())
      return;
    os << t.name();
    if (t.attribute() != "")
      os << "." << t.attribute();
    os << "\n";
  }
};
}  // namespace ctf

#endif
/*** End of file output_generator.hpp ***/