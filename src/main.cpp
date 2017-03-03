#include <iostream>
#include <ll_table.h>
#include <translation.h>
#include <translation_grammar.h>

using namespace bp;

using Rule = TranslationGrammar::Rule;

int main()
{
    LexicalAnalyzer dla;
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