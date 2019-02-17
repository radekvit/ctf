#include <catch.hpp>

#include <iostream>
#include <sstream>
#include "../src/ctf_lr_translation_control.hpp"
#include "test_utils.h"

using ctf::LexicalAnalyzer;
using ctf::TranslationGrammar;
using Rule = TranslationGrammar::Rule;
using ctf::LR0StateMachine;
using ctf::SLRTranslationControl;
using ctf::LALRTranslationControl;
using ctf::LR1TranslationControl;
using ctf::LALRStrictTranslationControl;
using ctf::LR1StrictTranslationControl;

using ctf::vector;
using ctf::string;
using ctf::Symbol;
using ctf::Token;
using ctf::InputReader;
using ctf::Location;
using ctf::PrecedenceSet;
using ctf::Associativity;

static constexpr ctf::Symbol operator""_nt(const char* s, size_t) {
  using namespace ctf::literals;
  if (c_streq(s, "S"))
    return 0_nt;
  if (c_streq(s, "Expr"))
    return 2_nt;
  if (c_streq(s, "E"))
    return 1_nt;
  if (c_streq(s, "A"))
    return 2_nt;
  if (c_streq(s, "F"))
    return 3_nt;

  return 100_nt;
}
static constexpr ctf::Symbol operator""_t(const char* s, size_t) {
  using namespace ctf::literals;
  if (c_streq(s, "o"))
    return 0_t;
  if (c_streq(s, "i"))
    return 1_t;
  if (c_streq(s, "("))
    return 2_t;
  if (c_streq(s, ")"))
    return 3_t;

  if (c_streq(s, "1"))
    return 4_t;
  if (c_streq(s, "2"))
    return 5_t;
  if (c_streq(s, "3"))
    return 6_t;
  if (c_streq(s, "4"))
    return 7_t;

  if (c_streq(s, "a"))
    return 0_t;
  if (c_streq(s, "b"))
    return 1_t;
  if (c_streq(s, "c"))
    return 2_t;
  if (c_streq(s, "d"))
    return 3_t;

  if (c_streq(s, "e"))
    return 2_t;

  if (c_streq(s, "+"))
    return 4_t;
  if (c_streq(s, "-"))
    return 5_t;
  if (c_streq(s, "*"))
    return 6_t;
  if (c_streq(s, "/"))
    return 7_t;
  if (c_streq(s, "^"))
    return 8_t;
  if (c_streq(s, "unary-"))
    return 666_t;

  return 100_t;
}

TEST_CASE("SLRTranslationTest", "[SLRTranslationControl]") {
  TranslationGrammar tg{{{"E"_nt, {}}}, "E"_nt};
  LexicalAnalyzer a;
  SLRTranslationControl(a, tg);
}

TEST_CASE("Empty SLR translation", "[SLRTranslationControl]") {
  LexicalAnalyzer a;
  TranslationGrammar tg{{{"E"_nt, {}}}, "E"_nt};
  std::stringstream in;
  InputReader r{in};
  a.set_reader(r);
  SLRTranslationControl slr(a, tg);
  slr.run();
  // only eof
  REQUIRE(slr.output().size() == 1);
  REQUIRE(slr.output().top() == Symbol::eof());
  REQUIRE(slr.output().top().location() == Location(1, 1));
}

TEST_CASE("Regular SLR translation", "[SLRTranslationControl]") {
  TranslationGrammar tg{{
                            {"S"_nt, {"S"_nt, "o"_t, "A"_nt}, {"1"_t, "S"_nt, "A"_nt}, {{0}}},
                            {"S"_nt, {"A"_nt}, {"2"_t, "A"_nt}},
                            {"A"_nt, {"i"_t}, {"3"_t}, {{0}}},
                            {"A"_nt, {"("_t, "S"_nt, ")"_t}, {"4"_t, "S"_nt}, {{0}, {}}},
                        },
                        "S"_nt};

  LexicalAnalyzer a;
  std::stringstream in;
  // expected output:
  // 2 4 1 2 3 4 1 2 3 3 eof
  in << "( i o ( i o i ) )";
  InputReader r{in};
  a.set_reader(r);
  SLRTranslationControl slr(a, tg);
  slr.run();
  REQUIRE(slr.output().size() == 11);
  auto it = slr.output().begin();
  Token os = *it++;
  REQUIRE(os == "2"_t);
  os = *it++;
  REQUIRE(os == "4"_t);
  REQUIRE(os.location() == Location(1, 1));
  os = *it++;
  REQUIRE(os == "1"_t);
  REQUIRE(os.location() == Location(1, 5));
  os = *it++;
  REQUIRE(os == "2"_t);
  os = *it++;
  REQUIRE(os == "3"_t);
  REQUIRE(os.location() == Location(1, 3));
  os = *it++;
  REQUIRE(os == "4"_t);
  REQUIRE(os.location() == Location(1, 7));
  os = *it++;
  REQUIRE(os == "1"_t);
  REQUIRE(os.location() == Location(1, 11));
  os = *it++;
  REQUIRE(os == "2"_t);
  os = *it++;
  REQUIRE(os == "3"_t);
  REQUIRE(os.location() == Location(1, 9));
  os = *it++;
  REQUIRE(os == "3"_t);
  REQUIRE(os.location() == Location(1, 13));
  os = *it++;
  REQUIRE(os == Symbol::eof());
  REQUIRE(os.location() == Location(1, 18));
}

TEST_CASE("Failed SLR translation", "[SLRTranslationControl]") {
  TranslationGrammar tg{{
                            {"S"_nt, {"S"_nt, "o"_t, "A"_nt}, {"1"_t, "S"_nt, "A"_nt}, {{0}}},
                            {"S"_nt, {"A"_nt}, {"2"_t, "A"_nt}},
                            {"A"_nt, {"i"_t}, {"3"_t}, {{0}}},
                            {"A"_nt, {"("_t, "S"_nt, ")"_t}, {"4"_t, "S"_nt}, {{0}, {}}},
                        },
                        "S"_nt};

  LexicalAnalyzer a;
  std::stringstream in;
  std::stringstream err;
  in << "( ( i o i ) ) )";
  InputReader r{in};
  a.set_reader(r);
  SLRTranslationControl slr(a, tg);
  slr.set_error_stream(err);
  slr.run();
  REQUIRE(slr.error());
}

TEST_CASE("LALR empty translation", "[LALRStrictTranslationControl]") {
  LexicalAnalyzer a;
  TranslationGrammar tg{{{"E"_nt, {}}}, "E"_nt};
  std::stringstream in;
  std::stringstream err;
  InputReader r{in};
  a.set_reader(r);
  LALRStrictTranslationControl lalr(a, tg);
  lalr.set_error_stream(err);
  lalr.run();
  REQUIRE(lalr.output().size() == 1);
  REQUIRE(lalr.output().top() == Symbol::eof());
  REQUIRE(lalr.output().top().location() == Location(1, 1));
}

TEST_CASE("LALR full translation", "[LALRStrictTranslationControl]") {
  TranslationGrammar tg{{
                            {"S"_nt, {"S"_nt, "o"_t, "A"_nt}, {"1"_t, "S"_nt, "A"_nt}, {{0}}},
                            {"S"_nt, {"A"_nt}, {"2"_t, "A"_nt}},
                            {"A"_nt, {"i"_t}, {"3"_t}, {{0}}},
                            {"A"_nt, {"("_t, "S"_nt, ")"_t}, {"4"_t, "S"_nt}, {{0}, {}}},
                        },
                        "S"_nt};

  LexicalAnalyzer a;
  std::stringstream in;
  // expected output:
  // 2 4 1 2 3 4 1 2 3 3 eof
  in << "( i o ( i o i ) )";
  InputReader r{in};
  a.set_reader(r);
  LALRStrictTranslationControl lalr(a, tg);
  lalr.run();
  REQUIRE(lalr.output().size() == 11);
  auto it = lalr.output().begin();
  Token os = *it++;
  REQUIRE(os == "2"_t);
  os = *it++;
  REQUIRE(os == "4"_t);
  REQUIRE(os.location() == Location(1, 1));
  os = *it++;
  REQUIRE(os == "1"_t);
  REQUIRE(os.location() == Location(1, 5));
  os = *it++;
  REQUIRE(os == "2"_t);
  os = *it++;
  REQUIRE(os == "3"_t);
  REQUIRE(os.location() == Location(1, 3));
  os = *it++;
  REQUIRE(os == "4"_t);
  REQUIRE(os.location() == Location(1, 7));
  os = *it++;
  REQUIRE(os == "1"_t);
  REQUIRE(os.location() == Location(1, 11));
  os = *it++;
  REQUIRE(os == "2"_t);
  os = *it++;
  REQUIRE(os == "3"_t);
  REQUIRE(os.location() == Location(1, 9));
  os = *it++;
  REQUIRE(os == "3"_t);
  REQUIRE(os.location() == Location(1, 13));
  os = *it++;
  REQUIRE(os == Symbol::eof());
  REQUIRE(os.location() == Location(1, 18));
}

TEST_CASE("LALR non-SLR translation", "[LALRStrictTranslationControl]") {
  TranslationGrammar tg{{
                            {"S"_nt, {"A"_nt, "a"_t}},
                            {"S"_nt, {"b"_t, "A"_nt, "c"_nt}},
                            {"S"_nt, {"d"_t, "c"_t}},
                            {"S"_nt, {"b"_t, "d"_t, "a"_t}},
                            {"A"_nt, {"d"_t}},
                        },
                        "S"_nt};

  LexicalAnalyzer a;
  std::stringstream in;
  // expected output:
  // d c eof
  in << "d  c";
  InputReader r{in};
  a.set_reader(r);
  LALRStrictTranslationControl lalr(a, tg);
  lalr.run();
  REQUIRE(lalr.output().size() == 3);

  auto it = lalr.output().begin();
  Token os = *it++;
  REQUIRE(os == "d"_t);
  REQUIRE(os.location() == Location(1, 1));
  os = *it++;
  REQUIRE(os == "c"_t);
  REQUIRE(os.location() == Location(1, 4));
  os = *it++;
  REQUIRE(os == Symbol::eof());
  REQUIRE(os.location() == Location(1, 5));
}

TEST_CASE("LR(1) empty translation", "[LR1StrictTranslationControl]") {
  LexicalAnalyzer a;
  TranslationGrammar tg{{{"E"_nt, {}}}, "E"_nt};
  std::stringstream in;
  std::stringstream err;
  InputReader r{in};
  a.set_reader(r);
  LR1StrictTranslationControl lr1(a, tg);
  lr1.set_error_stream(err);
  lr1.run();
  REQUIRE(lr1.output().size() == 1);
  REQUIRE(lr1.output().top() == Symbol::eof());
  REQUIRE(lr1.output().top().location() == Location(1, 1));
}

TEST_CASE("LR(1) full translation", "[LR1StrictTranslationControl]") {
  TranslationGrammar tg{{
                            {"S"_nt, {"S"_nt, "o"_t, "A"_nt}, {"1"_t, "S"_nt, "A"_nt}, {{0}}},
                            {"S"_nt, {"A"_nt}, {"2"_t, "A"_nt}},
                            {"A"_nt, {"i"_t}, {"3"_t}, {{0}}},
                            {"A"_nt, {"("_t, "S"_nt, ")"_t}, {"4"_t, "S"_nt}, {{0}, {}}},
                        },
                        "S"_nt};

  LexicalAnalyzer a;
  std::stringstream in;
  // expected output:
  // 2 4 1 2 3 4 1 2 3 3 eof
  in << "( i o ( i o i ) )";
  InputReader r{in};
  a.set_reader(r);
  LR1StrictTranslationControl lr1(a, tg);
  lr1.run();
  REQUIRE(lr1.output().size() == 11);
  auto it = lr1.output().begin();
  Token os = *it++;
  REQUIRE(os == "2"_t);
  os = *it++;
  REQUIRE(os == "4"_t);
  REQUIRE(os.location() == Location(1, 1));
  os = *it++;
  REQUIRE(os == "1"_t);
  REQUIRE(os.location() == Location(1, 5));
  os = *it++;
  REQUIRE(os == "2"_t);
  os = *it++;
  REQUIRE(os == "3"_t);
  REQUIRE(os.location() == Location(1, 3));
  os = *it++;
  REQUIRE(os == "4"_t);
  REQUIRE(os.location() == Location(1, 7));
  os = *it++;
  REQUIRE(os == "1"_t);
  REQUIRE(os.location() == Location(1, 11));
  os = *it++;
  REQUIRE(os == "2"_t);
  os = *it++;
  REQUIRE(os == "3"_t);
  REQUIRE(os.location() == Location(1, 9));
  os = *it++;
  REQUIRE(os == "3"_t);
  REQUIRE(os.location() == Location(1, 13));
  os = *it++;
  REQUIRE(os == Symbol::eof());
  REQUIRE(os.location() == Location(1, 18));
}

// S -> aEa | bEb | aFb | bFa
// E -> e
// F -> e

TEST_CASE("LR(1) full non-LALR translation", "[LR1StrictTranslationControl]") {
  TranslationGrammar tg{{
                            {"S"_nt, {"e"_t, "E"_nt, "a"_t}},
                            {"S"_nt, {"b"_t, "E"_nt, "b"_t}},
                            {"S"_nt, {"a"_t, "F"_nt, "b"_t}},
                            {"S"_nt, {"b"_t, "F"_nt, "a"_t}},
                            {"E"_nt, {"e"_t}},
                            {"F"_nt, {"e"_t}},
                        },
                        "S"_nt};

  LexicalAnalyzer a;
  std::stringstream in;
  // expected output:
  // a e b eof
  in << "a   e   b";
  InputReader r{in};
  a.set_reader(r);
  LR1StrictTranslationControl lr1(a, tg);
  lr1.run();
  REQUIRE(lr1.output().size() == 4);
  auto it = lr1.output().begin();
  Token os = *it++;
  REQUIRE(os == "a"_t);
  REQUIRE(os.location() == Location(1, 1));
  os = *it++;
  REQUIRE(os == "e"_t);
  REQUIRE(os.location() == Location(1, 5));
  os = *it++;
  REQUIRE(os == "b"_t);
  REQUIRE(os.location() == Location(1, 9));
  os = *it++;
  REQUIRE(os == Symbol::eof());
  REQUIRE(os.location() == Location(1, 10));
}

TEST_CASE("Simple infix to postfix calculator translation", "[LALRTranslationControl]") {
  // TODO state machine error, doesn't find isocores properly
  // https://www.gnu.org/software/bison/manual/html_node/Infix-Calc.html#Infix-Calc
  TranslationGrammar tg{
      vector<Rule>({
          {"S"_nt, {}},
          {"S"_nt, {"Expr"_nt}},
          {"Expr"_nt, {"i"_t}},
          {"Expr"_nt, {"Expr"_nt, "+"_t, "Expr"_nt}, {"Expr"_nt, "Expr"_nt, "+"_t}, {{2}}},
          {"Expr"_nt, {"Expr"_nt, "-"_t, "Expr"_nt}, {"Expr"_nt, "Expr"_nt, "-"_t}, {{2}}},
          {"Expr"_nt, {"Expr"_nt, "*"_t, "Expr"_nt}, {"Expr"_nt, "Expr"_nt, "*"_t}, {{2}}},
          {"Expr"_nt, {"Expr"_nt, "/"_t, "Expr"_nt}, {"Expr"_nt, "Expr"_nt, "/"_t}, {{2}}},
          {"Expr"_nt, {"-"_t, "Expr"_nt}, {"Expr"_nt, "-"_t}, {{1}}, true, "unary-"_t},
          {"Expr"_nt, {"Expr"_nt, "^"_t, "Expr"_nt}, {"Expr"_nt, "Expr"_nt, "^"_t}, {{2}}},
          {"Expr"_nt, {"("_t, "Expr"_nt, ")"_t}},
      }),
      "S"_nt,
      vector<PrecedenceSet>({
          {Associativity::LEFT, {"+"_t, "-"_t}},
          {Associativity::LEFT, {"*"_t, "/"_t}},
          {Associativity::NONE, {"unary-"_t}},
          {Associativity::RIGHT, {"^"_t}},
      })};
  LexicalAnalyzer a;
  std::stringstream in;
  // expected output:
  // i.0 i.3 unary- i.6 i.8 i.11 unary- * i.13 / - ^ ^ i.16 + EOF
  in << "i ^ - i ^ ( i - i * - i / i ) + i";
  InputReader r{in};
  a.set_reader(r);
  LALRTranslationControl lalr(a, tg);
  lalr.run();
  REQUIRE(lalr.output().size() == 18);
}

TEST_CASE("Simple infix to postfix calculator translation in LR1", "[LR1TranslationControl]") {
  // TODO state machine error, doesn't find isocores properly
  // https://www.gnu.org/software/bison/manual/html_node/Infix-Calc.html#Infix-Calc
  TranslationGrammar tg{
      vector<Rule>({
          {"S"_nt, {}},
          {"S"_nt, {"Expr"_nt}},
          {"Expr"_nt, {"i"_t}},
          {"Expr"_nt, {"Expr"_nt, "+"_t, "Expr"_nt}, {"Expr"_nt, "Expr"_nt, "+"_t}, {{2}}},
          {"Expr"_nt, {"Expr"_nt, "-"_t, "Expr"_nt}, {"Expr"_nt, "Expr"_nt, "-"_t}, {{2}}},
          {"Expr"_nt, {"Expr"_nt, "*"_t, "Expr"_nt}, {"Expr"_nt, "Expr"_nt, "*"_t}, {{2}}},
          {"Expr"_nt, {"Expr"_nt, "/"_t, "Expr"_nt}, {"Expr"_nt, "Expr"_nt, "/"_t}, {{2}}},
          {"Expr"_nt, {"-"_t, "Expr"_nt}, {"Expr"_nt, "-"_t}, {{1}}, true, "unary-"_t},
          {"Expr"_nt, {"Expr"_nt, "^"_t, "Expr"_nt}, {"Expr"_nt, "Expr"_nt, "^"_t}, {{2}}},
          {"Expr"_nt, {"("_t, "Expr"_nt, ")"_t}},
      }),
      "S"_nt,
      vector<PrecedenceSet>({
          {Associativity::LEFT, {"+"_t, "-"_t}},
          {Associativity::LEFT, {"*"_t, "/"_t}},
          {Associativity::NONE, {"unary-"_t}},
          {Associativity::RIGHT, {"^"_t}},
      })};
  LexicalAnalyzer a;
  std::stringstream in;
  // expected output:
  // i i unary- i i i unary- * i / - ^ ^ i + EOF
  in << "i ^ - i ^ ( i - i * - i / i ) + i";
  InputReader r{in};
  a.set_reader(r);
  LR1TranslationControl lr1(a, tg);
  lr1.run();
  REQUIRE(lr1.output().size() == 18);
}