#include <iostream>
#include <ll_table.h>
#include <translation_grammar.h>

using namespace bp;

using Rule = TranslationGrammar::Rule;

int main()
{
    Terminal i("i"), plus("+"), ast("*"), parl("("), parr(")");
    Nonterminal E("E"), E_("E'"), T("T"), T_("T'"), F("F");
    vector<Terminal> t{i, plus, ast, parl, parr};
    vector<Nonterminal> n{E, T, F};
    vector<Rule> r{
        Rule(E, vector<Symbol>{Symbol(E), Symbol(plus), Symbol(T)}),
        Rule(E, vector<Symbol>{Symbol(T)}),
        Rule(T, vector<Symbol>{Symbol(T), Symbol(ast), Symbol(F)}),
        Rule(T, vector<Symbol>{Symbol(F)}),
        Rule(F, vector<Symbol>{Symbol(parl), Symbol(E), Symbol(parr)}),
        Rule(F, vector<Symbol>{Symbol(i)}),
    };
    TranslationGrammar tg(t, n, r, Symbol(E));
    return 0;
}