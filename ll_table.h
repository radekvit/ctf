#ifndef XVITRA00_LL_H
#define XVITRA00_LL_H

#include <generic_types.h>
#include <translation_grammar.h>

namespace bp
{

class LLTable
{
public:
    using col = size_t;
    using row = vector<col>;
    using Terminal = TranslationGrammar::Terminal;
    using Nonterminal = TranslationGrammar::Nonterminal;
private:
public:
    LLTable();
};

}

#endif