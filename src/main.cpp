#include <ll_table.h>
#include <translation_grammar.h>

using namespace bp;

using Terminal = TranslationGrammar::Terminal;
using Nonterminal = TranslationGrammar::Nonterminal;
using Rule = TranslationGrammar::Rule;
using Symbol = TranslationGrammar::Symbol;

int main()
{
    Terminal i("i"), plus("+"), ast("*"), parl("("), parr(")");
    Nonterminal E("E"), E_("E'"), T("T"), T_("T'"), F("F");
    vector<Terminal> t{i, plus, ast, parl, parr};
    vector<Nonterminal> n{E, E_, T, T_, F};
    vector<Rule> r{
        Rule(E, vector<Symbol>{Symbol(T), Symbol(E_)}, vector<Symbol>()),
        Rule(E_, vector<Symbol>{Symbol(plus), Symbol(T), Symbol(E_)},
             vector<Symbol>()),
        Rule(E_, vector<Symbol>{Symbol(Symbol::Type::EPSILON)},
             vector<Symbol>()),
        Rule(T, vector<Symbol>{Symbol(F), Symbol(T_)}, vector<Symbol>()),
        Rule(T_, vector<Symbol>{Symbol(ast), Symbol(F), Symbol(T_)},
             vector<Symbol>()),
        Rule(T_, vector<Symbol>{Symbol(Symbol::Type::EPSILON)},
             vector<Symbol>()),
        Rule(F, vector<Symbol>{Symbol(parl), Symbol(E), Symbol(parr)},
             vector<Symbol>()),
        Rule(F, vector<Symbol>{Symbol(i)}, vector<Symbol>()),
    };
    TranslationGrammar tg(t, n, r, Symbol(E));
    tg.create_ll_table();
    return 0;
}