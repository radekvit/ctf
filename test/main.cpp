#include <ll_table.h>
#include <ll_translation_control.h>
#include <translation.h>
#include <translation_grammar.h>
#include <fstream>
#include <iostream>

using namespace ctf;
using Rule = TranslationGrammar::Rule;
using Type = Symbol::Type;

int main() {
  LexicalAnalyzer dla;
  Symbol i(Type::TERMINAL, "i"), plus(Type::TERMINAL, "+"), ast(Type::TERMINAL, "*"), parl(Type::TERMINAL, "("), parr(Type::TERMINAL, ")");
  Symbol E(Type::NONTERMINAL, "E"), E_(Type::NONTERMINAL, "E'"), T(Type::NONTERMINAL, "T"), T_(Type::NONTERMINAL, "T'"), F(Type::NONTERMINAL, "F");
  vector<Symbol> terminals{i, plus, ast, parl, parr};
  vector<Symbol> nonterminals{E, T, F, E_, T_};
  vector<Rule> r{
      {
          E, {{T}, {E_}},
      },
      {E_, {{plus}, {T}, {E_}}, {{T}, {plus}, {E_}}},
      {E_, {}},
      {T, {{F}, {T_}}},
      {T_, {{ast}, {F}, {T_}}, {{F}, {ast}, {T_}}},
      {T_, {}},
      {F, vector<Symbol>{{parl}, {E}, {parr}}, {E}},
      {F, vector<Symbol>{{i}}},
  };
  TranslationGrammar tg(r, E);

  LLTranslationControl tc(dla, tg);

  std::ifstream in("in");
  std::ofstream out("out");

  LexicalAnalyzer la{};
  OutputGenerator og{};

  Translation tranny{la, tc, tg, og};
  tranny.run(in, out);

  return 0;
}