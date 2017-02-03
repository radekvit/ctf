#ifndef XVITRA00_OUTPUT
#define XVITRA00_OUTPUT

#include <ostream>
#include <base.h>

namespace bp {

class OutputGenerator {
public:
    virtual void set_output(std::ostream &o) = 0;
    virtual void get_token(const nonterminal &nt) = 0;
};

}

#endif