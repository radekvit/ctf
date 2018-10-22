#include <catch.hpp>

#include "../src/ctf_lr_translation_control.hpp"

#include <iostream>
#include <sstream>

using ctf::LexicalAnalyzer;
using ctf::TranslationGrammar;
using Rule = TranslationGrammar::Rule;
using ctf::LR0StateMachine;
using ctf::SLRTranslationControl;
using ctf::LALRTranslationControl;

using ctf::vector;
using ctf::string;
using ctf::Symbol;
using ctf::InputReader;
using ctf::Location;

using namespace ctf::literals;

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
}

TEST_CASE("Regular SLR translation", "[SLRTranslationControl]") {
  TranslationGrammar tg{
      {
          {"S"_nt, {"S"_nt, "o"_t, "A"_nt}, {"1"_t, "S"_nt, "A"_nt}, {{0}}},
          {"S"_nt, {"A"_nt}, {"2"_t, "A"_nt}},
          {"A"_nt, {"i"_t}, {"3"_t}, {{0}}},
          {"A"_nt, {"("_t, "S"_nt, ")"_t}, {"4"_t, "S"_nt}, {{0}, {}}},
          {"S'"_nt, {"S"_nt}},
      },
      "S'"_nt};

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
  Symbol os = *it++;
  REQUIRE(os == "2"_t);
  os = *it++;
  REQUIRE(os == "4"_t);
  REQUIRE(os.location() == Location(1, 1));
}

TEST_CASE("Failed SLR translation", "[SLRTranslationControl]") {
  TranslationGrammar tg{
      {
          {"S"_nt, {"S"_nt, "o"_t, "A"_nt}, {"1"_t, "S"_nt, "A"_nt}, {{0}}},
          {"S"_nt, {"A"_nt}, {"2"_t, "A"_nt}},
          {"A"_nt, {"i"_t}, {"3"_t}, {{0}}},
          {"A"_nt, {"("_t, "S"_nt, ")"_t}, {"4"_t, "S"_nt}, {{0}, {}}},
          {"S'"_nt, {"S"_nt}},
      },
      "S'"_nt};

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

TEST_CASE("LALR empty translation", "[LALTTranslationControl]") {
  LexicalAnalyzer a;
  TranslationGrammar tg{{{"E"_nt, {}}}, "E"_nt};
  std::stringstream in;
  InputReader r{in};
  a.set_reader(r);
  LALRTranslationControl lalr(a, tg);
  lalr.run();
  // only eof
  REQUIRE(lalr.output().size() == 1);
  REQUIRE(lalr.output().top() == Symbol::eof());
}
