#ifndef XVITRA00_LEXICAL_ANALYZER
#define XVITRA00_LEXICAL_ANALYZER

#include <istream>
#include <base.h>

namespace bp {

class LexicalAnalyzer {
public:
    virtual void set_stream(std::istream &s) = 0;
}

}

#endif