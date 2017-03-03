#ifndef XVITRA00_LEXICAL_ANALYZER
#define XVITRA00_LEXICAL_ANALYZER

#include <base.h>
#include <functional>
#include <istream>

namespace bp {

using Token = Terminal;

class LexicalAnalyzer {
public:
    using token_function = std::function<Token(std::istream &)>;

private:
    std::istream *is;
    token_function tokenFunction;

public:
    LexicalAnalyzer(token_function f = [](std::istream &is) -> Token {
        char c;
        if (is.get(c))
            return Token{{1, c}};
        else
            return Token::EOI();
    })
        : is(nullptr), tokenFunction(f)
    {
    }
    LexicalAnalyzer(std::istream &_i,
                    token_function f = [](std::istream &is) -> Token {
                        char c;
                        if (is.get(c))
                            return Token{{1, c}};
                        else
                            return Token::EOI();
                    })
        : is(&_i), tokenFunction(f)
    {
    }

    bool stream_set() const { return is; }
    virtual void set_input(std::istream &s) { is = &s; }
    virtual Token get_token() { return tokenFunction(*is); };
};
}

#endif