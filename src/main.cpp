#include <iostream>
#include <ll_table.h>
#include <translation_grammar.h>
#include <translation.h>

using namespace bp;

using Rule = TranslationGrammar::Rule;

class DummyLexicalAnalyzer : public LexicalAnalyzer {
public:
    void set_input(std::istream &) override {};
    virtual Token get_token() override { return Token(); }
} dla;

int main()
{
    Terminal i("i"), plus("+"), ast("*"), parl("("), parr(")");
    Nonterminal E("E"), E_("E'"), T("T"), T_("T'"), F("F");
    vector<Terminal> t{i, plus, ast, parl, parr};
    vector<Nonterminal> n{E, T, F, E_, T_};
    vector<Rule> r{
        {E, {{T}, {E_}}},
        {E_, {{plus}, {T}, {E_}}},
        {E_, {}},
        {T, {{F}, {T_}}},
        {T_, {{ast}, {F}, {T_}}},
        {T_, {}},
        {F, vector<Symbol>{{parl}, {E}, {parr}}},
        {F, vector<Symbol>{{i}}},
    };
    TranslationGrammar tg(t, n, r, {E});

    LLTranslationControl(dla, tg);

    
    return 0;
}