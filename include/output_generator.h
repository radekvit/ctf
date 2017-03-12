/**
\file output_generator.h
\brief Defines class OutputGenerator, which transforms output tokens to output
and outputs it into a stream.
*/
#ifndef CTF_OUTPUT
#define CTF_OUTPUT

#include <base.h>
#include <ostream>

namespace ctf {
/**
Outputs tokens to output stream.
*/
class OutputGenerator {
public:
    /**
    \brief Alias for std::function.
    */
    using output_function =
        std::function<void(std::ostream &, const Terminal &)>;

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
    */
    OutputGenerator(output_function f =
                        [](std::ostream &os, const Terminal &t) {
                            os << t.name() << "." << t.attribute() << "\n";
                        })
        : os(nullptr), outputFunction(f)
    {
    }
    /**
    \brief Constructs OutputGenerator with an output stream.
    */
    OutputGenerator(std::ostream &_o,
                    output_function f =
                        [](std::ostream &os, const Terminal &t) {
                            os << t.name() << "." << t.attribute() << "\n";
                        })
        : os(&_o), outputFunction(f)
    {
    }

    /**
    \brief Returns true if an output stream has been set.
    */
    bool stream_set() const { return os; }
    /**
    \brief Sets the output stream.
    */
    void set_output(std::ostream &o) { os = &o; }
    /**
    \brief Outputs a token to the given stream. If OutputGenerator::stream_set()
    is false, this results in undefined behavior.
    */
    void get_token(const Terminal &t) { outputFunction(*os, t); }
};
} // namespace ctf

#endif
/*** End of file output_generator.h ***/