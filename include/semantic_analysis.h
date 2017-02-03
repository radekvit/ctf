#ifndef XVITRA00_SEMANTIC
#define XVITRA00_SEMANTIC

#include <ostream>

namespace bp {

class SemanticAnalyzer {
public:
    virtual void set_output(std::ostream &o) = 0;
    virtual void accept_token(const nonterminal &nt) = 0;
}

}

#endif