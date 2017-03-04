#ifndef XVITRA00_OUTPUT
#define XVITRA00_OUTPUT

#include <base.h>
#include <ostream>

namespace bp {

class OutputGenerator {
public:
    using output_function = std::function<void(std::ostream &, const Terminal &)>;

private:
    std::ostream *os;
    output_function outputFunction;

public:
    OutputGenerator(output_function f = [](std::ostream &os, const Terminal &t) {
        if(t.attribute() != "")
            os << t.attribute();
        else
            os << t.name();
        return;
    })
        : os(nullptr), outputFunction(f)
    {
    }
    OutputGenerator(std::ostream &_o,
                    output_function f = [](std::ostream &os, const Terminal &t) {
        if(t.attribute() != "")
            os << t.attribute();
        else
            os << t.name();
        return;
    })
        : os(&_o), outputFunction(f)
    {
    }

    bool stream_set() const { return os; }
    void set_output(std::ostream &o) { os = &o; }
    void get_token(const Terminal &t) { outputFunction(*os, t); }
};
}

#endif