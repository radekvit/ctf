#include <catch.hpp>

#include "../src/ctf_ll_table.hpp"

using ctf::LLTable;
using ctf::TranslationGrammar;
using ctf::Symbol;
using namespace ctf::literals;

TEST_CASE("LLTable construction", "[LLTable]") {
  SECTION("Wrong predict size") {
    REQUIRE_THROWS_AS(LLTable({}, {{"x"_nt}}), std::invalid_argument);
  }

  SECTION("Multiple terminals and nonterminals.") {
    TranslationGrammar tg{{"E"_nt, "F"_nt}, {"x"_t, "y"_t}, {}, "E"_nt};
    REQUIRE_NOTHROW(LLTable(tg, {{Symbol::eof()}}));
  }

  SECTION("No terminals, at least one nonterminal") {
    TranslationGrammar tg{{"E"_nt, "F"_nt}, {}, {}, "E"_nt};
    REQUIRE_NOTHROW(LLTable(tg, {{Symbol::eof()}}));
  }

  SECTION("empty TranslationGrammar") { REQUIRE_NOTHROW(LLTable({}, {{Symbol::eof()}})); }

  SECTION("non-LL Translation Grammar") {
    TranslationGrammar tg{{{"E"_nt, {"x"_t}}, {"E"_nt, {"x"_t}}}, "E"_nt};
    REQUIRE_THROWS_AS(LLTable(tg, {{"x"_nt}, {"x"_nt}}), std::invalid_argument);
  }

  SECTION("regular Translation Grammar") {
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
    REQUIRE(tg.starting_symbol().name() == "E''");
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
