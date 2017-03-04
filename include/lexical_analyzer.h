#ifndef XVITRA00_LEXICAL_ANALYZER
#define XVITRA00_LEXICAL_ANALYZER

#include <base.h>
#include <functional>
#include <istream>
#include <cctype>

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
read:
        if (is.get(c)) {
            if(std::isspace(static_cast<unsigned char>(c)))
                                goto read;
            return Token{{c}};
        }
        else
            return Token::EOI();
    })
        : is(nullptr), tokenFunction(f)
    {
    }
    LexicalAnalyzer(std::istream &_i,
                    token_function f = [](std::istream &is) -> Token {
                        char c;
            read:
                        if (is.get(c)) {
                            if(std::isspace(static_cast<unsigned char>(c)))
                                goto read;
                            return Token{{c}};
                        }
                        else
                            return Token::EOI();
                    })
        : is(&_i), tokenFunction(f)
    {
    }

    bool stream_set() const { return is; }
    void set_input(std::istream &s) { is = &s; }
    Token get_token() { return tokenFunction(*is); };
};
}

#endif