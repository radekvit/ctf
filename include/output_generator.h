#ifndef XVITRA00_OUTPUT
#define XVITRA00_OUTPUT

#include <base.h>
#include <ostream>

namespace bp {

class OutputGenerator {
public:
    virtual void set_output(std::ostream &o) = 0;
    virtual void get_token(const Terminal &nt) = 0;
};
}

#endif