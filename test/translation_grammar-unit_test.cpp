#include <catch.hpp>

#include "../src/translation_grammar.hpp"

#include <algorithm>

using ctf::vector;
using ctf::Symbol;
using ctf::TranslationGrammar;
using Rule = ctf::TranslationGrammar::Rule;

using namespace ctf::literals;

TEST_CASE("Rule construction", "[TranslationGrammar::Rule]") {
  REQUIRE_NOTHROW(Rule("NT"_nt, {}));
  REQUIRE_NOTHROW(Rule("NT"_nt, {"x"_t, "y"_t}));
  REQUIRE_NOTHROW(Rule("NT"_nt, {"X"_nt, "X"_nt, "x"_t}));
  REQUIRE(Rule("NT"_nt, {"X"_nt, "X"_nt, "x"_t}).actions() ==
          vector<vector<size_t>>{{2}});
  REQUIRE_NOTHROW(Rule("NT"_nt, {"x"_t, "y"_t}, {"y"_t, "y"_t}, {{}, {0, 1}}));

  REQUIRE_THROWS_AS(Rule("NT"_nt, {"x"_t}, {}, {{}, {}}),
                    std::invalid_argument &);
  REQUIRE_THROWS_AS(Rule("NT"_nt, {"x"_t}, {}, {{0}}), std::invalid_argument &);
  REQUIRE_THROWS_AS(Rule("NT"_nt, {"x"_t, "X"_nt}, {"X"_nt}, {{0}}),
                    std::invalid_argument &);
  REQUIRE_THROWS_AS(Rule("NT"_nt, {Symbol::eof()}, {"x"_t}),
                    std::invalid_argument &);
  REQUIRE_THROWS_AS(Rule("NT"_nt, {}, {Symbol::eof()}),
                    std::invalid_argument &);
  REQUIRE_THROWS_AS(Rule("NT"_nt, {"X"_nt, "Y"_nt}, {"Y"_nt, "X"_nt}),
                    std::invalid_argument &);
}

TEST_CASE("Rule basics", "[TranslationGrammar::Rule]") {
  Rule rule("NT"_nt, {"a"_t, "A"_nt, "B"_nt, "b"_t},
            {"A"_nt, "b"_t, "a"_t, "B"_nt}, {{1}, {2}});
  vector<vector<size_t>> expectedActions{{1}, {2}};
  REQUIRE(rule.actions() == expectedActions);
  REQUIRE(rule.nonterminal() == "NT"_nt);
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
  REQUIRE_NOTHROW(TranslationGrammar({}, "X"_nt));
  REQUIRE_NOTHROW(TranslationGrammar({{"X"_nt, {}}}, "X"_nt));
  REQUIRE_NOTHROW(TranslationGrammar(
      {{"X"_nt, {"X"_nt, "X"_t}}, {"X"_nt, {"x"_t, "X"_t, "special"_s}}},
      "X"_nt));
  REQUIRE_THROWS_AS(TranslationGrammar({}, {}, {}, "X"_nt),
                    std::invalid_argument &);
  REQUIRE_THROWS_AS(TranslationGrammar({}, {}, {{"X"_nt, {}}}, "X"_nt),
                    std::invalid_argument &);
  REQUIRE_THROWS_AS(
      TranslationGrammar({"X"_nt}, {}, {{"X"_nt, {"x"_t}}}, "X"_nt),
      std::invalid_argument &);
  REQUIRE_THROWS_AS(TranslationGrammar({"X"_nt}, {"x"_t},
                                       {{"X"_nt, {"x"_t, "A"_nt}}}, "X"_nt),
                    std::invalid_argument &);
  REQUIRE_NOTHROW(TranslationGrammar(
      {"X"_nt}, {"x"_t},
      {{"X"_nt, {"X"_nt, "X"_nt}}, {"X"_nt, {"x"_t, "X"_nt, "special"_s}}},
      "X"_nt));
}
TEST_CASE("TranslationGrammar basic", "[TranslationGrammar]") {
  TranslationGrammar grammar{
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
  vector<Symbol> expectedTerminals{"+"_nt, "("_nt, ")"_nt, "*"_nt, "i"_nt};
  vector<Symbol> expectedNonterminals{"E"_nt, "E'"_nt, "F"_nt, "T"_nt, "T'"_nt};
  std::sort(expectedTerminals.begin(), expectedTerminals.end());
  std::sort(expectedNonterminals.begin(), expectedNonterminals.end());

  REQUIRE(grammar.terminals() == expectedTerminals);
  REQUIRE(grammar.nonterminals() == expectedNonterminals);

  Symbol nonterminal = expectedNonterminals[2];
  Symbol terminal = expectedTerminals[3];
  REQUIRE(grammar.nonterminal_index(nonterminal) == 2);
  REQUIRE(grammar.terminal_index(terminal) == 3);

  REQUIRE(TranslationGrammar::EPSILON_STRING().size() == 0);
}