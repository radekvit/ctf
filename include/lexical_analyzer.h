#ifndef XVITRA00_LEXICAL_ANALYZER
#define XVITRA00_LEXICAL_ANALYZER

#include <base.h>
#include <istream>

namespace bp {

using Token = Terminal;

class LexicalAnalyzer {
public:
    virtual void set_input(std::istream &s) = 0;
    Token get_token();
};
}

#endif