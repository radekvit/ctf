/**
\file lexical_analyzer.h
\brief Defines class LexicalAnalyzer used to supply tokens from input stream.
\author Radek Vít
*/
#ifndef CTF_LEXICAL_ANALYZER
#define CTF_LEXICAL_ANALYZER

#include <base.h>
#include <cctype>
#include <functional>
#include <istream>

namespace ctf {

/**
\brief Alias for Terminal. Token and Terminal are interchangable.
*/
using Token = Terminal;

/**
\brief Default attribute setter.
*/
static Token default_token_attributes(std::istream &is)
{
    char c;
read:
    if (is.get(c)) {
        if (std::isspace(static_cast<unsigned char>(c)))
            goto read;
        return Token{{c}};
    } else
        return Token::EOI();
}

/**
\brief Extracts tokens from input stream.

Lexical errors are created by creating a token with an unused name. Then,
attribute should be used as an error message.
*/
class LexicalAnalyzer {
public:
    /**
    \brief Alias to std::function.
    */
    using token_function = std::function<Token(std::istream &)>;

private:
    /**
    \brief Pointer to the input stream that tokenFunction takes input from. May
    be changed between tokenFunction calls.
    */
    std::istream *is;
    /**
    \brief A function, lambda or callable object used to extract tokens from an
    input stream.
    */
    token_function tokenFunction;

public:
    /**
    \brief Constructs LexicalAnalyzer without a given input stream. If
    specified, f determines the tokenFunction.
    */
    LexicalAnalyzer(token_function f = &default_token_attributes)
        : is(nullptr), tokenFunction(f)
    {
    }
    /**
    \brief Constructs LexicalAnalyzer with a given istream. If specified, f
    determines the tokenFunction.
    */
    LexicalAnalyzer(std::istream &_i,
                    token_function f = &default_token_attributes)
        : is(&_i), tokenFunction(f)
    {
    }

    /**
    \brief Returns true when a stream has been set.
    */
    bool stream_set() const { return is; }
    /**
    \brief Sets the input stream to a given stream.
    */
    void set_input(std::istream &s) { is = &s; }
    /**
    \brief Uses tokenFunction to get a Token from input stream. If
    LexicalAnalyzer::stream_set() is false, this results in undefined behavior.
    */
    Token get_token() { return tokenFunction(*is); };
};
} // namespace ctf

#endif
/*** End of file lexical_analyzer.h ***/