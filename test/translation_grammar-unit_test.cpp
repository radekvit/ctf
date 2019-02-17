#include <catch.hpp>

#include <algorithm>
#include "../src/ctf_translation_grammar.hpp"
#include "test_utils.h"

using ctf::vector;
using ctf::vector_set;
using ctf::Symbol;
using ctf::TranslationGrammar;
using Rule = ctf::TranslationGrammar::Rule;

static constexpr Symbol operator""_nt(const char* s, size_t) {
  using namespace ctf::literals;
  if (c_streq(s, "A"))
    return 0_nt;
  if (c_streq(s, "B"))
    return 1_nt;
  if (c_streq(s, "C"))
    return 2_nt;
  if (c_streq(s, "D"))
    return 3_nt;
  if (c_streq(s, "E"))
    return 4_nt;
  if (c_streq(s, "S"))
    return 5_nt;
  if (c_streq(s, "X"))
    return 6_nt;
  if (c_streq(s, "Y"))
    return 7_nt;

  return 100_nt;
}
static constexpr Symbol operator""_t(const char* s, size_t) {
  using namespace ctf::literals;
  if (c_streq(s, "a"))
    return 0_t;
  if (c_streq(s, "b"))
    return 1_t;
  if (c_streq(s, "c"))
    return 2_t;
  if (c_streq(s, "x"))
    return 3_t;
  if (c_streq(s, "y"))
    return 4_t;
  if (c_streq(s, "X"))
    return 0_t;

  return 100_t;
}

using namespace ctf::literals;

TEST_CASE("Rule construction", "[TranslationGrammar::Rule]") {
  REQUIRE_NOTHROW(Rule("C"_nt, {}));
  REQUIRE_NOTHROW(Rule("C"_nt, {"x"_t, "y"_t}));
  REQUIRE_NOTHROW(Rule("C"_nt, {"X"_nt, "X"_nt, "x"_t}));
  REQUIRE(Rule("C"_nt, {"X"_nt, "X"_nt, "x"_t}).actions() == vector<vector_set<size_t>>{{2}});
  REQUIRE_NOTHROW(Rule("C"_nt, {"x"_t, "y"_t}, {"y"_t, "y"_t}, {{}, {0, 1}}));

  REQUIRE_THROWS_AS(Rule("C"_nt, {"x"_t}, {"y"_t}, {{}, {}}), std::invalid_argument);
  REQUIRE_THROWS_AS(Rule("C"_nt, {"x"_t}, {}, {{0}}), std::invalid_argument);
  REQUIRE_THROWS_AS(Rule("C"_nt, {"x"_t, "X"_nt}, {"X"_nt}, {{0}}), std::invalid_argument);
  REQUIRE_THROWS_AS(Rule("C"_nt, {"X"_nt, "Y"_nt}, {"Y"_nt, "X"_nt}), std::invalid_argument);
}

TEST_CASE("Rule basics", "[TranslationGrammar::Rule]") {
  Rule rule("C"_nt, {"a"_t, "A"_nt, "B"_nt, "b"_t}, {"A"_nt, "b"_t, "a"_t, "B"_nt}, {{1}, {2}});
  vector<vector_set<size_t>> expectedActions{{1}, {2}};
  REQUIRE(rule.actions() == expectedActions);
  REQUIRE(rule.nonterminal() == "C"_nt);
  vector<Symbol> expectedIn{"a"_t, "A"_nt, "B"_nt, "b"_t};
  vector<Symbol> expectedOut{"A"_nt, "b"_t, "a"_t, "B"_nt};
  REQUIRE(rule.input() == expectedIn);
  REQUIRE(rule.output() == expectedOut);
}

TEST_CASE("Rule comparison operators", "[TranslationGrammar::Rule]") {
  Rule r1("A"_nt, {});
  Rule r2("A"_nt, {}, {"a"_t});
  Rule r3("A"_nt, {}, {"b"_t});
  Rule r4("A"_nt, {"x"_t}, {});
  Rule r5("A"_nt, {"x"_t}, {"a"_t, "a"_t, "a"_t});
  Rule r6("A"_nt, {"x"_t}, {"a"_t, "a"_t, "b"_t});
  Rule r7("B"_nt, {});
  Rule r8("B"_nt, {});

  REQUIRE(r1 < r2);
  REQUIRE(r2 < r3);
  REQUIRE(r3 < r4);
  REQUIRE(r4 < r5);
  REQUIRE(r5 < r6);
  REQUIRE(r6 < r7);
  REQUIRE_FALSE(r5 < r2);
  REQUIRE(r7 == r8);
  REQUIRE(r8 == r7);
  REQUIRE_FALSE(r1 == r7);
  REQUIRE(r5 > r2);
  REQUIRE_FALSE(r1 > r2);
  REQUIRE(r1 != r7);
  REQUIRE_FALSE(r3 != r3);
  REQUIRE(r1 >= r1);
  REQUIRE(r3 >= r1);
  REQUIRE(r3 <= r3);
  REQUIRE(r3 <= r6);
  REQUIRE_FALSE(r4 <= r3);
  REQUIRE(r3 >= r1);
}

TEST_CASE("TranslationGrammar construction", "[TranslationGrammar]") {
  REQUIRE_NOTHROW(TranslationGrammar());
  REQUIRE_NOTHROW(TranslationGrammar(std::vector<ctf::Rule>(), "X"_nt));
  REQUIRE_NOTHROW(TranslationGrammar({{"X"_nt, {}}}, "X"_nt));
  REQUIRE_NOTHROW(
      TranslationGrammar({{"X"_nt, {"X"_nt, "X"_t}}, {"X"_nt, {"x"_t, "X"_t}}}, "X"_nt));
  REQUIRE_THROWS_AS(TranslationGrammar({}, {}, {}, "X"_nt), std::invalid_argument);
  REQUIRE_THROWS_AS(TranslationGrammar({}, {}, {{"X"_nt, {}}}, "X"_nt), std::invalid_argument);
  REQUIRE_THROWS_AS(TranslationGrammar("X"_nt.id() + 1, 1, {{"X"_nt, {"x"_t}}}, "X"_nt),
                    std::invalid_argument);
  REQUIRE_THROWS_AS(TranslationGrammar(1, "x"_t.id() + 1, {{"X"_nt, {"x"_t, "A"_nt}}}, "X"_nt),
                    std::invalid_argument);
  REQUIRE_NOTHROW(TranslationGrammar("X"_nt.id() + 1,
                                     "x"_t.id() + 1,
                                     {{"X"_nt, {"X"_nt, "X"_nt}}, {"X"_nt, {"x"_t, "X"_nt}}},
                                     "X"_nt));
}
TEST_CASE("TranslationGrammar basic", "[TranslationGrammar]") {
  TranslationGrammar grammar{{
                                 {"A"_nt, {"B"_nt, "C"_nt}},
                                 {"C"_nt, {}},
                                 {"C"_nt, {"a"_t, "B"_nt, "C"_nt}, {"B"_nt, "a"_t, "C"_nt}},
                                 {"D"_nt, {"b"_t, "A"_nt, "c"_t}, {"A"_nt}},
                                 {"D"_nt, {"y"_t}},
                                 {"B"_nt, {"D"_nt, "E"_nt}},
                                 {"E"_nt, {}},
                                 {"E"_nt, {"x"_t, "D"_nt, "E"_nt}, {"D"_nt, "x"_t, "E"_nt}},
                             },
                             "A"_nt};
  size_t expectedTerminals = "y"_t.id() + 1;
  size_t expectedNonterminals = "E"_nt.id() + 2;

  REQUIRE(grammar.terminals() == expectedTerminals);
  REQUIRE(grammar.nonterminals() == expectedNonterminals);
}