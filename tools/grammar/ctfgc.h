#ifndef CTFGRAMMAR_ctfgc_H
#define CTFGRAMMAR_ctfgc_H

#define CTF_NO_USING_NAMESPACE
#include <ctf.hpp>
#undef CTF_NO_USING_NAMESPACE

namespace ctfgc {

inline namespace literals {

inline constexpr ctf::Symbol operator""_nt(const char* s, std::size_t) {
  if (ctf::c_streq(s, "Associativity"))
    return ctf::Nonterminal(0);
  if (ctf::c_streq(s, "Attribute"))
    return ctf::Nonterminal(1);
  if (ctf::c_streq(s, "AttributeList"))
    return ctf::Nonterminal(2);
  if (ctf::c_streq(s, "Attributes"))
    return ctf::Nonterminal(3);
  if (ctf::c_streq(s, "GrammarC"))
    return ctf::Nonterminal(4);
  if (ctf::c_streq(s, "IntList"))
    return ctf::Nonterminal(5);
  if (ctf::c_streq(s, "OutputString"))
    return ctf::Nonterminal(6);
  if (ctf::c_streq(s, "Precedence"))
    return ctf::Nonterminal(7);
  if (ctf::c_streq(s, "PrecedenceAttribute"))
    return ctf::Nonterminal(8);
  if (ctf::c_streq(s, "PrecedenceLevels"))
    return ctf::Nonterminal(9);
  if (ctf::c_streq(s, "Rule"))
    return ctf::Nonterminal(10);
  if (ctf::c_streq(s, "RuleClause"))
    return ctf::Nonterminal(11);
  if (ctf::c_streq(s, "RuleClauses"))
    return ctf::Nonterminal(12);
  if (ctf::c_streq(s, "RulePrecedence"))
    return ctf::Nonterminal(13);
  if (ctf::c_streq(s, "Rules"))
    return ctf::Nonterminal(14);
  if (ctf::c_streq(s, "Rules'"))
    return ctf::Nonterminal(15);
  if (ctf::c_streq(s, "String"))
    return ctf::Nonterminal(16);
  if (ctf::c_streq(s, "TokenList"))
    return ctf::Nonterminal(17);

  return ctf::Nonterminal(18);
}

inline constexpr ctf::Symbol operator""_t(const char* s, std::size_t) {
  if (ctf::c_streq(s, ","))
    return ctf::Terminal(0);
  if (ctf::c_streq(s, "-"))
    return ctf::Terminal(1);
  if (ctf::c_streq(s, ":"))
    return ctf::Terminal(2);
  if (ctf::c_streq(s, "DEDENT"))
    return ctf::Terminal(3);
  if (ctf::c_streq(s, "INDENT"))
    return ctf::Terminal(4);
  if (ctf::c_streq(s, "NEWLINE"))
    return ctf::Terminal(5);
  if (ctf::c_streq(s, "attribute end"))
    return ctf::Terminal(16);
  if (ctf::c_streq(s, "attribute list end"))
    return ctf::Terminal(17);
  if (ctf::c_streq(s, "attributes"))
    return ctf::Terminal(18);
  if (ctf::c_streq(s, "grammar"))
    return ctf::Terminal(6);
  if (ctf::c_streq(s, "grammar name"))
    return ctf::Terminal(7);
  if (ctf::c_streq(s, "integer"))
    return ctf::Terminal(8);
  if (ctf::c_streq(s, "left"))
    return ctf::Terminal(9);
  if (ctf::c_streq(s, "level end"))
    return ctf::Terminal(19);
  if (ctf::c_streq(s, "none"))
    return ctf::Terminal(10);
  if (ctf::c_streq(s, "nonterminal"))
    return ctf::Terminal(11);
  if (ctf::c_streq(s, "precedence"))
    return ctf::Terminal(12);
  if (ctf::c_streq(s, "precedence end"))
    return ctf::Terminal(20);
  if (ctf::c_streq(s, "right"))
    return ctf::Terminal(13);
  if (ctf::c_streq(s, "rule block end"))
    return ctf::Terminal(21);
  if (ctf::c_streq(s, "rule end"))
    return ctf::Terminal(22);
  if (ctf::c_streq(s, "string end"))
    return ctf::Terminal(23);
  if (ctf::c_streq(s, "terminal"))
    return ctf::Terminal(14);
  if (ctf::c_streq(s, "|"))
    return ctf::Terminal(15);

  return ctf::Terminal(24);
}

}

inline ctf::string to_string(ctf::Symbol s) {
  using namespace ctf::literals;
  static ctf::map<ctf::Symbol, ctf::string> names = {
    {ctf::Terminal(0), "','"},
    {ctf::Terminal(1), "'-'"},
    {ctf::Terminal(2), "':'"},
    {ctf::Terminal(3), "'DEDENT'"},
    {ctf::Terminal(4), "'INDENT'"},
    {ctf::Terminal(5), "'NEWLINE'"},
    {ctf::Terminal(16), "'attribute end'"},
    {ctf::Terminal(17), "'attribute list end'"},
    {ctf::Terminal(18), "'attributes'"},
    {ctf::Terminal(6), "'grammar'"},
    {ctf::Terminal(7), "'grammar name'"},
    {ctf::Terminal(8), "'integer'"},
    {ctf::Terminal(9), "'left'"},
    {ctf::Terminal(19), "'level end'"},
    {ctf::Terminal(10), "'none'"},
    {ctf::Terminal(11), "'nonterminal'"},
    {ctf::Terminal(12), "'precedence'"},
    {ctf::Terminal(20), "'precedence end'"},
    {ctf::Terminal(13), "'right'"},
    {ctf::Terminal(21), "'rule block end'"},
    {ctf::Terminal(22), "'rule end'"},
    {ctf::Terminal(23), "'string end'"},
    {ctf::Terminal(14), "'terminal'"},
    {ctf::Terminal(15), "'|'"},
    {ctf::Nonterminal(0), "Associativity"},
    {ctf::Nonterminal(1), "Attribute"},
    {ctf::Nonterminal(2), "AttributeList"},
    {ctf::Nonterminal(3), "Attributes"},
    {ctf::Nonterminal(4), "GrammarC"},
    {ctf::Nonterminal(5), "IntList"},
    {ctf::Nonterminal(6), "OutputString"},
    {ctf::Nonterminal(7), "Precedence"},
    {ctf::Nonterminal(8), "PrecedenceAttribute"},
    {ctf::Nonterminal(9), "PrecedenceLevels"},
    {ctf::Nonterminal(10), "Rule"},
    {ctf::Nonterminal(11), "RuleClause"},
    {ctf::Nonterminal(12), "RuleClauses"},
    {ctf::Nonterminal(13), "RulePrecedence"},
    {ctf::Nonterminal(14), "Rules"},
    {ctf::Nonterminal(15), "Rules'"},
    {ctf::Nonterminal(16), "String"},
    {ctf::Nonterminal(17), "TokenList"},
  };
  auto it = names.find(s);
  if (it != names.end()) {
    return it->second;
  }
  return s.to_string();
}

extern ctf::TranslationGrammar grammar;

}
#endif
