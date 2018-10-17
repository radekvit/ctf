#include <catch.hpp>

#include "../src/ctf_lr_translation_control.hpp"

#include <sstream>

using ctf::LexicalAnalyzer;
using ctf::TranslationGrammar;
using Rule = TranslationGrammar::Rule;
using ctf::LR0StateMachine;
using ctf::SLRTranslationControl;
using ctf::vector;
using ctf::string;
using ctf::Symbol;
using ctf::InputReader;

using namespace ctf::literals;

TEST_CASE("SLRTranslationTest", "[SLRTranslationControl]") {
  TranslationGrammar tg{{{"E"_nt, {}}}, "E"_nt};
	tg.make_augmented();
  LexicalAnalyzer a;
  SLRTranslationControl(a, tg);
}

TEST_CASE("Empty translation") {
  LexicalAnalyzer a;
  TranslationGrammar tg{{{"E"_nt, {}}}, "E"_nt};
  tg.make_augmented();
  std::stringstream in;
  InputReader r{in};
  a.set_reader(r);
  SLRTranslationControl slr(a, tg);
  slr.run();
  // only eof
  REQUIRE(slr.output().size() == 1);
  REQUIRE(slr.output().top() == Symbol::eof());
}

TEST_CASE("Regular translation") {
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
  in << "( ( ( i ) ) o ( i o ( i ) ) )";
  InputReader r{in};
  a.set_reader(r);
  SLRTranslationControl slr(a, tg);
  slr.run();
	REQUIRE(slr.output().size() == 17);
}