#include <catch.hpp>

#include <sstream>
#include "../src/ctf_ll_translation_control.hpp"
#include "test_utils.h"

using ctf::Symbol;
using ctf::LLTranslationControl;
using ctf::TranslationGrammar;
using ctf::LexicalAnalyzer;
using ctf::InputReader;

static constexpr ctf::Symbol operator""_nt(const char* s, size_t) {
  using namespace ctf::literals;
  if (c_streq(s, "E"))
    return 0_nt;
  if (c_streq(s, "E'"))
    return 1_nt;
  if (c_streq(s, "T"))
    return 2_nt;
  if (c_streq(s, "T'"))
    return 3_nt;
  if (c_streq(s, "F"))
    return 4_nt;

  return 100_nt;
}
static constexpr ctf::Symbol operator""_t(const char* s, size_t) {
  using namespace ctf::literals;
  if (c_streq(s, "+"))
    return 0_t;
  if (c_streq(s, "*"))
    return 1_t;
  if (c_streq(s, "("))
    return 2_t;
  if (c_streq(s, ")"))
    return 3_t;
  if (c_streq(s, "i"))
    return 4_t;

  return 100_t;
}

TEST_CASE("LLTranslationControl construction", "[LLTranslationControl]") {
  TranslationGrammar tg{{
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

  SECTION("with grammar and analyzer") { REQUIRE_NOTHROW(LLTranslationControl(a, tg)); }
}

TEST_CASE("LLTranslationControl run", "[LLTranslationControl]") {
  LexicalAnalyzer a;
  SECTION("Accept empty string only") {
    TranslationGrammar tg{{{"E"_nt, {}}}, "E"_nt};
    std::stringstream in;
    InputReader r{in};
    a.set_reader(r);
    LLTranslationControl ll(a, tg);
    ll.run();
    // only eof
    REQUIRE(ll.output().size() == 1);
    REQUIRE(ll.output().top() == Symbol::eof());
  }
}
