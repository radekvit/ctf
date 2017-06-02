#include <catch.hpp>

#include "../src/ll_translation_control.hpp"

#include <sstream>

using ctf::Symbol;
using ctf::LLTranslationControl;
using ctf::TranslationGrammar;
using ctf::LexicalAnalyzer;
using namespace ctf::literals;

TEST_CASE("LLTranslationControl construction", "[LLTranslationControl]") {
  TranslationGrammar tg{
      {
          {"E"_nt, {"T"_nt, "E'"_nt}},
          {"E'"_nt, {}},
          {"E'"_nt, {"+"_t, "T"_nt, "E'"_nt}, {"T"_nt, "+"_t, "E'"_nt}},
          {"F"_nt, {"("_t, "E"_nt, ")"_t}, {"E"_nt}},
          {"F"_nt, {"i"_t}},
          {"T"_nt, {"F"_nt, "T'"_nt}},
          {"T'"_nt, {}},
          {"T'"_nt, {"*"_t, "F"_nt, "T'"_nt}, {"F"_nt, "*"_t, "T'"_nt}},
      },
      "E"_nt};
  LexicalAnalyzer a;
  SECTION("no arguments") { REQUIRE_NOTHROW(LLTranslationControl()); }

  SECTION("with grammar and analyzer") {
    REQUIRE_NOTHROW(LLTranslationControl(a, tg));
  }
}

TEST_CASE("LLTranslationControl run", "[LLTranslationControl]") {
  LexicalAnalyzer a;
  SECTION("Accept empty string only") {
    TranslationGrammar tg{{{"E"_nt, {}}}, "E"_nt};
    std::stringstream in;
    a.set_stream(in);
    LLTranslationControl ll(a, tg);
    ll.run();
    // only eof
    REQUIRE(ll.output().size() == 1);
    REQUIRE(ll.output().top() == Symbol::eof());
  }
}
