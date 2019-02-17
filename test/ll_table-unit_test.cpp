#include <catch.hpp>

#include "../src/ctf_ll_table.hpp"
#include "test_utils.h"

using ctf::LLTable;
using ctf::TranslationGrammar;
using ctf::Symbol;

static constexpr Symbol operator""_nt(const char* s, size_t) {
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
static constexpr Symbol operator""_t(const char* s, size_t) {
  using namespace ctf::literals;
  if (c_streq(s, "i"))
    return 0_t;
  if (c_streq(s, "+"))
    return 1_t;
  if (c_streq(s, "*"))
    return 2_t;
  if (c_streq(s, "("))
    return 3_t;
  if (c_streq(s, ")"))
    return 4_t;

  return 100_t;
}

TEST_CASE("LLTable construction", "[LLTable]") {
  SECTION("Multiple terminals and nonterminals.") {
    TranslationGrammar tg{2, 3, {}, "E"_nt};
    REQUIRE_NOTHROW(LLTable(tg, {{Symbol::eof()}}));
  }

  SECTION("No terminals, at least one nonterminal") {
    TranslationGrammar tg{2, 1, {}, "E"_nt};
    REQUIRE_NOTHROW(LLTable(tg, {{Symbol::eof()}}));
  }

  SECTION("empty TranslationGrammar") {
    REQUIRE_NOTHROW(LLTable({}, {{Symbol::eof()}, {Symbol::eof()}}));
  }

  SECTION("non-LL Translation Grammar") {
    TranslationGrammar tg{{{"E"_nt, {"i"_t}}, {"E"_nt, {"i"_t}}}, "E"_nt};
    REQUIRE_THROWS_AS(LLTable(tg, {{"i"_t}, {"i"_t}, {"i"_t}}), std::invalid_argument);
  }

  SECTION("regular Translation Grammar") {
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
    REQUIRE(tg.starting_symbol().id() == tg.nonterminals() - 1);
    REQUIRE_NOTHROW(LLTable(tg,
                            {{"i"_t, "("_t},
                             {")"_t, Symbol::eof()},
                             {"+"_t},
                             {"("_t},
                             {"i"_t},
                             {"i"_t, "("_t},
                             {"+"_t, ")"_t, Symbol::eof()},
                             {"*"_t},
                             {"i"_t, "("_t}}));
  }
}

TEST_CASE("rule index returning", "[LLTable]") {
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
  LLTable ll{tg,
             {{"i"_t, "("_t},
              {")"_t, Symbol::eof()},
              {"+"_t},
              {"("_t},
              {"i"_t},
              {"i"_t, "("_t},
              {"+"_t, ")"_t, Symbol::eof()},
              {"*"_t},
              {"i"_t, "("_t}}};
  REQUIRE(ll.rule_index("X"_nt, "+"_t) == tg.rules().size());
  REQUIRE(ll.rule_index("F"_nt, "("_t) == 3);
  REQUIRE(ll.rule_index("E"_nt, ")"_t) == tg.rules().size());
}

// TODO create tests for PriorityLLTable and GeneralLLTable.
